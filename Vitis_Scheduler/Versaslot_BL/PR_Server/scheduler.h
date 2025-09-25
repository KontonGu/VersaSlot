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

#ifndef VERSASLOT_H
#define VERSASLOT_H

#include "bitloader.h"
#include "platform.h"
#include "xil_io.h"
#include "xilfpga.h"
#include "xiltimer.h"
#include "xparameters.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <xgpio.h>
#include <xil_types.h>

struct LogEntry {
  u64 timestamp;
  std::string level;
  std::string message;
};

typedef struct {
  char bitstream_file_name[128];
  int slot_nr;
  int slot_type; // 0 = SLOT_S, 1 = SLOT_L
  uint32_t decouple_value;
  volatile uint8_t
      pr_status; // 0: 空闲, 1: 请求PR, 2: PR执行中，3: PR完成， 4: PR异常
  volatile uint8_t init_success; // 0: 初始化失败, 1: 初始化成功
  uint8_t padding[2];            // 填充2字节，确保总大小为144字节
} __attribute__((aligned(4))) PR_Task;

// 声明全局变量和日志容器
extern XTime program_start_time;
extern std::vector<LogEntry> log_entries;

enum SlotType { SLOT_S, SLOT_L };
enum TaskType { TASK_S, TASK_L };

struct SlotUsage {
  int small_slots = 0; // 小核占用数
  int large_slots = 0; // 大核占用数
};

class Application;
class Task;

class Slot {
private:
  SlotType type;           // Slot类型：SLOT_S 或 SLOT_L
  int slotNr;              // Slot 1-4
  uint32_t ctrl_addr;      // 控制寄存器地址
  uint32_t data_addr;      // 数据寄存器地址
  uint32_t decouple_value; // 解耦值
  bool slotIsIdle;         // 槽当前是否空闲
  u64 t_sd2mem_start;      // SD 到内存传输开始时间
  u64 t_sd2mem_end;        // SD 到内存传输结束时间
  u64 t_mem2fpga_start;    // 内存到 FPGA 传输开始时间
  u64 t_mem2fpga_end;      // 内存到 FPGA 传输结束时间
  char *BitstreamFile;     // 当前比特流

public:
  Task *curTask;       // 当前运行的任务
  Application *curApp; // 当前运行的应用

  Slot(SlotType slotType, int slotNumber, uint32_t ctrlAddr, uint32_t dataAddr,
       uint32_t decoupleVal, u64 t_sd2mem_Start, u64 t_sd2mem_End,
       u64 t_mem2fpga_Start, u64 t_mem2fpga_End, char *BitFile, Task *curTsk,
       Application *CurApp);

  // 根据 slotNr 更新 ctrl_addr 和 data_addr
  void update_addr();

  // 更新 GPIO Decouple
  void get_decouple_value();

  // 获取比特流名称
  void get_bitFile_name();

  // 分区重配置
  int Slot_PR(XGpio &GpioDecouple, XFpga &XFpgaInstance);

  // 设置 IP 参数
  void set_ip_para(u32 buffer1, u32 buffer2, u32 buffer3, u32 buffer4,
                   u32 buffer5, u32 buffer6, u32 buffer7, u32 buffer8,
                   u32 buffer9, u32 buffer10, u32 buffer11, u32 buffer12,
                   int intBuffer1, int intBuffer2, int intBuffer3,
                   int intBuffer4, int intBuffer5);

  // 启动槽
  void Slot_Start();

  // 检查槽状态
  void checkSlotState();

  // 获取槽号
  int getSlotNr() const;

  SlotType getType() const;

  // 判断是否空闲
  bool isSlotIdle() const;
};

class Task {
private:
  std::string name;
  Application *appname;
  u64 latency;
  int parameter; // for task time controll

public:
  TaskType type;
  int batch;               // 总的batch数
  int current_batch_index; //用于跟踪当前正在执行的批次索引
  int last_completed_batch_index; //用于跟踪最后一个已完成的批次索引

  // 添加批次信息结构和向量
  struct BatchInfo {
    XTime start_time;
    XTime end_time;
    bool isCompleted; // 批次是否完成
  };

  std::vector<BatchInfo> batch_infos; // 批次信息向量
  std::vector<Task *> dependencies;   // 依赖的任务列表

  bool IsDone; //所有批次都完成
  bool isScheduled;
  int index; //任务所在的App中的编号
  char *bitstream;
  Slot *slotNr;
  XTime arrival_time, start_time, end_time;
  bool isBlocked = false;
  double factor; //用于判断大核串行还是并行

  // 构造函数声明
  Task(std::string taskName, Application *taskAppname, u64 taskLatency,
       int taskParameter, int taskBatch, TaskType taskType, int taskIndex,
       double taskFactor);

  // 添加方法声明
  std::string getTaskName() const;
  Application *getAppName() const;
  char *getBitName();
  u64 getLatency() const;
  int getParameter() const;
  TaskType getTaskType() const;

  // 添加检查批次是否可以开始的方法
  bool canStartBatch(int batch_index);
};

class Application {
private:
  std::string name; // 应用名称
  int batch;

public:
  std::vector<Task *> tasksList;      // 任务列表
  std::vector<Task *> tasksList_3in1; // 查询列表
  double priority;                    // 优先级
  int priority_level;
  XTime arrival_time, start_time, end_time;

  int parallelism_S; // 并行任务数，默认初始化为0
  int parallelism_L;
  int threshold;       // 阈值，默认初始化为0
  int tasks_completed; // 已完成任务数，默认初始化为0
  bool task_extracted;
  int bundled_3in1tasks;

  // 构造函数
  Application(std::string appName, int appBatch,
              std::vector<Task *> appTaskList,
              std::vector<Task *> appTaskList3in1);

  std::string getAppName() const;
  void addTask(Task *task);
  void addTask_3in1(Task *task);
  int getBatch() const;
  void defineDependencies();
  void defineDependencies3in1();
};

std::vector<Slot *> initializeSlots();
uintptr_t get_physical_address(void *virtual_address);
double next_event_delay(int min, int max);
void print_log(const char *level, const char *message);
void update_priorities_task(std::list<Task *> &taskQueue);
void execute_tasks(std::list<Task *> &taskQueue, std::vector<Slot *> &slots,
                   XGpio &GpioDecouple, XFpga &XFpgaInstance, uintptr_t buffer,
                   int &D_task_blocked, int &D_PR);
void update_slot_state(std::vector<Slot> &slots);
void update_slot_task(std::vector<Slot> &slots, std::list<Task *> &taskQueue);
void allocate_resources(std::list<Application *> &candidate_apps);

// 应用程序生成器的类型，使用函数指针，回调函数，因为具体什么函数不确定。
typedef Application *(*AppGenerator)(int, int, int, double, int);

// 结构体，包含任意App生成函数
struct AppGenStruct {
  AppGenerator generator; // 应用程序生成函数
  int batch;
  int parallelism_S;
  int parallelism_L;
  double priority;
  int priority_level;
};

Application *add_new_event(std::list<Application *> &pending_apps);
double update_priorities_app(std::list<Application *> &pending_apps,
                             std::list<Application *> &candidate_apps);
bool update_candidate_apps(std::list<Application *> &pending_apps,
                           std::list<Application *> &candidate_apps,
                           double threshold);
void update_ready_tasks(std::list<Application *> &candidate_apps,
                        std::list<Task *> &ready_tasks);
void initialize_all_apps();
std::list<Application *> retire_apps(std::list<Application *> &candidate_apps,
                                     std::list<Task *> &ready_tasks);

#endif
