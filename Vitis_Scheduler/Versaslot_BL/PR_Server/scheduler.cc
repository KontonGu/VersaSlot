/*
Copyright 2024 VersaSlot Authors, KontonGu (Jianfeng Gu), Hao Wang, et. al.
@Techinical University of Munich, CAPS Cloud Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Note: We used the same priority asignment mechanism as Nimblock project for fair comparison.
*/

#include "scheduler.h"
#include "appgenerator.h"
#include "bitloader.h"
#include "xparameters.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <xil_io.h>
#include <xil_printf.h>

// 创建全局日志容器
std::vector<LogEntry> log_entries;

// 全局变量，用于时间戳
extern uint64_t program_start_time;

// 修改后的 print_log 函数
void print_log(const char *level, const char *message) {
  XTime current_time;
  XTime_GetTime(&current_time);
  u64 elapsed_time_us = ((current_time - program_start_time) * 1000000) /
                        XPAR_CPU_TIMESTAMP_CLK_FREQ;

  LogEntry entry;
  entry.timestamp = elapsed_time_us;
  entry.level = level;
  entry.message = message;

  log_entries.push_back(entry);
}
// Slot 构造函数
Slot::Slot(SlotType slotType, int slotNumber, uint32_t ctrlAddr,
           uint32_t dataAddr, uint32_t decoupleVal, u64 time_sd2mem_start,
           u64 time_sd2mem_end, u64 t_mem2fpga_start, u64 t_mem2fpga_end,
           char *BitFile, Task *curTsk, Application *CurApp)
    : type(slotType), slotNr(slotNumber), ctrl_addr(ctrlAddr),
      data_addr(dataAddr), decouple_value(decoupleVal), slotIsIdle(true),
      t_sd2mem_start(time_sd2mem_start), t_sd2mem_end(time_sd2mem_end),
      t_mem2fpga_start(t_mem2fpga_start), t_mem2fpga_end(t_mem2fpga_end),
      BitstreamFile(BitFile), curTask(curTsk), curApp(CurApp) {}

// 更新控制和数据地址
void Slot::update_addr() {
  if (type == SLOT_S) {
    switch (slotNr) {
    case 1:
      ctrl_addr = 0xA0050000;
      data_addr = 0xA0040000;
      break;
    case 2:
      ctrl_addr = 0xA0070000;
      data_addr = 0xA0060000;
      break;
    case 3:
      ctrl_addr = 0xA0090000;
      data_addr = 0xA0080000;
      break;
    case 4:
      ctrl_addr = 0xA00B0000;
      data_addr = 0xA00A0000;
      break;
    default:
      ctrl_addr = 0x0;
      data_addr = 0x0;
      break;
    }
  } else if (type == SLOT_L) {
    switch (slotNr) {
    case 1:
      ctrl_addr = 0xA0010000;
      data_addr = 0xA0000000;
      break;
    case 2:
      ctrl_addr = 0xA0030000;
      data_addr = 0xA0020000;
      break;
    default:
      ctrl_addr = 0x0;
      data_addr = 0x0;
      break;
    }
  }
}

// 更新GPIO Decouple值
void Slot::get_decouple_value() {
  if (type == SLOT_S) {
    if (slotNr >= 1 && slotNr <= 4) {
      decouple_value = 1 << (slotNr - 1);
    } else {
      decouple_value = 0;
    }
  } else if (type == SLOT_L) {
    if (slotNr >= 1 && slotNr <= 2) {
      decouple_value = 1 << (slotNr - 1 + 4);
    } else {
      decouple_value = 0;
    }
  }
}

// 获取比特流名称
void Slot::get_bitFile_name() { BitstreamFile = curTask->getBitName(); }

int Slot::Slot_PR(XGpio &GpioDecouple, XFpga &XFpgaInstance) {
  int status;
  char *PartialBit = NULL; // 修改为 char*
  u32 fileSize = 0;
  UINTPTR KeyAddr = (UINTPTR)NULL;
  XTime start_time, end_time;

  // 从SD卡加载比特流到内存
  XTime_GetTime(&start_time);
  t_sd2mem_start = start_time; // 记录 SD 卡到内存的开始时间

  status = load_bitstream_to_memory(BitstreamFile, &PartialBit,
                                    &fileSize); // 传递 PartialBit 的地址
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to load bitstream to memory");
    cleanup_platform();
    return XST_FAILURE;
  }
  XTime_GetTime(&end_time);
  t_sd2mem_end = end_time; // 记录 SD 卡到内存的结束时间

  // 加载比特流到FPGA
  XTime_GetTime(&start_time);
  t_mem2fpga_start = start_time; // 记录内存到FPGA的开始时间

  XGpio_DiscreteWrite(&GpioDecouple, 1, decouple_value);
  status = XFpga_BitStream_Load(&XFpgaInstance, (UINTPTR)PartialBit, KeyAddr,
                                fileSize, XFPGA_PARTIAL_EN);
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to load partial bitstream");
    cleanup_platform();
    return XST_FAILURE;
  }
  XGpio_DiscreteWrite(&GpioDecouple, 1, 0x0);
  XTime_GetTime(&end_time);
  t_mem2fpga_end = end_time; // 记录内存到FPGA的结束时间

  // 释放PartialBit内存
  free(PartialBit);

  return status;
}

// 设置 IP 参数
void Slot::set_ip_para(u32 buffer1, u32 buffer2, u32 buffer3, u32 buffer4,
                       u32 buffer5, u32 buffer6, u32 buffer7, u32 buffer8,
                       u32 buffer9, u32 buffer10, u32 buffer11, u32 buffer12,
                       int intBuffer1, int intBuffer2, int intBuffer3,
                       int intBuffer4, int intBuffer5) {

  Xil_Out32(data_addr + 0x10, buffer1);
  Xil_Out32(data_addr + 0x1C, buffer2);
  Xil_Out32(data_addr + 0x28, buffer3);
  Xil_Out32(data_addr + 0x34, buffer4);
  Xil_Out32(data_addr + 0x40, buffer5);
  Xil_Out32(data_addr + 0x4C, buffer6);
  Xil_Out32(data_addr + 0x58, buffer7);
  Xil_Out32(data_addr + 0x64, buffer8);
  Xil_Out32(data_addr + 0x70, buffer9);
  Xil_Out32(data_addr + 0x7C, buffer10);
  Xil_Out32(data_addr + 0x88, buffer11);
  Xil_Out32(data_addr + 0x94, buffer12);

  Xil_Out32(ctrl_addr + 0x18, (u32)intBuffer1);
  Xil_Out32(ctrl_addr + 0x20, (u32)intBuffer2);
  Xil_Out32(ctrl_addr + 0x28, (u32)intBuffer3);
  Xil_Out32(ctrl_addr + 0x30, (u32)intBuffer4);
  Xil_Out32(ctrl_addr + 0x38, (u32)intBuffer5);
}

// 启动槽
void Slot::Slot_Start() { Xil_Out32(ctrl_addr + 0x00, 0x1); }

// 检查槽状态
void Slot::checkSlotState() {
  uint32_t state = Xil_In32(ctrl_addr + 0x00);
  slotIsIdle = (state == 0x4);
}

// 获取槽号
int Slot::getSlotNr() const { return slotNr; }

SlotType Slot::getType() const { return type; }

// 判断是否空闲
bool Slot::isSlotIdle() const { return slotIsIdle; }

// Task 构造函数
Task::Task(std::string taskName, Application *taskAppname, u64 taskLatency,
           int taskParameter, int taskBatch, TaskType taskType, int taskIndex,
           double taskFactor)
    : name(taskName), appname(taskAppname), latency(taskLatency),
      parameter(taskParameter), type(taskType), batch(taskBatch),
      current_batch_index(0), last_completed_batch_index(-1), batch_infos(0),
      dependencies(0), IsDone(false), isScheduled(false), index(taskIndex),
      bitstream(NULL), slotNr(NULL), arrival_time(0), start_time(0),
      end_time(0), isBlocked(false), factor(taskFactor) {
  // 初始化批次信息向量
  batch_infos.resize(batch);
  for (auto &batch_info : batch_infos) {
    batch_info.isCompleted = false;
  }
}

// 获取任务名
std::string Task::getTaskName() const { return name; }

// 获取应用名
Application *Task::getAppName() const { return appname; }

u64 Task::getLatency() const { return latency; }

int Task::getParameter() const { return parameter; }

// 获取比特流文件名
char *Task::getBitName() {
  // 如果bitstream已经存在，直接返回
  if (bitstream != nullptr) {
    return bitstream;
  }

  // 生成比特流文件名
  std::string slotTypeStr = (slotNr->getType() == SLOT_S) ? "S" : "L";
  int slotID = slotNr->getSlotNr();
  std::string filename =
      name + "_" + slotTypeStr + std::to_string(slotID) + "_partial.bit";

  // 分配内存并复制字符串
  bitstream = new char[filename.length() + 1];
  std::strcpy(bitstream, filename.c_str());

  return bitstream;
}

bool Task::canStartBatch(int batch_index) {
  for (Task *depTask : dependencies) {
    int dep_max_batch_index = depTask->batch - 1;
    int required_batch_index = std::min(batch_index, dep_max_batch_index);
    if (depTask->last_completed_batch_index < required_batch_index) {
      return false;
    }
  }
  return true;
}

TaskType Task::getTaskType() const { return type; }

// Application 构造函数
Application::Application(std::string appName, int appBatch,
                         std::vector<Task *> appTaskList,
                         std::vector<Task *> appTaskList3in1)
    : name(appName), batch(appBatch), tasksList(appTaskList),
      tasksList_3in1(appTaskList3in1), arrival_time(0), start_time(0),
      end_time(0), parallelism_S(0),  // 并行任务数初始化为 0
      parallelism_L(0), threshold(0), // 阈值初始化为 0
      tasks_completed(0), task_extracted(false), // 已完成任务数初始化为 0
      bundled_3in1tasks(0) {}

// 获取应用名
std::string Application::getAppName() const { return name; }

// 添加任务
void Application::addTask(Task *task) { tasksList.push_back(task); }

void Application::addTask_3in1(Task *task) { tasksList_3in1.push_back(task); }

int Application::getBatch() const { return batch; }

void Application::defineDependencies() {
  // 假设每个任务依赖于前一个任务
  for (size_t i = 1; i < tasksList.size(); ++i) {
    Task *currentTask = tasksList[i];
    Task *prevTask = tasksList[i - 1];
    currentTask->dependencies.push_back(prevTask);
  }
}

void Application::defineDependencies3in1() {
  // 假设每个任务依赖于前一个任务
  for (size_t i = 1; i < tasksList_3in1.size(); ++i) {
    Task *currentTask = tasksList_3in1[i];
    Task *prevTask = tasksList_3in1[i - 1];
    currentTask->dependencies.push_back(prevTask);
  }
}

std::vector<Slot *> initializeSlots() {

  char *bitFile = nullptr; // 假设暂时没有比特流文件，后续可以更新
  Task *curTsk = nullptr;        // 暂时没有当前任务
  Application *curApp = nullptr; // 暂时没有当前应用

  // 使用 std::vector 来存储 Slot* 指针
  std::vector<Slot *> slots;

  // 初始化S型插槽
  for (int i = 0; i < 4; ++i) {
    slots.push_back(
        new Slot(SLOT_S, i + 1, 0, 0, 0, 0, 0, 0, 0, bitFile, curTsk, curApp));
  }

  // 初始化L型插槽
  for (int i = 0; i < 2; ++i) {
    slots.push_back(
        new Slot(SLOT_L, i + 1, 0, 0, 0, 0, 0, 0, 0, bitFile, curTsk, curApp));
  }

  // 循环运行每个 Slot 的 update_addr 和 get_decouple_value
  for (auto &slot : slots) {
    slot->update_addr();        // 为每个 Slot 赋值正确的地址
    slot->get_decouple_value(); // 更新 GPIO Decouple 值
  }

  return slots; // 返回 slots 向量
}

uintptr_t get_physical_address(void *virtual_address) {
  return (uintptr_t)virtual_address;
}

double next_event_delay(int min, int max) {
  int off_ms = rand() % max + min;
  return 1.0 * off_ms * 1000.0; // translate to us
}

// std::queue 用于保存应用程序生成顺序
std::queue<AppGenStruct> all_apps_queue;

//在此处预设跑分模型
void initialize_all_apps() {
  // 生成的 50 组 AppGenStruct
  // 格式：
  // {应用程序生成器
  // batch，parallelism_S，parallelism_L,priority，priority_level} 其中 priority
  // 和 priority_level 相等，取值为 1、3 或 9

  AppGenStruct app1 = {appgenerator_IC, 5, 3, 1, 1.0, 1};
  AppGenStruct app2 = {appgenerator_OF, 6, 2, 3, 1.0, 1};
  AppGenStruct app3 = {appgenerator_LENet, 3, 1, 9, 1.0, 1};
  AppGenStruct app4 = {appgenerator_3DR, 4, 2, 1, 1.0, 1};
  AppGenStruct app5 = {appgenerator_IC, 2, 4, 3, 1.0, 1};
  AppGenStruct app6 = {appgenerator_OF, 6, 3, 9, 1.0, 1};
  AppGenStruct app7 = {appgenerator_LENet, 4, 2, 1, 1.0, 1};
  AppGenStruct app8 = {appgenerator_3DR, 5, 1, 3, 1.0, 1};
  AppGenStruct app9 = {appgenerator_IC, 7, 4, 9, 1.0, 1};
  AppGenStruct app10 = {appgenerator_OF, 3, 1, 1, 1.0, 1};

  // 将生成的应用程序推入队列
  all_apps_queue.push(app1);
  all_apps_queue.push(app2);
  all_apps_queue.push(app3);
  all_apps_queue.push(app4);
  all_apps_queue.push(app5);
  all_apps_queue.push(app6);
  all_apps_queue.push(app7);
  all_apps_queue.push(app8);
  all_apps_queue.push(app9);
  all_apps_queue.push(app10);
}

///////////////////////////////////////////////////////////////////////////////////
// 调度核心算法
///////////////////////////////////////////////////////////////////////////////////

Application *add_new_event(std::list<Application *> &pending_apps) {
  // 检查是否还有预定的应用程序生成
  if (all_apps_queue.empty()) {
    // 队列为空，无法添加更多应用程序
    return NULL;
  }

  // 获取当前调度项
  AppGenStruct current_app = all_apps_queue.front();
  all_apps_queue.pop(); // 从队列中移除已处理的调度项

  // 使用生成函数创建一个新的 Application 对象
  // 注意：这里我们用的是 current_schedule.generator，它是函数指针
  Application *new_app = current_app.generator(
      current_app.batch, current_app.parallelism_S, current_app.parallelism_L,
      current_app.priority, current_app.priority_level);

  // 将生成的 Application 对象添加到 pending_apps 列表中
  pending_apps.push_back(new_app);
  return new_app;
}

double update_priorities_app(std::list<Application *> &pending_apps,
                             std::list<Application *> &candidate_apps) {
  // Get most up to date time
  u64 cur_time;
  XTime_GetTime(&cur_time);
  double max_tokens = 0;
  // Update tokens in pending_app queue
  for (auto it = pending_apps.begin(); it != pending_apps.end(); ++it) {
    Application *cur_app = *it;
    double current_tokens = cur_app->priority;
    int priority_class = cur_app->priority_level;
    u64 degradation = cur_time - cur_app->arrival_time;
    // Convert degradation to us
    double deg = 0.125 * 1.0 * degradation /
                 (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
    // Calculate latency of this application
    u64 latency = 0; // cur_task->latency*cur_task->batch; // Latencies are in
                     // us
    for (auto itr = cur_app->tasksList.begin(); itr != cur_app->tasksList.end();
         ++itr) {
      Task *cur_task = *itr;
      u64 lat = cur_task->getLatency();
      int batch = (cur_task->batch + 1);
      latency += lat * batch;
    }
    // 更新应用程序的优先级
    if (latency > 0) {
      cur_app->priority = current_tokens + priority_class * deg / latency;
    } else {
      // 如果 latency 为零，记录警告日志，并设置一个高优先级值
      char warning_message[200];
      snprintf(
          warning_message, sizeof(warning_message),
          "Application %s has zero latency. Setting priority to a high value.",
          cur_app->getAppName().c_str());
      print_log("WARNING", warning_message);
      cur_app->priority =
          std::numeric_limits<double>::max(); // 或者一个合理的高值
    }
    if (cur_app->priority > max_tokens) {
      max_tokens = cur_app->priority;
    }
  }
  // Update tokens in candidate apps
  for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it) {
    Application *cur_app = *it;
    double current_tokens = cur_app->priority;
    int priority_class = cur_app->priority_level;
    u64 degradation = cur_time - cur_app->arrival_time;
    // Convert degradation to us
    double deg = 0.125 * 1.0 * degradation /
                 (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
    // Calculate latency of this application
    u64 latency = 0; // cur_task->latency*cur_task->batch; // Latencies are in
                     // us
    for (auto itr = cur_app->tasksList.begin(); itr != cur_app->tasksList.end();
         ++itr) {
      Task *cur_task = *itr;
      u64 lat = cur_task->getLatency();
      int batch = (cur_task->batch + 1);
      latency += lat * batch;
    }
    // 更新应用程序的优先级
    if (latency > 0) {
      cur_app->priority = current_tokens + priority_class * deg / latency;
    } else {
      // 如果 latency 为零，记录警告日志，并设置一个高优先级值
      char warning_message[200];
      snprintf(
          warning_message, sizeof(warning_message),
          "Application %s has zero latency. Setting priority to a high value.",
          cur_app->getAppName().c_str());
      print_log("WARNING", warning_message);
      cur_app->priority =
          std::numeric_limits<double>::max(); // 或者一个合理的高值
    }
    if (cur_app->priority > max_tokens) {
      max_tokens = cur_app->priority;
    }
  }
  // TODO maybe add an extra priority level for old high priority applications
  // Give tokens to currently running tasks as well
  if (max_tokens > 9.0) {
    return 9.0;
  } else if (max_tokens > 3.0) {
    return 3.0;
  }
  return 1.0;
}

bool update_candidate_apps(std::list<Application *> &pending_apps,
                           std::list<Application *> &candidate_apps,
                           double threshold) {
  bool rethresh = false;
  for (auto it = pending_apps.begin(); it != pending_apps.end();) {
    Application *cur_app = *it;
    if (cur_app->priority >= threshold) {
      it = pending_apps.erase(it);
      candidate_apps.push_back(cur_app);
      rethresh = true;
      char log_message[200];
      snprintf(log_message, sizeof(log_message),
               "Moving application from pending to candidate with priority %f, "
               "threshold was %f",
               cur_app->priority, threshold);
      print_log("INFO", log_message);
    } else {
      ++it;
    }
  }
  // 第二个循环，同样添加打印
  for (auto it = candidate_apps.begin(); it != candidate_apps.end();) {
    Application *cur_app = *it;
    if (cur_app->priority < threshold) {
      it = candidate_apps.erase(it);
      // 暂时注释掉threshold的修改
      // cur_app->threshold = 0;
      pending_apps.push_back(cur_app);
      rethresh = true;
    } else {
      ++it;
    }
  }
  return rethresh;
}

void update_ready_tasks(std::list<Application *> &candidate_apps,
                        std::list<Task *> &ready_tasks) {
  // 遍历 candidate_apps 列表中的每个应用
  for (auto it = candidate_apps.begin(); it != candidate_apps.end();) {
    Application *app = *it;

    // 检查应用是否已经被标记为 beingadded
    if (app->task_extracted) {
      // 如果已经被标记为 beingadded，则跳过
      ++it;
      continue;
    }

    // 将应用中的任务移动到 ready_tasks
    for (Task *task : app->tasksList) {
      ready_tasks.push_back(task);
    }

    // 设置 beingadded 为 true
    app->task_extracted = true;

    ++it;
  }
}

std::list<Application *> retire_apps(std::list<Application *> &candidate_apps,
                                     std::list<Task *> &ready_tasks) {
  std::list<Application *> retired_apps; // 用于存储完成的应用

  // 遍历 candidate_apps 列表中的每个应用
  for (auto app_it = candidate_apps.begin(); app_it != candidate_apps.end();) {
    Application *app = *app_it;

    // 检查 app 是否完成：遍历 tasksList 和 tasksList_3in1，确保所有任务都已完成
    bool all_tasks_completed = true;

    // 检查 tasksList 中的所有任务
    for (Task *task : ready_tasks) {
      if (app == task->getAppName()) {
        if (!task->IsDone) {
          all_tasks_completed = false;
          break;
        }
      }
    }

    // 若所有任务已完成，将应用程序标记为完成并移至 retired_apps
    if (all_tasks_completed) {
      char retire_app_message[128];
      sprintf(retire_app_message, "Retiring App: %s, All Tasks Completed\n",
              app->getAppName().c_str());
      print_log("INFO", retire_app_message);

      // 将应用添加到 retired_apps 列表中
      retired_apps.push_back(app);
      app_it = candidate_apps.erase(app_it);

      // 如果需要，可以在此处进行额外的清理操作，例如释放内存
      // delete app; // 如果应用程序是通过 new 分配的，需要释放内存
    } else {
      // 如果应用程序还有未完成的任务，继续下一个应用
      ++app_it;
    }
  }

  // 返回已完成的应用列表
  return retired_apps;
}

void allocate_resources(std::list<Application *> &candidate_apps) {
  int rest_L = 2; // 大核剩余数量
  int rest_S = 4; // 小核剩余数量

  // 保存每个 app 的原始并行度 S 和 L 的目标值
  std::list<int> original_parallelism_S;
  std::list<int> original_parallelism_L;
  for (auto &app : candidate_apps) {
    original_parallelism_S.push_back(app->parallelism_S);
    original_parallelism_L.push_back(app->parallelism_L);
    app->parallelism_S = 0; // 初始化小核并行度
    app->parallelism_L = 0; // 初始化大核并行度
  }

  // 第一遍分配: 初始分配并计算等效剩余任务数
  auto orig_S_it = original_parallelism_S.begin();
  auto orig_L_it = original_parallelism_L.begin();
  for (auto &app : candidate_apps) {
    int tasks_left_total = app->tasksList.size() - app->tasks_completed;
    int tasks_left_large = tasks_left_total;

    // 大核分配: 如果任务数足够且有剩余大核
    if (tasks_left_large >= 3 && rest_L > 0) {
      app->parallelism_L = 1;
      tasks_left_large -= 3; // 大核等效减少3个任务
      rest_L--;
      (*orig_L_it)--; // 更新原始 L 值
    }

    // 小核分配: 所有 app 都分配一个小核
    if (rest_S > 0) {
      app->parallelism_S = 1;
      rest_S--;
      (*orig_S_it)--; // 更新原始 S 值
    }

    ++orig_S_it;
    ++orig_L_it;
  }

  // 第二遍分配: 基于原始目标并行度 S/L 分配，按等效剩余任务数判断
  orig_S_it = original_parallelism_S.begin();
  orig_L_it = original_parallelism_L.begin();
  for (auto &app : candidate_apps) {
    int tasks_left_large =
        app->tasksList.size() - app->tasks_completed - 3 * app->parallelism_L;
    int tasks_left_small =
        app->tasksList.size() - app->tasks_completed - app->parallelism_S;

    // 大核分配：符合原始并行度 L 的值且不超过等效剩余任务数
    if (tasks_left_large >= 3 && rest_L > 0 && *orig_L_it > 0) {
      int alloc_L = std::min(*orig_L_it, rest_L);
      app->parallelism_L += alloc_L;
      tasks_left_large -= 3 * alloc_L;
      rest_L -= alloc_L;
      *orig_L_it -= alloc_L;
    }

    // 小核分配：符合原始并行度 S 的值且不超过等效剩余任务数
    if (tasks_left_small > 0 && rest_S > 0 && *orig_S_it > 0) {
      int alloc_S = std::min(*orig_S_it, rest_S);
      app->parallelism_S += alloc_S;
      tasks_left_small -= alloc_S;
      rest_S -= alloc_S;
      *orig_S_it -= alloc_S;
    }

    ++orig_S_it;
    ++orig_L_it;

    if (rest_S == 0 && rest_L == 0)
      break;
  }

  // 第三遍分配：剩余大/小核，先到先得分配
  for (auto &app : candidate_apps) {
    int tasks_left_large =
        app->tasksList.size() - app->tasks_completed - 3 * app->parallelism_L;
    int tasks_left_small =
        app->tasksList.size() - app->tasks_completed - app->parallelism_S;

    // 分配剩余的大核
    while (tasks_left_large >= 3 && rest_L > 0) {
      app->parallelism_L += 1;
      tasks_left_large -= 3;
      rest_L--;
    }

    // 分配剩余的小核
    while (tasks_left_small > 0 && rest_S > 0) {
      app->parallelism_S += 1;
      tasks_left_small -= 1;
      rest_S--;
    }

    if (rest_S == 0 && rest_L == 0)
      break;
  }

  // 记录每个应用程序的最终并行度分配情况
  for (auto &app : candidate_apps) {
    char parallelism_log_message[128];
    snprintf(parallelism_log_message, sizeof(parallelism_log_message),
             "App: %s, Big Slot Parallelism: %d, Little Slot Parallelism: %d",
             app->getAppName().c_str(), app->parallelism_L, app->parallelism_S);
    print_log("INFO", parallelism_log_message);
  }
}

void bundle_ready_tasks(std::list<Task *> &ready_tasks) {
  std::map<Application *, std::vector<Task *>> app_tasks_map;
  for (Task *task : ready_tasks) {
    if (task->getTaskType() == TASK_S && task->slotNr == nullptr) {
      app_tasks_map[task->getAppName()].push_back(task);
    }
  }

  // 遍历每个 Application 的任务列表，按绑定条件创建 3in1 任务
  for (auto &app_pair : app_tasks_map) {
    Application *app = app_pair.first;
    std::vector<Task *> &tasks = app_pair.second;

    // 按 taskIndex 排序，确保绑定的任务是连续的
    std::sort(tasks.begin(), tasks.end(),
              [](Task *a, Task *b) { return a->index < b->index; });

    // 遍历每个任务，以 3 个为一组进行打包
    for (int i = tasks.size() - 1; i >= 2; --i) {
      Task *task3 = tasks[i];
      Task *task2 = tasks[i - 1];
      Task *task1 = tasks[i - 2];

      // 只有在满足绑定条件时创建3in1任务
      if ((task1->index % 3 == 0) && (task2->index == task1->index + 1) &&
          (task3->index == task1->index + 2)) {

        Task *bundled_task = nullptr;
        // 计算 tasksList_3in1 的索引，从末尾开始
        size_t total_bundles = app->tasksList_3in1.size() / 2;
        size_t bundle_index = total_bundles - 1 - app->bundled_3in1tasks;
        // 根据 factor 和 batch 判断是选择 BDQ_S 还是 BDQ_P
        if (task1->factor > app->getBatch()) {
          // 选择 `S` 类型的 3in1 任务
          bundled_task = app->tasksList_3in1[bundle_index * 2];
        } else {
          // 选择 `P` 类型的 3in1 任务
          bundled_task = app->tasksList_3in1[bundle_index * 2 + 1];
        }

        // 确保找到合适的 3in1 任务
        if (bundled_task == nullptr) {
          continue; // 跳过本次绑定
        }

        // 设置 3in1 大任务的依赖为第一个任务的依赖
        bundled_task->dependencies = task1->dependencies;
        // 替换 ready_tasks 中的三个任务为这个3in1任务
        auto task_it = std::find(ready_tasks.begin(), ready_tasks.end(), task1);

        // 删除第一个任务并更新迭代器
        task_it =
            ready_tasks.erase(task_it); // 删除 task1，task_it 现在指向 task2
        // 删除第二个任务并更新迭代器
        task_it =
            ready_tasks.erase(task_it); // 删除 task2，task_it 现在指向 task3
        // 删除第三个任务并更新迭代器
        task_it = ready_tasks.erase(
            task_it); // 删除 task3，task_it 现在指向下一个元素
        // 插入 3in1 打包任务
        ready_tasks.insert(task_it, bundled_task); // 在正确的位置插入打包任务

        // // 更新 3in1 大任务后的任务依赖
        // Update the dependencies of tasks after the bundled task
        auto next_task_it = task_it;
        while (next_task_it != ready_tasks.end()) {
          Task *next_task = *next_task_it;

          if (next_task->getAppName() == app) {
            auto &deps = next_task->dependencies;
            deps.erase(std::remove(deps.begin(), deps.end(), task1),
                       deps.end());
            deps.erase(std::remove(deps.begin(), deps.end(), task2),
                       deps.end());
            deps.erase(std::remove(deps.begin(), deps.end(), task3),
                       deps.end());

            if (std::find(deps.begin(), deps.end(), bundled_task) ==
                deps.end()) {
              deps.push_back(bundled_task);
            }
          } else {
            break;
          }

          ++next_task_it;
        }
        app->bundled_3in1tasks++;
        return;
      }
    }
  }
}

void update_slot_state(std::vector<Slot *> &slots) {
  for (Slot *slot : slots) {
    slot->checkSlotState(); // 更新 Slot 的空闲状态
  }
}

void execute_tasks(std::list<Task *> &taskQueue, std::vector<Slot *> &slots,
                   XGpio &GpioDecouple, XFpga &XFpgaInstance, uintptr_t buffer,
                   int &D_task_blocked, int &D_PR) {

  for (auto &slot : slots) {
    if (slot->isSlotIdle() && slot->curTask == nullptr &&
        slot->getType() == SLOT_L) {
      bundle_ready_tasks(taskQueue);
      //如果有大Slot空出来了，执行一次task3in1
    }
  }

  // 定义 SlotUsage 结构体来分别记录大小核的使用情况
  struct SlotUsage {
    int small_slots = 0; // 小核占用数
    int large_slots = 0; // 大核占用数
  };

  // 初始化应用到 Slot 占用数量的映射
  std::map<Application *, SlotUsage> app_slot_count;

  // 统计每个应用在大小核的 Slot 占用情况
  for (auto &slot : slots) {
    if (slot->curTask != nullptr) {
      auto &usage = app_slot_count[slot->curTask->getAppName()];
      if (slot->getType() == SLOT_S) {
        usage.small_slots++;
      } else {
        usage.large_slots++;
      }
    }
  }

  // 处理批次完成情况
  for (auto &slot : slots) {

    Task *task = slot->curTask;
    if (task != nullptr && slot->isSlotIdle()) {
      if (task->current_batch_index > 0) {
        int batch_index = task->current_batch_index - 1;

        if (batch_index >= 0 && !task->batch_infos[batch_index].isCompleted) {
          XTime_GetTime(&task->batch_infos[batch_index].end_time);
          task->batch_infos[batch_index].isCompleted = true;

          // 检查批次是否阻塞并更新 D_task_blocked
          u64 batch_duration = (task->batch_infos[batch_index].end_time -
                                task->batch_infos[batch_index].start_time) /
                               100;

          char batch_duration_log[128];
          snprintf(batch_duration_log, sizeof(batch_duration_log),
                   "Batch duration: %llu us, Task latency: %llu us",
                   batch_duration, task->getLatency());
          print_log("INFO", batch_duration_log);

          if (batch_duration > task->getLatency() + 250000 &&
              !task->isBlocked) {
            D_task_blocked++; // 批次阻塞时增加计数
            task->isBlocked = true; // 标记任务为阻塞状态，避免重复计数
          }

          const char *slot_type_str = (slot->getType() == SLOT_S) ? "S" : "L";
          char batch_end_message[128];
          sprintf(batch_end_message,
                  "App %s, Task %s, Batch %d finished at Slot %s%d",
                  task->getAppName()->getAppName().c_str(),
                  task->getTaskName().c_str(), batch_index + 1, slot_type_str,
                  slot->getSlotNr());
          print_log("INFO", batch_end_message);

          task->last_completed_batch_index = batch_index;
        }
      }

      // 检查任务是否还有剩余批次
      if (task->current_batch_index < task->batch) {
        // 检查下一个批次是否可以开始

        if (task->canStartBatch(task->current_batch_index)) {
          // 记录下一个批次的开始时间
          XTime_GetTime(
              &task->batch_infos[task->current_batch_index].start_time);

          // 启动下一个批次
          slot->Slot_Start();
          // 记录批次开始日志
          const char *slot_type_str = (slot->getType() == SLOT_S) ? "S" : "L";
          char batch_start_message[128];
          sprintf(batch_start_message,
                  "App %s, Task %s, Batch %d start at Slot %s%d",
                  task->getAppName()->getAppName().c_str(),
                  task->getTaskName().c_str(), task->current_batch_index + 1,
                  slot_type_str, slot->getSlotNr());
          print_log("INFO", batch_start_message);

          // **在这里递增 current_batch_index**
          task->current_batch_index++;
        }
      } else {
        // 任务所有批次已完成
        task->IsDone = true;
        task->getAppName()->tasks_completed++;
        // 记录任务完成日志
        const char *slot_type_str = (slot->getType() == SLOT_S) ? "S" : "L";
        char batch_start_message[128];
        sprintf(batch_start_message,
                "App %s, Task %s, Batch %d started at Slot %s%d",
                task->getAppName()->getAppName().c_str(),
                task->getTaskName().c_str(), task->current_batch_index + 1,
                slot_type_str, slot->getSlotNr());
        print_log("INFO", batch_start_message);

        // 从任务队列中移除任务
        taskQueue.remove(task);

        // 清除 Slot 的当前任务
        slot->curTask = nullptr;
        task->slotNr = nullptr;

        // 根据 Slot 类型更新应用程序的 Slot 占用计数
        // 完成了就释放
        SlotUsage &usage = app_slot_count[task->getAppName()];
        if (slot->getType() == SLOT_S) {
          usage.small_slots--;
        } else {
          usage.large_slots--;
        }
      }
    }
  }

  // 遍历任务队列并调度任务
  for (Task *task : taskQueue) {

    if (task->IsDone || task->isScheduled) {
      continue;
    }

    Application *app = task->getAppName();
    auto &usage = app_slot_count[app];

    // 检查小核和大核的占用情况
    if (usage.small_slots >= app->parallelism_S &&
        usage.large_slots >= app->parallelism_L) {
      continue; // 小核和大核的并行度都已达到上限，跳过任务
    }

    // 检查任务类型并查找合适的 Slot
    Slot *available_slot = nullptr;
    if (task->type == TASK_S) { // 小任务
      // 查找空闲的小 Slot
      for (auto &slot : slots) {
        if (slot->isSlotIdle() && slot->curTask == nullptr &&
            slot->getType() == SLOT_S) {
          available_slot = slot;
          break;
        }
      }
    } else if (task->type == TASK_L) { // 大任务
      // 查找空闲的大 Slot
      for (auto &slot : slots) {
        if (slot->isSlotIdle() && slot->curTask == nullptr &&
            slot->getType() == SLOT_L) {
          available_slot = slot;
          bundle_ready_tasks(taskQueue);
          // 如果有大 Slot 空出来了，执行一次 task3in1
          break;
        }
      }
    }

    // 检查是否找到合适的 Slot
    if (available_slot == nullptr) {
      // 没有找到合适的 Slot，跳过该任务
      continue;
    }

    // 分配任务到 Slot 并更新状态
    task->isScheduled = true;
    available_slot->curTask = task;
    task->slotNr = available_slot;
    //不需要赋值SlotType，因为本身就是根据TASK_S/L找的Slot

    // 更新 SlotUsage 计数
    if (available_slot->getType() == SLOT_S) {
      usage.small_slots++;
    } else {
      usage.large_slots++;
    }

    // 加载比特流文件
    task->getBitName();
    available_slot->get_bitFile_name();

    // 执行部分重配置
    int status = available_slot->Slot_PR(GpioDecouple, XFpgaInstance);
    D_PR++;

    if (status != XST_SUCCESS) {
      // 处理错误
      char error_message[128];
      sprintf(error_message, "Failed to load bitstream to memory for task:%s",
              task->getTaskName().c_str());
      print_log("ERROR", error_message);

      // 标记任务为已完成以避免阻塞
      task->IsDone = true;
      available_slot->curTask = nullptr;
      SlotUsage &usage = app_slot_count[task->getAppName()];
      if (task->getTaskType() == TASK_S) {
        usage.small_slots--;
      } else {
        usage.large_slots--;
      }
      continue;
    }

    int parameter = task->getParameter();

    // 设置 IP 参数（假设这些是硬件相关的配置参数）
    available_slot->set_ip_para((u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                parameter, parameter, parameter, parameter, 0);
  }
}
