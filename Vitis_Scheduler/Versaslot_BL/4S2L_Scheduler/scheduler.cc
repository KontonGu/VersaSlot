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
#include <sys/_stdint.h>
#include <vector>
#include <xil_io.h>
#include <xil_printf.h>

// 创建全局日志容器
std::vector<LogEntry *> log_entries;

// 全局变量，用于时间戳
extern uint64_t program_start_time;
extern volatile PR_Task *pr_task;

// 修改后的 print_log 函数
void print_log(const char *level, const char *message) {
  XTime current_time;
  XTime_GetTime(&current_time);
  u64 elapsed_time_us = ((current_time - program_start_time) * 1000000) /
                        XPAR_CPU_TIMESTAMP_CLK_FREQ;

  // 在堆上分配新的 LogEntry 实例
  LogEntry *entry = new LogEntry();
  entry->timestamp = elapsed_time_us;
  entry->level = level;
  entry->message = message;

  log_entries.push_back(entry); // 将指针加入 log_entries
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
      BitstreamFile(BitFile), curTask(curTsk), curApp(CurApp),
      partial_reconfigured(false) {}

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
uint32_t Slot::get_decouple_value() {
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
  return decouple_value;
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

  {
    const char *slot_type_str = (type == SLOT_S) ? "S" : "L";
    // 打印日志信息，包含Slot类型和编号
    char sd2mm_start_message[128];
    sprintf(sd2mm_start_message, "Slot %s%d starting PR", slot_type_str,
            slotNr);
    print_log("INFO", sd2mm_start_message);
  }

  status = load_bitstream_to_memory(BitstreamFile, &PartialBit,
                                    &fileSize); // 传递 PartialBit 的地址
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to load bitstream to memory");
    cleanup_platform();
    return XST_FAILURE;
  }
  XTime_GetTime(&end_time);
  t_sd2mem_end = end_time; // 记录 SD 卡到内存的结束时间

  //   // 打印 SD 卡到内存传输的开始和结束时间
  //   {
  //     const char *slot_type_str = (type == SLOT_S) ? "S" : "L";
  //     // 打印日志信息，包含Slot类型和编号
  //     char sd2mm_end_message[128];
  //     sprintf(sd2mm_end_message, "Slot %s%d: t_sd2mm_end", slot_type_str,
  //     slotNr); print_log("INFO", sd2mm_end_message);
  //   }

  // 加载比特流到FPGA
  XTime_GetTime(&start_time);
  t_mem2fpga_start = start_time; // 记录内存到FPGA的开始时间

  //   {
  //     const char *slot_type_str = (type == SLOT_S) ? "S" : "L";
  //     // 打印日志信息，包含Slot类型和编号
  //     char mem2fpga_start_message[128];
  //     sprintf(mem2fpga_start_message, "Slot %s%d: t_mem2fpga_start",
  //             slot_type_str, slotNr);
  //     print_log("INFO", mem2fpga_start_message);
  //   }

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

  // 打印内存到FPGA传输的开始和结束时间
  {
    const char *slot_type_str = (type == SLOT_S) ? "S" : "L";
    // 打印日志信息，包含Slot类型和编号
    char mem2fpga_end_message[128];
    sprintf(mem2fpga_end_message, "Slot %s%d finished PR", slot_type_str,
            slotNr);
    print_log("INFO", mem2fpga_end_message);
  }

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
      dependencies(0), dependencies3in1P(0), dependencies3in1S(0),
      IsDone(false), isScheduled(false), index(taskIndex), bitstream(NULL),
      slotNr(NULL), arrival_time(0), start_time(0), end_time(0),
      isBlocked(false), factor(taskFactor) {
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
  // 检查大任务 (TASK_L)
  if (this->getTaskType() == TASK_L) {
    if (this->index == 1) { // P 类型任务
      for (Task *depTask : dependencies3in1P) {
        int dep_max_batch_index = depTask->batch - 1;
        int required_batch_index = std::min(batch_index, dep_max_batch_index);
        if (depTask->last_completed_batch_index < required_batch_index) {
          return false;
        }
      }
    } else if (this->index == 0) { // S 类型任务
      for (Task *depTask : dependencies3in1S) {
        int dep_max_batch_index = depTask->batch - 1;
        int required_batch_index = std::min(batch_index, dep_max_batch_index);
        if (depTask->last_completed_batch_index < required_batch_index) {
          return false;
        }
      }
    }
  } else { // 检查小任务的 dependencies
    for (Task *depTask : dependencies) {
      int dep_max_batch_index = depTask->batch - 1;
      int required_batch_index = std::min(batch_index, dep_max_batch_index);
      if (depTask->last_completed_batch_index < required_batch_index) {
        return false;
      }
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
      end_time(0), parallelism_S(0), // 并行任务数初始化为 0
      parallelism_L(0), optimal_S(0), optimal_L(0),
      threshold(0),                              // 阈值初始化为 0
      tasks_completed(0), task_extracted(false), // 已完成任务数初始化为 0
      bundled_3in1tasks(0), isAllocated(false), isAppStart(false) {}

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
  // 分别处理 P 和 S 类型任务的依赖
  Task *prevPTask = nullptr;
  Task *prevSTask = nullptr;

  for (size_t i = 0; i < tasksList_3in1.size(); ++i) {
    Task *currentTask = tasksList_3in1[i];

    // 如果是 P 类型任务，依赖前一个 P 类型任务
    if (currentTask->index == 1) { // factor == 1 表示 P 类型任务
      if (prevPTask != nullptr) {
        currentTask->dependencies3in1P.push_back(prevPTask);
      }
      prevPTask = currentTask; // 更新最新的 P 类型任务
    }

    // 如果是 S 类型任务，依赖前一个 S 类型任务
    if (currentTask->index == 0) { // factor == 0 表示 S 类型任务
      if (prevSTask != nullptr) {
        currentTask->dependencies3in1S.push_back(prevSTask);
      }
      prevSTask = currentTask; // 更新最新的 S 类型任务
    }
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
    if (pr_task->slot_nr == slot->getSlotNr() &&
        pr_task->slot_type == slot->getType() &&
        (pr_task->pr_status == 2 || pr_task->pr_status == 4)) {
      continue; // 跳过此 slot 的状态更新
    }
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
// void initialize_all_apps() {
//   // 生成的 20 组 AppGenStruct
//   AppGenStruct app1 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   AppGenStruct app2 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
//   AppGenStruct app3 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app4 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   AppGenStruct app5 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   AppGenStruct app6 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app7 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
//   AppGenStruct app8 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   AppGenStruct app9 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app10 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   AppGenStruct app11 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app12 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   AppGenStruct app13 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app14 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   AppGenStruct app15 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app16 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   AppGenStruct app17 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   AppGenStruct app18 = {appgenerator_3DR, 20, 2, 1, 9.0, 9};
//   AppGenStruct app19 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   AppGenStruct app20 = {appgenerator_LENet, 20, 2, 1, 9.0, 9};
//   //   AppGenStruct app1 = {appgenerator_OF, 20, 1, 2, 9.0, 9};
//   //   AppGenStruct app2 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app3 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app4 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   //   AppGenStruct app5 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app6 = {appgenerator_OF, 20, 1, 2, 9.0, 9};
//   //   AppGenStruct app7 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app8 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app9 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   //   AppGenStruct app10 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app11 = {appgenerator_OF, 20, 1, 2, 6.0, 6};
//   //   AppGenStruct app12 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app13 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   //   AppGenStruct app14 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app15 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app16 = {appgenerator_OF, 20, 1, 2, 6.0, 6};
//   //   AppGenStruct app17 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
//   //   AppGenStruct app18 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
//   //   AppGenStruct app19 = {appgenerator_OF, 20, 1, 2, 6.0, 6};
//   //   AppGenStruct app20 = {appgenerator_IC, 20, 3, 2, 9.0, 9};

//   // 将生成的应用程序推入队列
//   all_apps_queue.push(app1);
//   all_apps_queue.push(app2);
//   all_apps_queue.push(app3);
//   all_apps_queue.push(app4);
//   all_apps_queue.push(app5);
//   all_apps_queue.push(app6);
//   all_apps_queue.push(app7);
//   all_apps_queue.push(app8);
//   all_apps_queue.push(app9);
//   all_apps_queue.push(app10);
//   all_apps_queue.push(app11);
//   all_apps_queue.push(app12);
//   all_apps_queue.push(app13);
//   all_apps_queue.push(app14);
//   all_apps_queue.push(app15);
//   all_apps_queue.push(app16);
//   all_apps_queue.push(app17);
//   all_apps_queue.push(app18);
//   all_apps_queue.push(app19);
//   all_apps_queue.push(app20);
// }

void initialize_all_apps() {
  // 生成的应用程序
  //   AppGenStruct app1 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app2 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app3 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app4 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app5 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app6 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app7 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app8 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app9 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app10 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app11 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app12 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app13 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app14 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app15 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app16 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app17 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app18 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app19 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app20 = {appgenerator_OF, 20, 1, 2, 3.0, 3};

  //   AppGenStruct app21 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app22 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app23 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app24 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app25 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app26 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app27 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app28 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app29 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app30 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app31 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app32 = {appgenerator_OF, 20, 1, 2, 3.0, 3};
  //   AppGenStruct app33 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
  //   AppGenStruct app34 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
  //   AppGenStruct app35 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
  //   AppGenStruct app36 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
  //   AppGenStruct app37 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};
  //   AppGenStruct app38 = {appgenerator_3DR, 20, 2, 2, 9.0, 9};
  //   AppGenStruct app39 = {appgenerator_IC, 20, 3, 2, 9.0, 9};
  //   AppGenStruct app40 = {appgenerator_LENet, 20, 2, 2, 9.0, 9};

  //   AppGenStruct app41 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  //   AppGenStruct app42 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  //   AppGenStruct app43 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  //   AppGenStruct app44 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  //   AppGenStruct app45 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app46 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app47 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app48 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app49 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app50 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app51 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app52 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app53 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app54 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app55 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app56 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app57 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app58 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app59 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app60 = {appgenerator_IC, 10, 3, 2, 9.0, 9};

  //   AppGenStruct app61 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app62 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app63 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app64 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app65 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app66 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app67 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app68 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app69 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app70 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app71 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app72 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app73 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app74 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app75 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app76 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app77 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app78 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  //   AppGenStruct app79 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  //   AppGenStruct app80 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};

  //   AppGenStruct app81 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app82 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app83 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app84 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app85 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app86 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app87 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app88 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app89 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app90 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app91 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app92 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app93 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app94 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app95 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app96 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app97 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app98 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  //   AppGenStruct app99 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  //   AppGenStruct app100 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app1 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app2 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app3 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app4 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app5 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app6 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app7 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app8 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app9 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  AppGenStruct app10 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app11 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  AppGenStruct app12 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app13 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app14 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app15 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app16 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app17 = {appgenerator_OF, 10, 1, 2, 3.0, 3};
  AppGenStruct app18 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app19 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app20 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  AppGenStruct app21 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app22 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app23 = {appgenerator_3DR, 10, 2, 2, 9.0, 9};
  AppGenStruct app24 = {appgenerator_IC, 10, 3, 2, 9.0, 9};
  AppGenStruct app25 = {appgenerator_LENet, 10, 2, 2, 9.0, 9};
  AppGenStruct app26 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app27 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app28 = {appgenerator_OF, 5, 1, 2, 3.0, 3};
  AppGenStruct app29 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app30 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app31 = {appgenerator_OF, 5, 1, 2, 3.0, 3};
  AppGenStruct app32 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app33 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app34 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  AppGenStruct app35 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app36 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app37 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  AppGenStruct app38 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app39 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app40 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  AppGenStruct app41 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app42 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};
  AppGenStruct app43 = {appgenerator_3DR, 5, 2, 2, 9.0, 9};
  AppGenStruct app44 = {appgenerator_IC, 5, 3, 2, 9.0, 9};
  AppGenStruct app45 = {appgenerator_LENet, 5, 2, 2, 9.0, 9};

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
  all_apps_queue.push(app11);
  all_apps_queue.push(app12);
  all_apps_queue.push(app13);
  all_apps_queue.push(app14);
  all_apps_queue.push(app15);
  all_apps_queue.push(app16);
  all_apps_queue.push(app17);
  all_apps_queue.push(app18);
  all_apps_queue.push(app19);
  all_apps_queue.push(app20);
  all_apps_queue.push(app21);
  all_apps_queue.push(app22);
  all_apps_queue.push(app23);
  all_apps_queue.push(app24);
  all_apps_queue.push(app25);
  all_apps_queue.push(app26);
  all_apps_queue.push(app27);
  all_apps_queue.push(app28);
  all_apps_queue.push(app29);
  all_apps_queue.push(app30);
  all_apps_queue.push(app31);
  all_apps_queue.push(app32);
  all_apps_queue.push(app33);
  all_apps_queue.push(app34);
  all_apps_queue.push(app35);
  all_apps_queue.push(app36);
  all_apps_queue.push(app37);
  all_apps_queue.push(app38);
  all_apps_queue.push(app39);
  all_apps_queue.push(app40);
  all_apps_queue.push(app41);
  all_apps_queue.push(app42);
  all_apps_queue.push(app43);
  all_apps_queue.push(app44);
  all_apps_queue.push(app45);
  //   all_apps_queue.push(app46);
  //   all_apps_queue.push(app47);
  //   all_apps_queue.push(app48);
  //   all_apps_queue.push(app49);
  //   all_apps_queue.push(app50);
  //   all_apps_queue.push(app51);
  //   all_apps_queue.push(app52);
  //   all_apps_queue.push(app53);
  //   all_apps_queue.push(app54);
  //   all_apps_queue.push(app55);
  //   all_apps_queue.push(app56);
  //   all_apps_queue.push(app57);
  //   all_apps_queue.push(app58);
  //   all_apps_queue.push(app59);
  //   all_apps_queue.push(app60);
  //   all_apps_queue.push(app61);
  //   all_apps_queue.push(app62);
  //   all_apps_queue.push(app63);
  //   all_apps_queue.push(app64);
  //   all_apps_queue.push(app65);
  //   all_apps_queue.push(app66);
  //   all_apps_queue.push(app67);
  //   all_apps_queue.push(app68);
  //   all_apps_queue.push(app69);
  //   all_apps_queue.push(app70);
  //   all_apps_queue.push(app71);
  //   all_apps_queue.push(app72);
  //   all_apps_queue.push(app73);
  //   all_apps_queue.push(app74);
  //   all_apps_queue.push(app75);
  //   all_apps_queue.push(app76);
  //   all_apps_queue.push(app77);
  //   all_apps_queue.push(app78);
  //   all_apps_queue.push(app79);
  //   all_apps_queue.push(app80);
  //   all_apps_queue.push(app81);
  //   all_apps_queue.push(app82);
  //   all_apps_queue.push(app83);
  //   all_apps_queue.push(app84);
  //   all_apps_queue.push(app85);
  //   all_apps_queue.push(app86);
  //   all_apps_queue.push(app87);
  //   all_apps_queue.push(app88);
  //   all_apps_queue.push(app89);
  //   all_apps_queue.push(app90);
  //   all_apps_queue.push(app91);
  //   all_apps_queue.push(app92);
  //   all_apps_queue.push(app93);
  //   all_apps_queue.push(app94);
  //   all_apps_queue.push(app95);
  //   all_apps_queue.push(app96);
  //   all_apps_queue.push(app97);
  //   all_apps_queue.push(app98);
  //   all_apps_queue.push(app99);
  //   all_apps_queue.push(app100);
}

///////////////////////////////////////////////////////////////////////////////////
// 调度核心算法
///////////////////////////////////////////////////////////////////////////////////

Application *add_new_event(std::list<Application *> &pending_apps,
                           int app_cnt) {
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
      current_app.batch, current_app.optimal_S, current_app.optimal_L,
      current_app.priority, current_app.priority_level);
  new_app->unique_ID = app_cnt;

  // 将生成的 Application 对象添加到 pending_apps 列表中
  pending_apps.push_back(new_app);
  char add_app_message[128];
  sprintf(add_app_message, "App ID: %d, App: %s, Added to pending apps",
          new_app->unique_ID, new_app->getAppName().c_str());
  print_log("INFO", add_app_message);
  return new_app;
}

double update_priorities_app(std::list<Application *> &pending_apps,
                             std::list<Application *> &candidate_apps) {
  // 获取最新的时间
  u64 cur_time;
  XTime_GetTime(&cur_time);
  double max_tokens = 0;
  // 更新 pending_app 队列中的 tokens
  for (auto it = pending_apps.begin(); it != pending_apps.end(); ++it) {
    Application *cur_app = *it;
    double current_tokens = cur_app->priority;
    int priority_class = cur_app->priority_level;
    u64 degradation = cur_time - cur_app->arrival_time;
    // 将 degradation 转换为微秒
    double deg = 0.125 * 1.0 * degradation /
                 (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
    // 计算此应用程序的延迟
    u64 latency = 0;
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
  // 更新 candidate apps 中的 tokens
  for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it) {
    Application *cur_app = *it;
    double current_tokens = cur_app->priority;
    int priority_class = cur_app->priority_level;
    u64 degradation = cur_time - cur_app->arrival_time;
    // 将 degradation 转换为微秒
    double deg = 0.125 * 1.0 * degradation /
                 (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
    // 计算此应用程序的延迟
    u64 latency = 0;
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
  // 返回适当的阈值
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
  //   // 防止pending app中的app过长等待，即使有SLOT 可用
  //   bool isAllAppsStarted = true;

  //   for (Application *app : candidate_apps) {
  //     if (!app->isAppStart) {
  //       isAllAppsStarted = false;
  //       break;
  //     }
  //   }

  //   int slot_s_used = 0;
  //   int slot_l_used = 0;

  //   if (isAllAppsStarted) {
  //     for (Application *app : candidate_apps) {
  //       slot_s_used += app->parallelism_S;
  //       slot_l_used += app->parallelism_L;
  //     }
  //     if (slot_l_used < 2 && slot_s_used < 4) {
  //       pending_apps.front()->priority = 9;
  //       pending_apps.front()->priority_level = 9;
  //     }
  //   }

  bool rethresh = false;
  for (auto it = pending_apps.begin(); it != pending_apps.end();) {
    Application *cur_app = *it;
    if (cur_app->priority >= threshold) {
      it = pending_apps.erase(it);
      candidate_apps.push_back(cur_app);
      rethresh = true;
      char log_message[200];
      snprintf(
          log_message, sizeof(log_message),
          "Moving application %s from pending to candidate with priority %f, "
          "threshold was %f",
          cur_app->getAppName().c_str(), cur_app->priority, threshold);
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

    // 获取已完成的任务数量
    int completed_tasks = app->tasks_completed;
    int total_tasks = app->tasksList.size();

    // 若所有任务已完成，将应用程序标记为完成并移至 retired_apps
    if (all_tasks_completed) {
      char retire_app_message[128];
      sprintf(retire_app_message,
              "App ID: %d, Retiring  App: %s, Completed Tasks: %d/%d\n",
              app->unique_ID, app->getAppName().c_str(), completed_tasks,
              total_tasks);
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

void allocate_resources(std::list<Application *> &candidate_apps,
                        std::vector<Slot *> &slots,
                        std::list<Task *> ready_tasks) {

  //   xil_printf("task in task list:\n");
  //   for (Task *tsk : ready_tasks) {
  //     xil_printf("%s, ", tsk->getTaskName().c_str());
  //   }
  //   xil_printf("\n");

  int usage_s = 0;
  int usage_l = 0;

  // 统计使用中的小核和大核
  for (auto &slot : slots) {
    if (slot->curTask != nullptr) {
      //   xil_printf("Slot Type: %s, Task Name: %s, Task Index: %d\n",
      //              slot->getType() == SLOT_S ? "SLOT_S" : "SLOT_L",
      //              slot->curTask->getTaskName().c_str(),
      //              slot->curTask->index);

      if (slot->getType() == SLOT_S) {
        usage_s++;
      } else {
        usage_l++;
      }
    }
  }

  //   xil_printf("slot usage L: %d, S: %d\n", usage_l, usage_s);

  int slotL_already_allocated = 0;
  //   int slotS_already_allocated = 0;

  //   // 统计已经分配的资源
  //   for (auto &app : candidate_apps) {
  //     slotL_already_allocated += app->parallelism_L;
  //     //   slotS_already_allocated += app->parallelism_S;
  //   }

  for (Task *task : ready_tasks) {
    if (task->IsDone) {
      continue;
    }
    if (task->getTaskType() == TASK_L) {
      slotL_already_allocated += 1;
    }
  }

  int rest_L = 2 - slotL_already_allocated; // 大核剩余数量
  //   int rest_S = 4 - usage_s - slotS_already_allocated; // 小核剩余数量

  //   int rest_L = 2 - usage_l; // 大核剩余数量
  int rest_S = 4 - usage_s; // 小核剩余数量

  // TODO 只能跑两个大核一次

  // 如果没有空余S/L slot直接跳过函数
  if (rest_L <= 0 && rest_S <= 0) {
    return;
  }

  for (auto &app : candidate_apps) {
    if (usage_l == 0 && app->isAllocated == 2 && !app->isAppStart) {
      app->isAllocated = 0;
      app->parallelism_L = 0;
      app->parallelism_S = 0;
      break;
    }
    if (usage_l == 1 && app->isAllocated == 2 && !app->isAppStart &&
        app->tasksList.size() <= 6) {
      app->isAllocated = 0;
      app->parallelism_L = 0;
      app->parallelism_S = 0;
      break;
    }
  }

  int slot_left_S = 4;

  for (auto &app : candidate_apps) {

    int total_task = app->tasksList.size();

    // 如果应用已经分配到大核跳过
    if (app->isAllocated == 1) {
      continue;
    }

    // 分配大核: 如果任务数足够且有剩余大核
    if (app->isAllocated == 0 && total_task >= 3 && rest_L > 0) {
      app->parallelism_L = app->optimal_L;
      // 大核分配后，不再分配小核
      app->parallelism_S = 0; // 确保小核为0
      app->isAllocated = 1;   // 标记应用已分配
      rest_L -= 1;
      continue; // 跳过小核分配
    }

    int task_left = app->tasksList.size() - app->tasks_completed;
    // 未分配/已分配小核的App 可以到这
    if (rest_S > 0 && !app->isAllocated) {
      app->parallelism_L = 0; // 确保大核为0
      app->parallelism_S = std::min(app->optimal_S, task_left);
      slot_left_S -= app->parallelism_S;
      slot_left_S = std::max(0, slot_left_S);
      app->isAllocated = 2;
    }
  }

  // 尽可能分配剩余的小核
  if (slot_left_S > 0) {
    for (auto &app : candidate_apps) {
      if (app->isAllocated == 1) {
        continue;
      }

      // 只有小核task能到这

      int task_left = app->tasksList.size() - app->tasks_completed;
      int slot_diff = task_left - app->parallelism_S;

      if (slot_diff > 0) {
        app->parallelism_S += slot_diff;
        slot_left_S -= slot_diff;
      }
      if (slot_left_S <= 0) {
        break;
      }
    }
  }

  //   // 打印每个应用的分配结果
  //   for (Application *app : candidate_apps) {
  //     xil_printf("App:%s, S: %d, L: %d, isAllocated: %d\n",
  //                app->getAppName().c_str(), app->parallelism_S,
  //                app->parallelism_L, (app->isAllocated == true) ? 1 : 0);
  //   }
}

void bundle_ready_tasks(std::list<Task *> &ready_tasks) {
  std::map<Application *, std::vector<Task *>> app_tasks_map;

  // 将每个 Task 按其父 Application 分类
  for (Task *task : ready_tasks) {
    if (task->getTaskType() == TASK_S && task->slotNr == nullptr) {
      app_tasks_map[task->getAppName()].push_back(task);
    }
  }

  // 遍历每个 Application 的任务列表，按绑定条件创建 3in1 任务
  for (auto &app_pair : app_tasks_map) {
    Application *app = app_pair.first;
    std::vector<Task *> &tasks = app_pair.second;

    // 检查 Application 的 parallelism_L 是否不为0
    if (app->parallelism_L == 0) {
      continue; // 如果大核并行度为0，则跳过该 Application
    }

    // 按 taskIndex 排序，确保绑定的任务是连续的
    std::sort(tasks.begin(), tasks.end(),
              [](Task *a, Task *b) { return a->index < b->index; });

    // 遍历每个任务，以 3 个为一组进行打包
    while (tasks.size() >= 3) { // 确保还有足够的任务可以打包
      Task *task3 = tasks.back();
      Task *task2 = tasks[tasks.size() - 2];
      Task *task1 = tasks[tasks.size() - 3];

      // 只有在满足绑定条件时创建3in1任务
      if ((task1->index % 3 == 0) && (task2->index == task1->index + 1) &&
          (task3->index == task1->index + 2)) {

        Task *bundled_task = nullptr;
        // 计算 tasksList_3in1 的索引，从末尾开始
        size_t total_bundles = app->tasksList_3in1.size() / 2;
        size_t bundle_index = total_bundles - 1 - app->bundled_3in1tasks;
        // 根据 factor 和 batch 判断是选择 BDQ_S 还是 BDQ_P
        if (task1->factor > app->getBatch()) {
          // 选择 S 类型的 3in1 任务
          bundled_task = app->tasksList_3in1[bundle_index * 2];
        } else {
          // 选择 P 类型的 3in1 任务
          bundled_task = app->tasksList_3in1[bundle_index * 2 + 1];
        }

        // 确保找到合适的 3in1 任务
        if (bundled_task == nullptr) {
          tasks.pop_back();
          tasks.pop_back();
          tasks.pop_back();
          continue; // 跳过本次绑定
        }

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

        app->bundled_3in1tasks++;
      }
      // 移除已经打包的三个任务
      tasks.pop_back();
      tasks.pop_back();
      tasks.pop_back();
    }
  }
}

void update_slot_state(std::vector<Slot *> &slots) {
  for (Slot *slot : slots) {
    if (pr_task->slot_nr == slot->getSlotNr() &&
        pr_task->slot_type == slot->getType() &&
        (pr_task->pr_status == 2 || pr_task->pr_status == 4)) {
      continue; // 跳过此 slot 的状态更新
    }
    slot->checkSlotState(); // 更新 Slot 的空闲状态
  }
}

void execute_tasks(std::list<Task *> &taskQueue, std::vector<Slot *> &slots,
                   uintptr_t buffer, int &D_task_blocked, int &D_PR,
                   XTime last_pr_start, XTime last_pr_finished) {
  //   xil_printf("run execute task\n");
  //   bool isSlotLfree = false;

  // 定义 SlotUsage 结构体来分别记录大小核的使用情况
  struct SlotUsage {
    int small_slots = 0; // 小核占用数
    int large_slots = 0; // 大核占用数
  };

  // 初始化应用到 Slot 占用数量的映射
  std::map<Application *, SlotUsage> app_slot_count;

  // 统计每个应用在大小核的 Slot 占用情况
  for (auto &slot : slots) {
    // // 判断是否当前 PR 任务涉及到当前 slot 且处于执行中或异常状态
    // if (pr_task->slot_nr == slot->getSlotNr() &&
    //     pr_task->slot_type == slot->getType() &&
    //     (pr_task->pr_status == 2 || pr_task->pr_status == 4)) {
    //   continue; // 跳过此 slot 的状态更新
    // }
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

    update_slot_state(slots);

    if (pr_task->slot_nr == slot->getSlotNr() &&
        pr_task->slot_type == slot->getType() &&
        (pr_task->pr_status == 2 || pr_task->pr_status == 4)) {
      continue; // 跳过此 slot 的状态更新
    }

    // if (slot->isSlotIdle() && slot->curTask == nullptr &&
    //     slot->getType() == SLOT_L) {
    //   isSlotLfree = true;
    //   //如果有大Slot空出来了，执行一次task3in1
    // }

    if (slot->partial_reconfigured == false) {
      continue;
    }

    Task *task = slot->curTask;
    if (task != nullptr && slot->isSlotIdle()) {

      //   xil_printf("Task: %s current batch index: %d\n",
      //              task->getTaskName().c_str(), task->current_batch_index);

      if (task->current_batch_index > 0) {
        int batch_index = task->current_batch_index - 1;

        if (batch_index >= 0 && !task->batch_infos[batch_index].isCompleted) {
          XTime_GetTime(&task->batch_infos[batch_index].end_time);
          task->batch_infos[batch_index].isCompleted = true;

          const char *slot_type_str = (slot->getType() == SLOT_S) ? "S" : "L";
          char batch_end_message[128];
          sprintf(batch_end_message,
                  "App %s, Task %s, Batch %d finished at Slot %s%d",
                  task->getAppName()->getAppName().c_str(),
                  task->getTaskName().c_str(), batch_index + 1, slot_type_str,
                  slot->getSlotNr());
          print_log("INFO", batch_end_message);

          //   xil_printf("App %s, Task %s, Batch %d finished at Slot %s%d\n",
          //              task->getAppName()->getAppName().c_str(),
          //              task->getTaskName().c_str(), batch_index + 1,
          //              slot_type_str, slot->getSlotNr());

          task->last_completed_batch_index = batch_index;
        }
      }

      // 检查任务是否还有剩余批次
      if (task->current_batch_index < task->batch) {
        // 检查下一个批次是否可以开始

        // xil_printf("Task %s before canStartTask check\n",
        //            task->getAppName()->getAppName().c_str());

        // if (task->getTaskType() == TASK_L) {
        //   xil_printf("Task: %s (Large Task)\n", task->getTaskName().c_str());

        //   // 根据任务类型选择打印的依赖列表
        //   if (task->index == 1) { // P 类型任务
        //     xil_printf("Dependencies3in1P:\n");
        //     for (Task *depTask : task->dependencies3in1P) {
        //       xil_printf("  - %s (Batch: %d, Last Completed Batch: %d)\n",
        //                  depTask->getTaskName().c_str(), depTask->batch,
        //                  depTask->last_completed_batch_index);
        //     }
        //   } else if (task->index == 0) { // S 类型任务
        //     xil_printf("Dependencies3in1S:\n");
        //     for (Task *depTask : task->dependencies3in1S) {
        //       xil_printf("  - %s (Batch: %d, Last Completed Batch: %d)\n",
        //                  depTask->getTaskName().c_str(), depTask->batch,
        //                  depTask->last_completed_batch_index);
        //     }
        //   }
        // } else { // 如果是小任务，打印 dependencies
        //   xil_printf("Task: %s (Small Task)\n", task->getTaskName().c_str());
        //   xil_printf("Dependencies:\n");
        //   for (Task *depTask : task->dependencies) {
        //     xil_printf("  - %s (Batch: %d, Last Completed Batch: %d)\n",
        //                depTask->getTaskName().c_str(), depTask->batch,
        //                depTask->last_completed_batch_index);
        //   }
        // }

        if (task->canStartBatch(task->current_batch_index)) {

          //   xil_printf("Task %s after canStartTask check\n",
          //              task->getAppName()->getAppName().c_str());
          // 记录下一个批次的开始时间
          XTime_GetTime(
              &task->batch_infos[task->current_batch_index].start_time);

          int parameter = task->getParameter();

          slot->set_ip_para((u32)buffer, (u32)buffer, (u32)buffer, (u32)buffer,
                            (u32)buffer, (u32)buffer, (u32)buffer, (u32)buffer,
                            (u32)buffer, (u32)buffer, (u32)buffer, (u32)buffer,
                            parameter, parameter, parameter, parameter, 0);

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

          //   xil_printf("App %s, Task %s, Batch %d start at Slot %s%d\n",
          //              task->getAppName()->getAppName().c_str(),
          //              task->getTaskName().c_str(), task->current_batch_index
          //              + 1, slot_type_str, slot->getSlotNr());

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
                "App %s, Task %s, Batch %d finished at Slot %s%d",
                task->getAppName()->getAppName().c_str(),
                task->getTaskName().c_str(), task->current_batch_index + 1,
                slot_type_str, slot->getSlotNr());
        print_log("INFO", batch_start_message);

        // xil_printf("App %s, Task %s, Batch %d finished at Slot %s%d\n",
        //            task->getAppName()->getAppName().c_str(),
        //            task->getTaskName().c_str(), task->current_batch_index +
        //            1, slot_type_str, slot->getSlotNr());

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

  //   if (isSlotLfree) {
  //     bundle_ready_tasks(taskQueue);
  //   }

  // 遍历任务队列并调度任务
  for (Task *task : taskQueue) {

    if (task->IsDone || task->isScheduled) {
      //   xil_printf("Task %s continued at if (task->IsDone || "
      //              "task->isScheduled)\n",
      //              task->getTaskName().c_str());
      continue;
    }

    Application *app = task->getAppName();
    auto &usage = app_slot_count[app];

    // 检查任务类型
    if (task->type == TASK_S) {
      // 当前是小任务，检查小核的占用情况
      if (usage.small_slots >= app->parallelism_S) {
        // 小核的并行度已达到上限，跳过任务
        // xil_printf("Task: %s continued cz usage full\n",
        //            task->getTaskName().c_str());
        continue;
      }
    } else if (task->type == TASK_L) {
      // 当前是大任务，检查大核的占用情况
      if (usage.large_slots >= app->parallelism_L) {
        // xil_printf("Task: %s continued cz usage full\n",
        //            task->getTaskName().c_str());
        // 大核的并行度已达到上限，跳过任务
        continue;
      }
    }

    if (pr_task->pr_status == 3) {
      XTime_GetTime(&last_pr_finished);
      const char *slot_type_str = (pr_task->slot_type == SLOT_S) ? "S" : "L";
      char time_log_message[128];
      snprintf(time_log_message, sizeof(time_log_message),
               "Slot %s%d finished PR", slot_type_str, pr_task->slot_nr);
      print_log("INFO", time_log_message);

      pr_task->pr_status = 0;
      for (auto &slot : slots) {

        if (slot->getSlotNr() == pr_task->slot_nr &&
            slot->getType() == pr_task->slot_type) {
          slot->partial_reconfigured = true;
        }
      }
    }

    if (pr_task->pr_status != 0) {
      break; // PR 引擎忙，跳过当前任务
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
          break;
        }
      }
    }

    // 检查是否找到合适的 Slot
    if (available_slot == nullptr) {
      // 没有找到合适的 Slot，跳过该任务
      continue;
    }
    // const char *slot_type_str1 =
    //     (available_slot->getType() == SLOT_S) ? "S" : "L";
    // xil_printf("selected task is %s, selected slot is %s%d\n",
    //            task->getTaskName().c_str(), slot_type_str1,
    //            available_slot->getSlotNr());

    // if (!task->canStartBatch(0)) {
    //   task->getAppName()->slot_dep_fail++;
    //   if (task->getAppName()->slot_dep_fail > 3) {
    //     continue;
    //   }
    // } else {
    //   task->getAppName()->slot_dep_fail = 0;
    // }

    // 分配任务到 Slot 并更新状态
    task->isScheduled = true;
    task->getAppName()->isAppStart = true;

    // xil_printf("Task %s is scheduled\n", task->getTaskName().c_str());

    available_slot->curTask = task;
    task->slotNr = available_slot;
    //不需要赋值SlotType，因为本身就是根据TASK_S/L找的Slot
    available_slot->partial_reconfigured = false;
    // 更新 SlotUsage 计数
    if (available_slot->getType() == SLOT_S) {
      usage.small_slots++;
    } else {
      usage.large_slots++;
    }

    // 加载比特流文件
    task->getBitName();
    available_slot->get_bitFile_name();

    // 进行部分重配置
    // 准备共享内存中的 PR 请求
    pr_task->slot_nr = available_slot->getSlotNr();
    pr_task->slot_type = (task->getTaskType() == TASK_S) ? 0 : 1;
    strncpy((char *)pr_task->bitstream_file_name, task->getBitName(),
            sizeof(pr_task->bitstream_file_name) - 1);
    pr_task->bitstream_file_name[sizeof(pr_task->bitstream_file_name) - 1] =
        '\0'; // 确保字符串以空字符结尾
    pr_task->decouple_value = available_slot->get_decouple_value();

    pr_task->pr_status = 1; // 发送 PR 请求
    XTime_GetTime(&last_pr_start);

    u64 pr_time_diff = ((last_pr_start - last_pr_finished) * 1000000) /
                       XPAR_CPU_TIMESTAMP_CLK_FREQ;
    if (pr_time_diff < 200) {
      D_task_blocked++;
    }

    const char *slot_type_str = (pr_task->slot_type == SLOT_S) ? "S" : "L";
    char time_log_message[128];
    snprintf(time_log_message, sizeof(time_log_message),
             "Slot %s%d starting PR", slot_type_str, pr_task->slot_nr);
    print_log("INFO", time_log_message);

    if (available_slot->getType() == SLOT_L) {
      D_PR += 3;
    }
    if (available_slot->getType() == SLOT_S) {
      D_PR++;
    }
  }

  if (pr_task->pr_status == 3) {
    XTime_GetTime(&last_pr_finished);
    const char *slot_type_str = (pr_task->slot_type == SLOT_S) ? "S" : "L";
    char time_log_message[128];
    snprintf(time_log_message, sizeof(time_log_message),
             "Slot %s%d finished PR", slot_type_str, pr_task->slot_nr);
    print_log("INFO", time_log_message);

    pr_task->pr_status = 0;
    for (auto &slot : slots) {

      if (slot->getSlotNr() == pr_task->slot_nr &&
          slot->getType() == pr_task->slot_type) {
        slot->partial_reconfigured = true;
      }
    }
  }
}
