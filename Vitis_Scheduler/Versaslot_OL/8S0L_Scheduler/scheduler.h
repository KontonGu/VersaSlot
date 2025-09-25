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
  char bitstream_file_name[128]; // 128 Bytes
  int slot_nr;                   // 4 Bytes
  int slot_type;                 // 4 Bytes
  uint32_t decouple_value;       // 4 Bytes
  volatile uint8_t pr_status;    // 1 Byte
  volatile uint8_t init_success; // 1 Byte
  uint8_t padding[2];            // 2 Bytes to fill，ensuare total size to be 144 Bytes
} __attribute__((aligned(4))) PR_Task;


extern XTime program_start_time;
extern std::vector<LogEntry *> log_entries;

class Application;
class Task;

class Slot {
private:
  int slotNr;              // Slot 1-8
  uint32_t ctrl_addr;      // control register address
  uint32_t data_addr;      // data register address
  uint32_t decouple_value; // decouple value
  bool slotIsIdle;         // check if the slot is idle
  u64 t_sd2mem_start;      // the start time of transfering data from SD to memory
  u64 t_sd2mem_end;        // the end time of transfering data from SD to memory
  u64 t_mem2fpga_start;    // the start time of transfering data from memory to FPGA
  u64 t_mem2fpga_end;      // the end time of transfering data from memory to FPGA
  char *BitstreamFile;     // current bitstream

public:
  Task *curTask;       // currently running task
  Application *curApp; // currently running application
  bool partial_reconfigured;

  Slot(int slotNumber, uint32_t ctrlAddr, uint32_t dataAddr,
       uint32_t decoupleVal, u64 t_sd2mem_Start, u64 t_sd2mem_End,
       u64 t_mem2fpga_Start, u64 t_mem2fpga_End, char *BitFile, Task *curTsk,
       Application *CurApp);

  // according to slotNr, update ctrl_addr and data_addr
  void update_addr();

  // update GPIO Decouple
  uint32_t get_decouple_value();

  void get_bitFile_name();

  // partial reconfiguration
  int Slot_PR(XGpio &GpioDecouple, XFpga &XFpgaInstance);

  void set_ip_para(u32 buffer1, u32 buffer2, u32 buffer3, u32 buffer4,
                   u32 buffer5, u32 buffer6, u32 buffer7, u32 buffer8,
                   u32 buffer9, u32 buffer10, u32 buffer11, u32 buffer12,
                   int intBuffer1, int intBuffer2, int intBuffer3,
                   int intBuffer4, int intBuffer5);

  // start slot
  void Slot_Start();

  // check slot status
  void checkSlotState();

  // get slot number
  int getSlotNr() const;

  // check if the slot is idle
  bool isSlotIdle() const;
};

class Task {
private:
  std::string name;
  Application *appname;
  u64 latency;
  int parameter; 

public:
  int batch;                      // total batch size
  int current_batch_index;        // the batch index that is currently running
  int last_completed_batch_index; // the batch index last finished

  struct BatchInfo {
    XTime start_time;
    XTime end_time;
    bool isCompleted; // if the batch is completed
  };

  std::vector<BatchInfo> batch_infos; // information of each batch
  std::vector<Task *> dependencies;   // the front tasks that current task relies on

  bool IsDone;     
  bool isScheduled; 
  int index;        
  char *bitstream;
  Slot *slotNr;
  XTime arrival_time, start_time, end_time;
  bool isBlocked;

  
  Task(std::string taskName, Application *taskAppname, u64 taskLatency,
       int taskParameter, int taskBatch, int taskIndex);

  const std::string &getTaskName() const;
  Application *getAppName() const;
  char *getBitName() const;
  u64 getLatency() const;
  int getParameter() const;

  // check if the batch can start
  bool canStartBatch(int batch_index);
};

class Application {
private:
  std::string name; // 
  int batch;        // total batch size

public:
  std::vector<Task *> tasksList; // task list
  double priority;               // priority
  int priority_level;
  XTime arrival_time, start_time, end_time;

  int parallelism;     // number of tasks in parallel, default to be 0
  int threshold;       // default to be 0
  int tasks_completed; // number of tasks completed, default to be 0
  bool tasks_extracted;
  int unique_ID;
  int slot_dep_fail; // the number of unsatisfied tasks when performing slot pre-loading
  int optimal;

  Application(std::string appName, int appBatch,
              std::vector<Task *> appTaskList);

  const std::string &getAppName() const;
  void addTask(Task *task);
  int getBatch() const;
  void defineDependencies();
};

std::vector<Slot *> initializeSlots();
uintptr_t get_physical_address(void *virtual_address);
double next_event_delay(int min, int max);
void print_log(const char *level, const char *message);
void execute_tasks(std::list<Task *> &taskQueue, std::vector<Slot *> &slots,
                   uintptr_t buffer, int &D_task_blocked, int &D_PR,
                   std::list<Application *> &candidate_apps,
                   XTime last_pr_start, XTime last_pr_finished);
void update_slot_state(std::vector<Slot *> &slots);
void allocate_resources(std::list<Application *> &candidate_apps);
void update_priorities_task(std::list<Task *> &taskQueue);

typedef Application *(*AppGenerator)(int batch, int parallelism,
                                     double priority, int priority_level);

struct AppGenStruct {
  AppGenerator generator; 
  int batch;
  int parallelism;
  double priority;
  int priority_level;
};

Application *add_new_event(std::list<Application *> &pending_apps, int app_cnt);
double update_priorities_app(std::list<Application *> &pending_apps,
                             std::list<Application *> &candidate_apps);
bool update_candidate_apps(std::list<Application *> &pending_apps,
                           std::list<Application *> &candidate_apps,
                           double threshold);
void update_ready_tasks(std::list<Application *> &candidate_apps,
                        std::list<Task *> &ready_tasks);
void initialize_all_apps();
std::list<Application *> retire_apps(std::list<Application *> &candidate_apps);

#endif
