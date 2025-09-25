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

#include "scheduler.h"
#include <list>

Application *appgenerator_IC(int batch, int optimal_S, int optimal_L,
                             double priority, int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("IC", batch, std::vector<Task *>(),
                                     std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->optimal_S = optimal_S;
  app->optimal_L = optimal_L;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize,
  // dequantize, idct, recon
  Task *task1 = new Task("BUN", app, 943388, 160, batch, TASK_S, 0, 2.31);
  Task *task2 = new Task("DCT", app, 684727, 128, batch, TASK_S, 1, 2.31);
  Task *task3 = new Task("QUA", app, 133772, 128, batch, TASK_S, 2, 2.31);
  Task *task4 = new Task("DEQ", app, 133991, 128, batch, TASK_S, 3, 2.07);
  Task *task5 = new Task("IDC", app, 703851, 128, batch, TASK_S, 4, 2.07);
  Task *task6 = new Task("REC", app, 869325, 128, batch, TASK_S, 5, 2.07);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);
  app->addTask(task4);
  app->addTask(task5);
  app->addTask(task6);

  app->defineDependencies();

  Task *task_3in1_1 = new Task("BDQ_S", app, 0, 128, batch, TASK_L, 0, 0);
  Task *task_3in1_2 = new Task("BDQ_P", app, 0, 150, batch + 2, TASK_L, 1, 0);
  Task *task_3in1_3 = new Task("DIR_S", app, 0, 128, batch, TASK_L, 0, 0);
  Task *task_3in1_4 = new Task("DIR_P", app, 0, 128, batch + 2, TASK_L, 1, 0);

  app->addTask_3in1(task_3in1_1);
  app->addTask_3in1(task_3in1_2);
  app->addTask_3in1(task_3in1_3);
  app->addTask_3in1(task_3in1_4);

  app->defineDependencies3in1();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_LENet(int batch, int optimal_S, int optimal_L,
                                double priority, int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("LEN", batch, std::vector<Task *>(),
                                     std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->optimal_S = optimal_S;
  app->optimal_L = optimal_L;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("LET1", app, 113300, 376, batch, TASK_S, 0, 1.38);
  Task *task2 = new Task("LET2", app, 50575, 255, batch, TASK_S, 1, 1.38);
  Task *task3 = new Task("LET3", app, 113179, 376, batch, TASK_S, 2, 1.38);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);

  app->defineDependencies();
  // 62628 us 156,570us

  Task *task_3in1_1 = new Task("LEN_S", app, 0, 390, batch, TASK_L, 0, 0);
  Task *task_3in1_2 = new Task("LEN_P", app, 0, 248, batch + 2, TASK_L, 1, 0);

  app->addTask_3in1(task_3in1_1);
  app->addTask_3in1(task_3in1_2);

  app->defineDependencies3in1();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_3DR(int batch, int optimal_S, int optimal_L,
                              double priority, int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("3DR", batch, std::vector<Task *>(),
                                     std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->optimal_S = optimal_S;
  app->optimal_L = optimal_L;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("3DRTSK1", app, 178684, 477, batch, TASK_S, 0, 2.52);
  Task *task2 = new Task("3DRTSK2", app, 106395, 379, batch, TASK_S, 1, 2.52);
  Task *task3 = new Task("3DRTSK3", app, 35345, 218, batch, TASK_S, 2, 2.52);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);

  app->defineDependencies();

  Task *task_3in1_1 = new Task("3DR_S", app, 0, 450, batch, TASK_L, 0, 0);
  Task *task_3in1_2 = new Task("3DR_P", app, 0, 320, batch + 2, TASK_L, 1, 0);

  app->addTask_3in1(task_3in1_1);
  app->addTask_3in1(task_3in1_2);

  app->defineDependencies3in1();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_OF(int batch, int optimal_S, int optimal_L,
                             double priority, int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("OF", batch, std::vector<Task *>(),
                                     std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->optimal_S = optimal_S;
  app->optimal_L = optimal_L;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("OFTSK1", app, 843610, 1038, batch, TASK_S, 0, 1.52);
  Task *task2 = new Task("OFTSK2", app, 232166, 554, batch, TASK_S, 1, 1.52);
  Task *task3 = new Task("OFTSK3", app, 185482, 492, batch, TASK_S, 2, 1.52);
  Task *task4 = new Task("OFTSK4", app, 492121, 800, batch, TASK_S, 3, 1.52);
  Task *task5 = new Task("OFTSK5", app, 504672, 805, batch, TASK_S, 4, 1.52);
  Task *task6 = new Task("OFTSK6", app, 756930, 989, batch, TASK_S, 5, 1.52);
  Task *task7 = new Task("OFTSK7", app, 783327, 1010, batch, TASK_S, 6, 1.39);
  Task *task8 = new Task("OFTSK8", app, 745533, 986, batch, TASK_S, 7, 1.39);
  Task *task9 = new Task("OFTSK9", app, 377923, 707, batch, TASK_S, 8, 1.39);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);
  app->addTask(task4);
  app->addTask(task5);
  app->addTask(task6);
  app->addTask(task7);
  app->addTask(task8);
  app->addTask(task9);

  app->defineDependencies();

  Task *task_3in1_1 = new Task("OFB1_S", app, 0, 820, batch, TASK_L, 0, 0);
  Task *task_3in1_2 = new Task("OFB1_P", app, 0, 675, batch + 2, TASK_L, 1, 0);
  Task *task_3in1_3 = new Task("OFB2_S", app, 0, 1020, batch, TASK_L, 0, 0);
  Task *task_3in1_4 = new Task("OFB2_P", app, 0, 650, batch + 2, TASK_L, 1, 0);
  Task *task_3in1_5 = new Task("OFB3_S", app, 0, 1030, batch, TASK_L, 0, 0);
  Task *task_3in1_6 = new Task("OFB3_P", app, 0, 660, batch + 2, TASK_L, 1, 0);

  app->addTask_3in1(task_3in1_1);
  app->addTask_3in1(task_3in1_2);
  app->addTask_3in1(task_3in1_3);
  app->addTask_3in1(task_3in1_4);
  app->addTask_3in1(task_3in1_5);
  app->addTask_3in1(task_3in1_6);

  app->defineDependencies3in1();

  // 返回创建的 Application 对象
  return app;
}