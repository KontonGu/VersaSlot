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
// /*
//  * helloworld.c: simple test application
//  *
//  * This application configures UART 16550 to baud rate 9600.
//  * PS7 UART (Zynq) is not initialized by this application, since
//  * bootrom/bsp configures it to baud rate 115200
//  *
//  * ------------------------------------------------
//  * | UART TYPE   BAUD RATE                        |
//  * ------------------------------------------------
//  *   uartns550   9600
//  *   uartlite    Configurable only in HW design
//  *   ps7_uart    115200 (configured by bootrom/bsp)
//  */

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

#define SHARED_MEMORY_BASE 0xfffc0000

XFpga XFpgaInstance;             // 初始化全局 PL 对象
XGpio GpioDecouple;              // 初始化全局 Decouple 对象
XGpio GpioDecoupleStatus;        // 初始化全局 Decouple Status 对象
uint64_t program_start_time = 0; // 定义并初始化
int status;                      // 全局状态控制
volatile PR_Task *pr_task = (volatile PR_Task *)SHARED_MEMORY_BASE;

int main() {
  init_platform();     // Initialize platform
  Xil_DCacheDisable(); // Disable cache

  // 初始化硬件
  status = XFpga_Initialize(&XFpgaInstance);
  if (status != XST_SUCCESS) {
    xil_printf("PR Server: FPGA Initialization Failed\r\n");
    pr_task->init_success = 0; // 初始化失败
    return XST_FAILURE;
  }
  status = XGpio_Initialize(&GpioDecouple, XPAR_DECOUPLE_BASEADDR);
  if (status != XST_SUCCESS) {
    xil_printf("PR Server: GPIO Initialization Failed\r\n");
    pr_task->init_success = 0; // 初始化失败
    return XST_FAILURE;
  }

  status = init_filesystem(); // 初始化 SD 卡文件系统
  if (status != XST_SUCCESS) {
    xil_printf("PR Server: Failed to initialize SD FileSystem\r\n");
    pr_task->init_success = 0; // 初始化失败
    return XST_FAILURE;
  }

  pr_task->init_success = 1; // 初始化成功，通知核心 0
  pr_task->pr_status = 0;
  xil_printf("PR Server Ready\n");

  // 不断检查 pr_status，等待 PR 请求
  while (1) {
    if (pr_task->pr_status == 1) { // Check for PR request
      pr_task->pr_status = 2;
      // Load bitstream from SD to memory
      char *PartialBit = NULL;
      u32 fileSize = 0;
      status = load_bitstream_to_memory(
          (const char *)pr_task->bitstream_file_name, &PartialBit, &fileSize);

      if (status != XST_SUCCESS) {
        pr_task->pr_status = 4; // Reset status if failed
        continue;
      }

      // Start FPGA reconfiguration
      XGpio_DiscreteWrite(&GpioDecouple, 1,
                          pr_task->decouple_value); // Decouple
      status = XFpga_BitStream_Load(&XFpgaInstance, (UINTPTR)PartialBit, 0,
                                    fileSize, XFPGA_PARTIAL_EN);

      if (status != XST_SUCCESS) {
        pr_task->pr_status = 4; // Reset on failure
      } else {
        XGpio_DiscreteWrite(&GpioDecouple, 1, 0x0); // Re-enable decouple
        // usleep(100);
        free(PartialBit);
        pr_task->pr_status = 3; // PR complete
      }
    }
  }
  cleanup_platform();
  return 0;
}
