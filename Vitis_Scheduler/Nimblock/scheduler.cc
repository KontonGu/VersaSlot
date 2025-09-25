#include "scheduler.h"
#include "appgenerator.h"
#include "bitloader.h"
#include "xparameters.h"
#include <algorithm> // 用于排序
#include <cstddef>
#include <cstdio>
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
Slot::Slot(int slotNumber, uint32_t ctrlAddr, uint32_t dataAddr,
           uint32_t decoupleVal, u64 time_sd2mem_start, u64 time_sd2mem_end,
           u64 t_mem2fpga_start, u64 t_mem2fpga_end, char *BitFile,
           Task *curTsk, Application *CurApp)
    : slotNr(slotNumber), ctrl_addr(ctrlAddr), data_addr(dataAddr),
      decouple_value(decoupleVal), slotIsIdle(true),
      t_sd2mem_start(time_sd2mem_start), t_sd2mem_end(time_sd2mem_end),
      t_mem2fpga_start(t_mem2fpga_start), t_mem2fpga_end(t_mem2fpga_end),
      BitstreamFile(BitFile), curTask(curTsk), curApp(CurApp) {}

// 更新控制和数据地址
void Slot::update_addr() {
  switch (slotNr) {
  case 1:
    ctrl_addr = 0xA0010000;
    data_addr = 0xA0000000;
    break;
  case 2:
    ctrl_addr = 0xA0030000;
    data_addr = 0xA0020000;
    break;
  case 3:
    ctrl_addr = 0xA0050000;
    data_addr = 0xA0040000;
    break;
  case 4:
    ctrl_addr = 0xA0070000;
    data_addr = 0xA0060000;
    break;
  case 5:
    ctrl_addr = 0xA0090000;
    data_addr = 0xA0080000;
    break;
  case 6:
    ctrl_addr = 0xA00B0000;
    data_addr = 0xA00A0000;
    break;
  case 7:
    ctrl_addr = 0xA00D0000;
    data_addr = 0xA00C0000;
    break;
  case 8:
    ctrl_addr = 0xA00F0000;
    data_addr = 0xA00E0000;
    break;
  default:
    ctrl_addr = 0x0;
    data_addr = 0x0;
    break;
  }
}

// 更新GPIO Decouple值
void Slot::get_decouple_value() {
  if (slotNr >= 1 && slotNr <= 8) {
    decouple_value = 1 << (slotNr - 1);
  } else {
    decouple_value = 0;
  }
}

// 获取比特流名称
void Slot::get_bitFile_name() { BitstreamFile = curTask->getBitName(); }

// Slot 部分重配置函数
int Slot::Slot_PR(XGpio &GpioDecouple, XFpga &XFpgaInstance) {
  int status;
  char *PartialBit = NULL;
  u32 fileSize = 0;
  UINTPTR KeyAddr = (UINTPTR)NULL;
  XTime start_time, end_time;

  // 从SD卡加载比特流到内存
  XTime_GetTime(&start_time);
  t_sd2mem_start = start_time; // 记录 SD 卡到内存的开始时间

  {
    char time_log_message[128];
    snprintf(time_log_message, sizeof(time_log_message), "Slot %d starting PR",
             slotNr);
    print_log("INFO", time_log_message);
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

  // 打印 SD 卡到内存传输的开始和结束时间
  //   {
  //     char time_log_message[128];
  //     snprintf(time_log_message, sizeof(time_log_message),
  //              "Slot %d: t_sd2mem_end", slotNr);
  //     print_log("INFO", time_log_message);
  //   }

  // 加载比特流到FPGA
  XTime_GetTime(&start_time);
  t_mem2fpga_start = start_time; // 记录内存到FPGA的开始时间

  //   {
  //     char time_log_message[128];
  //     snprintf(time_log_message, sizeof(time_log_message),
  //              "Slot %d: t_mem2fpga_start", slotNr);
  //     print_log("INFO", time_log_message);
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
    char time_log_message[128];
    snprintf(time_log_message, sizeof(time_log_message), "Slot %d finished PR",
             slotNr);
    print_log("INFO", time_log_message);
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

// 判断是否空闲
bool Slot::isSlotIdle() const { return slotIsIdle; }

// Task 构造函数
Task::Task(std::string taskName, Application *taskAppname, u64 taskLatency,
           int taskParameter, int taskBatch, int taskIndex)
    : name(taskName), appname(taskAppname), latency(taskLatency),
      parameter(taskParameter), batch(taskBatch), current_batch_index(0),
      last_completed_batch_index(-1), IsDone(false), isScheduled(false),
      index(taskIndex), bitstream(NULL), slotNr(NULL), arrival_time(0),
      start_time(0), end_time(0), isBlocked(false) {
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
char *Task::getBitName() const {
  uint slotID = slotNr->getSlotNr();
  std::string appName = appname->getAppName();

  // 创建 std::string 的文件名
  std::string filename =
      appName + "_" + name + "_slot_" + std::to_string(slotID) + "_partial.bit";

  // 将 std::string 转换为 char*
  char *result = new char[filename.size() + 1]; // 分配动态内存
  strcpy(result, filename.c_str());             // 复制内容到 result

  return result;
}

bool Task::canStartBatch(int batch_index) {
  for (Task *depTask : dependencies) {
    if (depTask->last_completed_batch_index < batch_index) {
      return false;
    }
  }
  return true;
}

// Application 构造函数
Application::Application(std::string appName, int appBatch,
                         std::vector<Task *> appTaskList)
    : name(appName), batch(appBatch), tasksList(appTaskList), priority(0),
      priority_level(0), arrival_time(0), start_time(0), end_time(0),
      parallelism(0), // 并行任务数初始化为 0
      threshold(0),   // 阈值初始化为 0
      tasks_completed(0), tasks_extracted(false),
      occupancy(0) // 已完成任务数初始化为 0
{}

// 获取应用名
std::string Application::getAppName() const { return name; }

// 添加任务
void Application::addTask(Task *task) { tasksList.push_back(task); }

int Application::getBatch() const { return batch; }

void Application::defineDependencies() {
  // 假设每个任务依赖于前一个任务
  for (size_t i = 1; i < tasksList.size(); ++i) {
    Task *currentTask = tasksList[i];
    Task *prevTask = tasksList[i - 1];
    currentTask->dependencies.push_back(prevTask);
  }
}

// 初始化 Slots
std::vector<Slot *> initializeSlots() {

  char *bitFile = nullptr; // 假设暂时没有比特流文件，后续可以更新
  Task *curTsk = nullptr;        // 暂时没有当前任务
  Application *curApp = nullptr; // 暂时没有当前应用

  // 使用 std::vector 来存储 Slot* 指针
  std::vector<Slot *> slots;

  // 初始化 slots，但不提供 ctrlAddr 和 dataAddr，使用默认值 0
  for (int i = 0; i < 8; i++) {
    slots.push_back(
        new Slot(i + 1, 0, 0, 0, 0, 0, 0, 0, bitFile, curTsk, curApp));
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
  return 1.0 * off_ms * 1000.0; // 转换为微秒
}

// std::queue 用于保存应用程序生成顺序
std::queue<AppGenStruct> all_apps_queue;

// 在此处预设跑分模型
void initialize_all_apps() {
  // 生成的应用程序
  // 前20个应用设为OF
  AppGenStruct app1 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app2 = {appgenerator_LENet, 20, 2, 9.0, 9};
  AppGenStruct app3 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app4 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app5 = {appgenerator_IC, 20, 4, 9.0, 9};
  AppGenStruct app6 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app7 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app8 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app9 = {appgenerator_3DR, 20, 2, 9.0, 9};
  AppGenStruct app10 = {appgenerator_IC, 20, 4, 9.0, 9};
  AppGenStruct app11 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app12 = {appgenerator_OF, 10, 3, 3.0, 3};

  // 随后的20个应用中，OF占比60%

  // 再往后的20个应用中OF比例20%
  AppGenStruct app13 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app14 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app15 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app16 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app17 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app18 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app19 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app20 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app21 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app22 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app23 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app24 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app25 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app26 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app27 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app28 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app29 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app30 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app31 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app32 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app33 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app34 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app35 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app36 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app37 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app38 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app39 = {appgenerator_LENet, 20, 2, 9.0, 9};
  AppGenStruct app40 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app41 = {appgenerator_OF, 10, 3, 3.0, 3};

  AppGenStruct app42 = {appgenerator_IC, 10, 4, 9.0, 9};

  // 没有OF的20个应用
  AppGenStruct app43 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app44 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app45 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app46 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app47 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app48 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app49 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app50 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app51 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app52 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app53 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app54 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app55 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app56 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app57 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app58 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app59 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app60 = {appgenerator_3DR, 10, 2, 9.0, 9};
  AppGenStruct app61 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app62 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app63 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app64 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app65 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app66 = {appgenerator_IC, 10, 4, 9.0, 9};
  AppGenStruct app67 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app68 = {appgenerator_LENet, 20, 2, 9.0, 9};
  AppGenStruct app69 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app70 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app71 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app72 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app73 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app74 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app75 = {appgenerator_LENet, 10, 2, 9.0, 9};
  AppGenStruct app76 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app77 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app78 = {appgenerator_3DR, 20, 2, 9.0, 9};
  AppGenStruct app79 = {appgenerator_OF, 10, 3, 3.0, 3};
  AppGenStruct app80 = {appgenerator_OF, 10, 3, 3.0, 3};

  // 最后20个应用，不包含OF，且batch值设为5
  AppGenStruct app81 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app82 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app83 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app84 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app85 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app86 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app87 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app88 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app89 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app90 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app91 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app92 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app93 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app94 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app95 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app96 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app97 = {appgenerator_LENet, 5, 2, 9.0, 9};
  AppGenStruct app98 = {appgenerator_3DR, 5, 2, 9.0, 9};
  AppGenStruct app99 = {appgenerator_IC, 5, 3, 9.0, 9};
  AppGenStruct app100 = {appgenerator_LENet, 5, 2, 9.0, 9};

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
  all_apps_queue.push(app46);
  all_apps_queue.push(app47);
  all_apps_queue.push(app48);
  all_apps_queue.push(app49);
  all_apps_queue.push(app50);
  all_apps_queue.push(app51);
  all_apps_queue.push(app52);
  all_apps_queue.push(app53);
  all_apps_queue.push(app54);
  all_apps_queue.push(app55);
  all_apps_queue.push(app56);
  all_apps_queue.push(app57);
  all_apps_queue.push(app58);
  all_apps_queue.push(app59);
  all_apps_queue.push(app60);
  all_apps_queue.push(app61);
  all_apps_queue.push(app62);
  all_apps_queue.push(app63);
  all_apps_queue.push(app64);
  all_apps_queue.push(app65);
  all_apps_queue.push(app66);
  all_apps_queue.push(app67);
  all_apps_queue.push(app68);
  all_apps_queue.push(app69);
  all_apps_queue.push(app70);
  all_apps_queue.push(app71);
  all_apps_queue.push(app72);
  all_apps_queue.push(app73);
  all_apps_queue.push(app74);
  all_apps_queue.push(app75);
  all_apps_queue.push(app76);
  all_apps_queue.push(app77);
  all_apps_queue.push(app78);
  all_apps_queue.push(app79);
  all_apps_queue.push(app80);
  all_apps_queue.push(app81);
  all_apps_queue.push(app82);
  all_apps_queue.push(app83);
  all_apps_queue.push(app84);
  all_apps_queue.push(app85);
  all_apps_queue.push(app86);
  all_apps_queue.push(app87);
  all_apps_queue.push(app88);
  all_apps_queue.push(app89);
  all_apps_queue.push(app90);
  all_apps_queue.push(app91);
  all_apps_queue.push(app92);
  all_apps_queue.push(app93);
  all_apps_queue.push(app94);
  all_apps_queue.push(app95);
  all_apps_queue.push(app96);
  all_apps_queue.push(app97);
  all_apps_queue.push(app98);
  all_apps_queue.push(app99);
  all_apps_queue.push(app100);
}

// 调度核心算法

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
  Application *new_app =
      current_app.generator(current_app.batch, current_app.parallelism,
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
  //   for (auto it = candidate_apps.begin(); it != candidate_apps.end();) {
  //     Application *cur_app = *it;
  //     if (cur_app->priority < threshold) {
  //       it = candidate_apps.erase(it);
  //       pending_apps.push_back(cur_app);
  //       rethresh = true;
  //     } else {
  //       ++it;
  //     }
  //   }
  return rethresh;
}

void update_ready_tasks(std::list<Application *> &candidate_apps,
                        std::list<Task *> &ready_tasks) {
  // 遍历 candidate_apps 列表中的每个应用
  for (auto it = candidate_apps.begin(); it != candidate_apps.end();) {
    Application *app = *it;

    // 检查应用是否已经被标记为 tasks_extracted
    if (app->tasks_extracted) {
      // 如果已经被标记为 tasks_extracted，则跳过
      ++it;
      continue;
    }

    // 将应用中的任务移动到 ready_tasks
    for (Task *task : app->tasksList) {
      ready_tasks.push_back(task);
    }

    // 设置 tasks_extracted 为 true
    app->tasks_extracted = true;

    ++it;
  }
}

std::list<Application *> retire_apps(std::list<Application *> &candidate_apps) {
  std::list<Application *> retired_apps; // 用于存储完成的应用

  // 遍历 candidate_apps 列表中的每个应用
  for (auto app_it = candidate_apps.begin(); app_it != candidate_apps.end();) {
    Application *app = *app_it;

    // 获取已完成的任务数量
    int completed_tasks = app->tasks_completed;
    int total_tasks = app->tasksList.size();

    // 检查是否所有任务都已完成
    if (completed_tasks == total_tasks) {
      // 记录日志
      char retire_app_message[128];
      sprintf(retire_app_message,
              "App ID: %d, Retiring  App: %s, Completed Tasks: %d/%d\n",
              app->unique_ID, app->getAppName().c_str(), completed_tasks,
              total_tasks);
      print_log("INFO", retire_app_message);

      // 将应用添加到 retired_apps 列表中
      retired_apps.push_back(app);
      // 从 candidate_apps 中移除该应用
      app_it = candidate_apps.erase(app_it);
    } else {
      // 如果应用程序还有未完成的任务，继续下一个应用
      ++app_it;
    }
  }

  // 返回已完成的应用列表
  return retired_apps;
}

#define THRESHOLD_LEVEL 1

void allocate_resources(std::list<Application *> &candidate_apps) {
  int num_slots_active = 8; // 假设有8个Slot
  int assigned_slots = 0;

  // 新建一个列表来存储每个应用程序的原始 parallelism 值
  std::list<int> original_parallelism;
  for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it) {
    original_parallelism.push_back((*it)->parallelism); // 保存原始值
  }

  // 第一遍：分配每个应用程序最低的 Slot 数量
  for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it) {
    Application *my_app = *it;
    int tasks_left = my_app->tasksList.size() - my_app->tasks_completed;
    // 分配最小数量的 Slot，确保任务可以运行
    int slots = std::min(THRESHOLD_LEVEL, tasks_left);
    my_app->parallelism = slots; // 更新应用的并行度
    assigned_slots += slots;
  }

  // 第二遍：如果还有空闲的 Slot，更新到目标并行度
  if (assigned_slots < num_slots_active) {
    // 从原始 parallelism 值中读取目标值
    auto orig_it = original_parallelism.begin();
    for (auto it = candidate_apps.begin(); it != candidate_apps.end();
         ++it, ++orig_it) {
      Application *my_app = *it;
      int tasks_left = my_app->tasksList.size() - my_app->tasks_completed;
      int my_goal = *orig_it; // 使用原始 parallelism 值
      int goal_slots = std::min(my_goal, tasks_left);
      int slot_diff = goal_slots - my_app->parallelism; // 计算差值
      my_app->parallelism += slot_diff;                 // 更新并行度
      assigned_slots += slot_diff;

      if (assigned_slots >= num_slots_active)
        break; // 如果已分配的 Slot 达到最大数，退出循环
    }
  }

  // 第三遍：如果还有剩余的 Slot，根据任务数分配更多的 Slot
  if (assigned_slots < num_slots_active) {
    for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it) {
      Application *my_app = *it;
      int tasks_left = my_app->tasksList.size() - my_app->tasks_completed;
      int slot_diff = tasks_left - my_app->parallelism; // 计算剩余任务的差值
      if (slot_diff > 0) {
        // 如果有额外的 Slot 可用，继续分配
        my_app->parallelism += slot_diff;
        assigned_slots += slot_diff;

        if (assigned_slots >= num_slots_active)
          break; // Slot 用完后退出
      }
    }
  }

  // 遍历 candidate_apps 队列并输出每个应用的 parallelism
  //   for (auto it = candidate_apps.begin(); it != candidate_apps.end(); ++it)
  //   {
  //     Application *my_app = *it;
  //     char parallelism_log_message[128];
  //     snprintf(parallelism_log_message, sizeof(parallelism_log_message),
  //              "App: %s, Parallelism: %d", my_app->getAppName().c_str(),
  //              my_app->parallelism);
  //     print_log("INFO", parallelism_log_message);
  //   }
}

void update_slot_state(std::vector<Slot *> &slots) {
  for (Slot *slot : slots) {
    slot->checkSlotState(); // 更新 Slot 的空闲状态
  }
}

void preempt_slot_apps(std::vector<Slot *> machines,
                       std::list<Task *> &ready_tasks,
                       std::list<Application *> &candidate_apps) {

  for (Application *App : candidate_apps) {
    std::vector<Slot *> app_slots; // 临时存放属于 App 的 Slot

    // 第一次遍历：找到所有属于 App 的任务
    for (Slot *cur_slot : machines) {
      if (cur_slot->curTask && cur_slot->curTask->getAppName() == App) {
        app_slots.push_back(cur_slot);
      }
    }

    int slot_used = app_slots.size();
    // xil_printf("App %s, slot used: %d\n", App->getAppName().c_str(),
    // slot_used);

    // 按任务的 current_batch_index 从小到大排序，以保证顺序
    std::sort(app_slots.begin(), app_slots.end(), [](Slot *a, Slot *b) {
      return a->curTask->index < b->curTask->index;
    });

    // 检查 slot_used 是否超出 App->optimal，并抢占多余的任务
    if (slot_used > App->optimal) {
      for (int i = App->optimal; i < static_cast<int>(app_slots.size()); ++i) {
        Slot *cur_slot = app_slots[i];
        // cur_slot->curTask->current_batch_index -= 1;
        cur_slot->curTask->isScheduled = false;
        ready_tasks.remove(cur_slot->curTask);
        ready_tasks.push_back(cur_slot->curTask);

        char batch_end_message[128];
        sprintf(batch_end_message,
                "App %s, Task %s, Batch %d finished at Slot %d",
                cur_slot->curTask->getAppName()->getAppName().c_str(),
                cur_slot->curTask->getTaskName().c_str(),
                cur_slot->curTask->current_batch_index, cur_slot->getSlotNr());
        print_log("INFO", batch_end_message);

        // 打印抢占任务日志
        char batch_preempt_message[128];
        sprintf(batch_preempt_message, "Preempt task %s in Slot %d",
                cur_slot->curTask->getTaskName().c_str(),
                cur_slot->getSlotNr());
        print_log("INFO", batch_preempt_message);

        // 清空该 slot 上的任务和应用
        cur_slot->curTask = nullptr;
        cur_slot->curApp = nullptr;
        slot_used--; // 更新 slot_used 数量
      }
    }
  }
}

void execute_tasks(std::list<Task *> &taskQueue, std::vector<Slot *> &slots,
                   XGpio &GpioDecouple, XFpga &XFpgaInstance, uintptr_t buffer,
                   int &D_task_blocked, int &D_PR,
                   std::list<Application *> &candidate_apps) {

  // 初始化应用程序到 Slot 占用数量的映射
  std::map<Application *, int> app_slot_count;
  int used_slot = 0;
  // 统计每个应用程序当前占用了多少个 Slot
  for (auto &slot : slots) {
    if (slot->curTask != nullptr) {
      app_slot_count[slot->curTask->getAppName()]++;
      used_slot++;
    }
  }

  for (Task *task : taskQueue) {
    if (used_slot == 8) {

      //   xil_printf("Ready Task List:\n");
      //   for (Task *task : taskQueue) {
      //     xil_printf("%s ", task->getTaskName().c_str());
      //   }
      //   xil_printf("\n");

      preempt_slot_apps(slots, taskQueue, candidate_apps);
    }
    // 如果任务已完成或已被调度，跳过
    if (task->IsDone || task->isScheduled) {
      continue;
    }

    Application *app = task->getAppName();

    // 检查应用程序的并行度限制
    int app_parallelism = app->parallelism;
    int occupied_slots = app_slot_count[app];

    if (occupied_slots >= app_parallelism) {
      continue; // 应用程序已达到并行度限制，跳过
    }

    // 检查任务的当前批次是否可以开始
    if (!task->canStartBatch(task->current_batch_index)) {
      continue; // 依赖关系未满足，跳过
    }

    // 查找一个空闲的 Slot
    Slot *available_slot = nullptr;

    if (available_slot == nullptr) {
      for (auto &slot : slots) {
        if (slot->isSlotIdle() && slot->curTask == nullptr) {
          available_slot = slot;
          break;
        }
      }
    }

    if (available_slot == nullptr) {
      // 没有可用的 Slot，跳过任务
      continue;
    }

    // 分配任务到 Slot，但未必直接启动
    task->isScheduled = true;
    available_slot->curTask = task;
    app_slot_count[app]++;
    task->slotNr = available_slot;
    task->getAppName()->occupancy++;

    // 获取比特流文件名
    task->getBitName();
    available_slot->get_bitFile_name();

    // 进行部分重配置
    int status = available_slot->Slot_PR(GpioDecouple, XFpgaInstance);
    D_PR++; // 增加 PR 计数
    if (status != XST_SUCCESS) {
      // 处理错误
      char error_message[128];
      sprintf(error_message, "Failed to load bitstream to memory for task:%s",
              task->getTaskName().c_str());
      print_log("ERROR", error_message);

      // 标记任务为已完成以避免阻塞
      task->IsDone = true;
      available_slot->curTask = nullptr;
      app_slot_count[app]--;
      continue;
    }

    int parameter = task->getParameter();

    // 设置 IP 参数（需要根据实际参数填写）
    available_slot->set_ip_para((u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                (u32)buffer, (u32)buffer, (u32)buffer,
                                parameter, parameter, parameter, parameter, 0);
    // 记录任务开始时间
    XTime_GetTime(&task->start_time);

    // 记录批次开始时间
    int batch_index = task->current_batch_index;
    XTime_GetTime(&task->batch_infos[batch_index].start_time);

    // 启动批次
    available_slot->Slot_Start();

    // 记录批次开始日志
    char batch_start_message[128];
    sprintf(batch_start_message, "App %s, Task %s, Batch %d started at Slot %d",
            task->getAppName()->getAppName().c_str(),
            task->getTaskName().c_str(), batch_index + 1,
            available_slot->getSlotNr());
    print_log("INFO", batch_start_message);

    // xil_printf("App %s, Task %s, Batch %d started at Slot %d\n",
    //            task->getAppName()->getAppName().c_str(),
    //            task->getTaskName().c_str(), batch_index + 1,
    //            available_slot->getSlotNr());

    // **在这里递增 current_batch_index**
    task->current_batch_index++;
  }

  // **在这里开始 Slot 循环**
  for (auto &slot : slots) {

    update_slot_state(slots);

    Task *task = slot->curTask;
    // 仅在slot为空，但slot的task仍然存在的情况下处理
    if (task != nullptr && slot->isSlotIdle()) {
      // 只有当 current_batch_index 大于 0 时，才处理批次完成
      if (task->current_batch_index > 0) {
        int batch_index = task->current_batch_index - 1;

        //这个时候slot为空，说明batch完成了。如果batch index为0
        //说明任务还没开始
        if (batch_index >= 0 && !task->batch_infos[batch_index].isCompleted) {
          // 记录批次结束时间
          XTime_GetTime(&task->batch_infos[batch_index].end_time);

          // 标记当前批次已完成
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

          // 记录批次完成日志
          char batch_end_message[128];
          sprintf(batch_end_message,
                  "App %s, Task %s, Batch %d finished at Slot %d",
                  task->getAppName()->getAppName().c_str(),
                  task->getTaskName().c_str(), batch_index + 1,
                  slot->getSlotNr());
          print_log("INFO", batch_end_message);

          //   xil_printf("App %s, Task %s, Batch %d finished at Slot %d\n",
          //              task->getAppName()->getAppName().c_str(),
          //              task->getTaskName().c_str(), batch_index + 1,
          //              slot->getSlotNr());

          // 更新 last_completed_batch_index
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
          char batch_start_message[128];
          sprintf(batch_start_message,
                  "App %s, Task %s, Batch %d started at Slot %d",
                  task->getAppName()->getAppName().c_str(),
                  task->getTaskName().c_str(), task->current_batch_index + 1,
                  slot->getSlotNr());
          print_log("INFO", batch_start_message);

          //   xil_printf("App %s, Task %s, Batch %d started at Slot %d\n",
          //              task->getAppName()->getAppName().c_str(),
          //              task->getTaskName().c_str(), task->current_batch_index
          //              + 1, slot->getSlotNr());

          // **在这里递增 current_batch_index**
          task->current_batch_index++;
        }
      } else {
        // 任务所有批次已完成
        task->IsDone = true;
        task->getAppName()->tasks_completed++;
        // 记录任务完成日志
        char end_time_message[128];
        sprintf(end_time_message,
                "App %s, Task %s, all batches completed at Slot %d",
                task->getAppName()->getAppName().c_str(),
                task->getTaskName().c_str(), slot->getSlotNr());
        print_log("INFO", end_time_message);

        // // 从任务队列中移除任务
        // taskQueue.remove(task);

        // 清除 Slot 的当前任务
        slot->curTask = nullptr;
        task->getAppName()->occupancy--;

        // 更新应用程序的 Slot 占用计数
        app_slot_count[task->getAppName()]--;
      }
    }
  }

  // 遍历任务队列
}
