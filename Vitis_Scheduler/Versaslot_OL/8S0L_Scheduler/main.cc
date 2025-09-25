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

#define SCHED_INTERVAL 400000
#define MAX_APPS 100
#define SHARED_MEMORY_BASE 0xfffc0000
volatile PR_Task *pr_task = (volatile PR_Task *)SHARED_MEMORY_BASE;

XFpga XFpgaInstance;      // 初始化全局 PL 对象
XGpio GpioDecouple;       // 初始化全局 Decouple 对象
XGpio GpioDecoupleStatus; // 初始化全局 Decouple Status 对象
int status;               // 全局状态控制
XTime start_time, current_time, program_start_time;
volatile int spin = 0;
bool updated_already = false;

int main() {

  /////////////////////////////////////////////
  ///////////////// 初始化系统 /////////////////
  ////////////////////////////////////////////

  init_platform(); // 初始化平台

  // 获取程序启动时的时间，作为基准时间
  XTime_GetTime(&program_start_time);

  Xil_DCacheDisable(); // 禁用缓存

  print_log("INFO", "System Initializing ...");

  status = init_filesystem(); // 初始化 SD 卡文件系统
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to initialize SD FileSystem");
    cleanup_platform();
    return XST_FAILURE;
  }
  print_log("INFO", "SD FileSystem successfully initialized");

  status = XFpga_Initialize(&XFpgaInstance); // 初始化 FPGA 驱动
  if (status != XST_SUCCESS) {
    print_log("ERROR", "Failed to initialize PL Part");
    cleanup_platform();
    return XST_FAILURE;
  }
  print_log("INFO", "FPGA successfully initialized");

  status = XGpio_Initialize(
      &GpioDecouple, XPAR_DECOUPLE_BASEADDR); // 初始化 GPIO 控制器 Decouple
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
  // MAXI需要实际物理地址而不是虚拟地址
  char *buffer = (char *)memalign(64, 4096 * 9216);

  if (buffer == NULL) {
    //需要在lscript.ld中改为4000 0000
    xil_printf("[ERROR] Failed to allocate memory for buffer\n");
    cleanup_platform();
    return XST_FAILURE;
  }
  uintptr_t buffer_phys_addr = get_physical_address(buffer);

  print_log("INFO", "LAUNCHING Versaslot run...");
  std::vector<Slot *> machines = initializeSlots(); // 实例化 8 个 slot0 - slot7
  XTime i_time;
  XTime_GetTime(&i_time);
  double initial_time =
      1.0 * i_time / (static_cast<double>(COUNTS_PER_SECOND) / 1000000);
  double initial_time_event = initial_time;
  int apps_added = 0;
  //   double event_delay = next_event_delay(0, 50);
  double event_delay = 2000 * 1000; // us
  double threshold = 1.0;
  bool is_app_added;
  std::list<Application *> pending_apps;
  std::list<Application *> candidate_apps; //被选择出来的app，进入slot进行执行
  std::list<Application *> retired_apps;
  std::list<Task *> ready_tasks;
  int D_task_blocked = 0; //累计
  int D_PR = 0;           //累计
  initialize_all_apps();

  // 手动添加一个应用程序到 pending_apps
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

    //只有当时间大于调度周期时才触发调度
    if (time_diff > SCHED_INTERVAL) {
      //   xil_printf("time_diff is greater than SCHED_INTERVAL\n");
      initial_time = cur_time;
      threshold = update_priorities_app(pending_apps, candidate_apps);
      allocate_resources(candidate_apps);
      updated_already = true;
    }

    if (apps_added < MAX_APPS && time_diff_event > event_delay) {
      //   xil_printf("time_diff_event is greater than event_delay\n");
      XTime_GetTime(&current_time);
      add_new_event(pending_apps, apps_added);
      if (!updated_already) {
        threshold = update_priorities_app(pending_apps, candidate_apps);
        updated_already = true;
      }

      initial_time_event = cur_time;
      ++apps_added;
    }
    // 先检查 pending_apps 是否为空
    if (!pending_apps.empty()) {
      is_app_added =
          update_candidate_apps(pending_apps, candidate_apps, threshold);
      if (is_app_added) {
        allocate_resources(candidate_apps);
      }
    }

    // xil_printf("Apps in Candidate: \n");
    // for (Application *app : candidate_apps) {
    //   xil_printf("%s ", app->getAppName().c_str());
    // }
    // xil_printf("\n");

    // 检查 candidate_apps 是否为空
    if (!candidate_apps.empty()) {
      update_ready_tasks(candidate_apps, ready_tasks);
    }

    // xil_printf("Tasks in ready Tasks: \n");
    // for (Task *task : ready_tasks) {
    //   xil_printf("%s ", task->getTaskName().c_str());
    // }
    // xil_printf("\n");

    if (!ready_tasks.empty()) {
      execute_tasks(ready_tasks, machines, buffer_phys_addr, D_task_blocked,
                    D_PR, candidate_apps, last_pr_start, last_pr_finished);
    }

    // for (Application *app : candidate_apps) {
    //   xil_printf("App: %s Completed task: %d\n", app->getAppName().c_str(),
    //              app->tasks_completed);
    // }
    int N_app = 0;   // candidate_apps 中的应用程序数量
    int N_Batch = 0; // 所有应用程序的 batch 总数
    N_app = candidate_apps.size();
    for (Application *app : candidate_apps) {
      N_Batch += app->getBatch(); // 累加每个应用的 batch 数
    }

    std::list<Application *> new_retired_apps = retire_apps(candidate_apps);
    retired_apps.splice(retired_apps.end(), new_retired_apps);
    if (new_retired_apps.size() != 0) {
      allocate_resources(candidate_apps);
    }

    if (retired_apps.size() > 4) {
      // 打印或记录 N_app 和 N_Batch
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

      retired_apps.clear(); // 清空 retired_apps 列表
      D_task_blocked = 0;
      D_PR = 0;
    }

    // 如果所有列表都为空，且没有任务在运行，退出循环
    XTime_GetTime(&current_time);
    program_duration = ((current_time - program_start_time) * 1000000) /
                       XPAR_CPU_TIMESTAMP_CLK_FREQ;

    // 如果所有列表都为空，且没有任务在运行，退出循环
    if (pending_apps.empty() && candidate_apps.empty() &&
        program_duration > 450 * 1000 * 1000) {
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