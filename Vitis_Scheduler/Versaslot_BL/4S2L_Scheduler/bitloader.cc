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

#include "bitloader.h"
#include "ff.h" // 需要用于文件操作的函数
#include "xil_printf.h"
#include "xilffs.h" // FAT 文件系统头文件
#include <list>
#include <malloc.h> // 包含 memalign 函数
#include <map>
#include <string>
#include <vector>

FATFS fatfs; //全局文件系统

// 独立文件系统挂载的部分
int init_filesystem() {
  FRESULT res;

  // 挂载文件系统
  res = f_mount(&fatfs, "0:/", 1);
  if (res != FR_OK) {
    xil_printf("[ERROR] Failed to mount SD card: %d\n", res);
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}

// 文件读取和内存分配部分
int load_bitstream_to_memory(const char *BitstreamFile,
                             char **DestinationAddress, u32 *fileSize) {
  FIL fil;
  FRESULT res;
  UINT bytesRead;

  // 打开比特流文件
  res = f_open(&fil, BitstreamFile, FA_READ);
  if (res != FR_OK) {
    xil_printf("[ERROR] Failed to open file %s: %d\n", BitstreamFile, res);
    return XST_FAILURE;
  }

  // 获取文件大小
  *fileSize = f_size(&fil); // 通过指针返回文件大小

  // 动态分配内存，并确保分配成功
  *DestinationAddress =
      (char *)memalign(8, *fileSize); // 在src/lscript.ld中修改heap size！
  if (*DestinationAddress == NULL) {
    xil_printf("[ERROR] Failed to allocate memory for bitstream\n");
    f_close(&fil);
    return XST_FAILURE;
  }

  // 读取文件内容到目标内存地址
  res = f_read(&fil, (void *)*DestinationAddress, *fileSize, &bytesRead);
  if (res != FR_OK || bytesRead != *fileSize) {
    xil_printf("[ERROR] Failed to read file %s: %d, read %u bytes, expected "
               "%lu bytes\n",
               BitstreamFile, res, bytesRead, *fileSize);
    f_close(&fil);
    return XST_FAILURE;
  }

  // 关闭文件
  f_close(&fil);

  return XST_SUCCESS;
}
