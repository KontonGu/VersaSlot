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
*/


#include "appgenerator.h"
#include "bitloader.h"
#include "platform.h"
#include "scheduler.h"
#include "xgpio.h"
#include "xilfpga.h"
#include "xparameters.h"
#include <iostream>
#include <list>
#include <malloc.h>
#include <stdio.h>
#include <xil_printf.h>

#define SCHED_INTERVAL 400000
#define MAX_APPS 100
#define SHARED_MEMORY_BASE 0xfffc0000
volatile PR_Task *pr_task = (volatile PR_Task *)SHARED_MEMORY_BASE;

XFpga XFpgaInstance;      //Initialize global PL object
XGpio GpioDecouple;       // Initialize global object, Decouple
XGpio GpioDecoupleStatus; // Initialize global object, Decouple Status 
int status;               // Global Status Control
XTime start_time, current_time, program_start_time;
volatile int spin = 0;
bool updated_already = false;

int main() {
  /////////////////////////////////////////////
  ///////////////// System Initialization /////////////////
  ////////////////////////////////////////////

  init_platform(); // Initialize platform

  // Get the program start time for timestamping
  XTime_GetTime(&program_start_time);

  Xil_DCacheDisable(); // Disable cache

  print_log("INFO", "System Initializing ...");

  status = init_filesystem(); // Initialize SD card file system
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to initialize SD FileSystem");
    cleanup_platform();
    return XST_FAILURE;
  }
  print_log("INFO", "SD FileSystem successfully initialized");

  status = XFpga_Initialize(&XFpgaInstance); // Initialize FPGA driver
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to initialize PL Part");
    cleanup_platform();
    return XST_FAILURE;
  }
  print_log("INFO", "FPGA successfully initialized");

  status = XGpio_Initialize(
      &GpioDecouple,
      XPAR_DECOUPLE_BASEADDR); // Initialize GPIO controller Decouple
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to initialize GPIO Decouple");
    cleanup_platform();
    return XST_FAILURE;
  }
  while (pr_task->init_success != 1) {
    print_log("INFO", "Waiting for PR Server initialization...");
    for (volatile int i = 0; i < 10000000; i++)
      ; // 延迟，避免过多输出
  }

  print_log("INFO", "GPIO Decouple successfully initialized");
  print_log("INFO", "All System successfully initialized");

  /////////////////////////////////////////////
  ///////////////// 调度器开始 /////////////////
  ////////////////////////////////////////////
  srand(1);
  // MAXI needs actual physical address, not virtual address
  char *buffer = (char *)memalign(64, 4096 * 9216);

  if (buffer == NULL) {
    // Need to change to 0x40000000 in lscript.ld
    print_log("ERROR", "Failed to allocate memory for buffer");
    cleanup_platform();
    return XST_FAILURE;
  }
  uintptr_t buffer_phys_addr = get_physical_address(buffer);

  print_log("INFO", "LAUNCHING Versaslot run...");
  std::vector<Slot *> machines =
      initializeSlots(); // Instantiate 8 slots slot0 - slot7
  XTime i_time;
  XTime_GetTime(&i_time);
  double initial_time =
      1.0 * i_time / (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
  double initial_time_event = initial_time;
  int apps_added = 0;
  //   double event_delay = next_event_delay(10, 20);
  double event_delay = 2000 * 1000; // us
  bool is_app_added;
  double threshold = 1.0;
  std::list<Application *> pending_apps;
  std::list<Application *>
      candidate_apps; // Selected apps to be executed in slots
  std::list<Application *> retired_apps;
  std::list<Task *> ready_tasks;
  std::list<Task *> bundle_tasks;
  int D_task_blocked = 0; // Cumulative
  int D_PR = 0;           // Cumulative
  initialize_all_apps();

  // Manually add an application to pending_apps
  XTime_GetTime(&current_time);
  Application *initial_app = add_new_event(pending_apps, apps_added);

  if (initial_app == NULL) {
    print_log("ERROR", "Failed to generate initial application");
    cleanup_platform();
    return 1;
  }
  apps_added++;

  u64 program_duration;
  XTime last_pr_start = 0, last_pr_finished = 0;

  while (1) {
    updated_already = false;
    XTime c_time;
    XTime_GetTime(&c_time);
    double cur_time =
        1.0 * c_time /
        (static_cast<double>(COUNTS_PER_SECOND) / 1000000); // time in us
    double time_diff = cur_time - initial_time;
    double time_diff_event = cur_time - initial_time_event;

    // Trigger scheduling only when time exceeds scheduling interval
    if (time_diff > SCHED_INTERVAL) {
      initial_time = cur_time;
      threshold = update_priorities_app(pending_apps, candidate_apps);
      allocate_resources(candidate_apps, machines, ready_tasks);
      updated_already = true;
    }

    if (apps_added < MAX_APPS && time_diff_event > event_delay) {
      XTime_GetTime(&current_time);
      add_new_event(pending_apps, apps_added);
      if (!updated_already) {
        threshold = update_priorities_app(pending_apps, candidate_apps);
        updated_already = true;
      }

      initial_time_event = cur_time;
      ++apps_added;
    }
    // First check if pending_apps is empty
    if (!pending_apps.empty()) {
      is_app_added =
          update_candidate_apps(pending_apps, candidate_apps, threshold);
      if (is_app_added) {
        allocate_resources(candidate_apps, machines, ready_tasks);
      }
    }
    // Check if candidate_apps is empty
    if (!candidate_apps.empty()) {
      update_ready_tasks(candidate_apps, ready_tasks);
    }

    for (auto &slot : machines) {
      if (slot->isSlotIdle() && slot->curTask == nullptr &&
          slot->getType() == SLOT_L) {
        bundle_ready_tasks(ready_tasks);
      }
    }

    // xil_printf("task in task list:\n");
    // for (Task *tsk : ready_tasks) {
    //   xil_printf("%s, ", tsk->getTaskName().c_str());
    // }
    // xil_printf("\n");

    if (!ready_tasks.empty()) {
      execute_tasks(ready_tasks, machines, buffer_phys_addr, D_task_blocked,
                    D_PR, last_pr_start, last_pr_finished);
    }

    int N_app = 0;   // candidate_apps 中的应用程序数量
    int N_Batch = 0; // 所有应用程序的 batch 总数
    N_app = candidate_apps.size();
    for (Application *app : candidate_apps) {
      N_Batch += app->getBatch(); // 累加每个应用的 batch 数
    }

    std::list<Application *> new_retired_apps =
        retire_apps(candidate_apps, ready_tasks);
    retired_apps.splice(retired_apps.end(), new_retired_apps);
    if (new_retired_apps.size() != 0) {
      allocate_resources(candidate_apps, machines, ready_tasks);
    }

    if (retired_apps.size() > 4) {
      // Print or record N_app and N_Batch
      char app_batch_message[128];
      snprintf(app_batch_message, sizeof(app_batch_message),
               "N_Block_tasks: %d, N_PR: %d, N_apps: %d, N_Batch: %d\n",
               D_task_blocked, D_PR, N_app, N_Batch);
      print_log("INFO", app_batch_message);

      double final_metric =
          (static_cast<double>(D_task_blocked) * N_app) / (D_PR * N_Batch);

      char final_metric_message[128];
      snprintf(final_metric_message, sizeof(final_metric_message),
               "D_switch = %f\n", final_metric);
      print_log("INFO", final_metric_message);

      retired_apps.clear(); // Clear the retired_apps list
      D_task_blocked = 0;
      D_PR = 0;
    }

    // xil_printf("Apps in Candidate: \n");
    // for (Application *app : candidate_apps) {
    //   xil_printf("%s ", app->getAppName().c_str());
    // }
    // xil_printf("\n");

    // xil_printf("Tasks in ready Tasks: \n");
    // for (Task *task : ready_tasks) {
    //   xil_printf("%s ", task->getTaskName().c_str());
    // }
    // xil_printf("\n");

    // If all lists are empty and no tasks are running, break the loop
    XTime_GetTime(&current_time);
    program_duration = ((current_time - program_start_time) * 1000000) /
                       XPAR_CPU_TIMESTAMP_CLK_FREQ;

    // 如果所有列表都为空，且没有任务在运行，退出循环
    if (pending_apps.empty() && candidate_apps.empty() &&
        program_duration > 100 * 1000 * 1000) {
      break;
    }
  }

  print_log("INFO", "All tasks have been executed");
  for (const auto &entry : log_entries) {
    xil_printf("[%llu us] [%s] %s\n", entry->timestamp, entry->level.c_str(),
               entry->message.c_str());
  }

  cleanup_platform();
  return 0;
}
