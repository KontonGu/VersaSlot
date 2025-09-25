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

#ifndef APPGEN_H
#define APPGEN_H

#include "scheduler.h"
#include <list>

Application *appgenerator_IC(int batch, int optimal_S, int optimal_L,
                             double priority, int priority_level);
Application *appgenerator_LENet(int batch, int optimal_S, int optimal_L,
                                double priority, int priority_level);
Application *appgenerator_3DR(int batch, int optimal_S, int optimal_L,
                              double priority, int priority_level);
Application *appgenerator_OF(int batch, int optimal_S, int optimal_L,
                             double priority, int priority_level);
#endif
