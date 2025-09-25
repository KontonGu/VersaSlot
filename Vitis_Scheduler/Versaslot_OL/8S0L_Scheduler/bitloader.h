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

#ifndef BITLOADER_H
#define BITLOADER_H

#include <iostream>
#include <vector>
#include <xil_types.h>

int init_filesystem();
int load_bitstream_to_memory(const char *BitstreamFile,
                             char **DestinationAddress, u32 *fileSize);

#endif // BITLOADER_H