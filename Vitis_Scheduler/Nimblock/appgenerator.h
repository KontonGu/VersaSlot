#ifndef APPGEN_H
#define APPGEN_H

#include "scheduler.h"
#include <list>

Application *appgenerator_IC(int batch, int parallelism, double priority,
                             int priority_level);
Application *appgenerator_LENet(int batch, int parallelism, double priority,
                                int priority_level);
Application *appgenerator_3DR(int batch, int parallelism, double priority,
                              int priority_level);
Application *appgenerator_OF(int batch, int parallelism, double priority,
                             int priority_level);
#endif
