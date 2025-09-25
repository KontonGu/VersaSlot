#include "scheduler.h"
#include <list>

Application *appgenerator_IC(int batch, int parallelism, double priority,
                             int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("IC", batch, std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->parallelism = parallelism;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0
  app->optimal = app->parallelism;

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct,
  // quantize, dequantize, idct, recon
  Task *task1 = new Task("bundle1", app, 943388, 128, batch, 0);
  Task *task2 = new Task("dct", app, 684727, 128, batch, 1);
  Task *task3 = new Task("quantize", app, 133772, 128, batch, 2);
  Task *task4 = new Task("dequantize", app, 133991, 128, batch, 3);
  Task *task5 = new Task("idct", app, 703851, 128, batch, 4);
  Task *task6 = new Task("recon", app, 869325, 128, batch, 5); //重测

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);
  app->addTask(task4);
  app->addTask(task5);
  app->addTask(task6);

  app->defineDependencies();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_LENet(int batch, int parallelism, double priority,
                                int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("LEN", batch, std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->parallelism = parallelism;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0
  app->optimal = app->parallelism;

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("letsk1", app, 113300, 300, batch, 0);
  Task *task2 = new Task("letsk2", app, 50575, 205, batch, 1);
  Task *task3 = new Task("letsk3", app, 113179, 299, batch, 2);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);

  app->defineDependencies();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_3DR(int batch, int parallelism, double priority,
                              int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("3DR", batch, std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->parallelism = parallelism;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0
  app->optimal = app->parallelism;

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("3drtsk1", app, 178684, 385, batch, 0);
  Task *task2 = new Task("3drtsk2", app, 106395, 300, batch, 1);
  Task *task3 = new Task("3drtsk3", app, 35345, 170, batch, 2);

  // 将任务加入 Application 的任务列表
  app->addTask(task1);
  app->addTask(task2);
  app->addTask(task3);

  app->defineDependencies();

  // 返回创建的 Application 对象
  return app;
}

Application *appgenerator_OF(int batch, int parallelism, double priority,
                             int priority_level) {
  // 创建一个 Application 对象，名字为 "IC"，batch 号由参数指定
  Application *app = new Application("OF", batch, std::vector<Task *>());

  // 初始化应用的优先级、并行任务数、阈值、优先级等级以及已完成任务数
  app->priority = priority;
  app->priority_level = priority_level;
  app->parallelism = parallelism;
  app->threshold = 0;       // 默认初始化为 0
  app->tasks_completed = 0; // 默认初始化为 0
  app->optimal = app->parallelism;

  // 创建 6 个任务，任务名和latency值依次为 bundle1, dct, quantize, dequantize,
  // idct, recon
  Task *task1 = new Task("oftsk1", app, 843610, 825, batch, 0);
  Task *task2 = new Task("oftsk2", app, 232166, 440, batch, 1);
  Task *task3 = new Task("oftsk3", app, 185482, 390, batch, 2);
  Task *task4 = new Task("oftsk4", app, 492121, 635, batch, 3);
  Task *task5 = new Task("oftsk5", app, 504672, 640, batch, 4);
  Task *task6 = new Task("oftsk6", app, 756930, 785, batch, 5);
  Task *task7 = new Task("oftsk7", app, 783327, 798, batch, 6);
  Task *task8 = new Task("oftsk8", app, 745533, 779, batch, 7);
  Task *task9 = new Task("oftsk9", app, 377923, 558, batch, 8);

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

  // 返回创建的 Application 对象
  return app;
}