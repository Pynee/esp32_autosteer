#ifndef AUTOSTEER_H
#define AUTOSTEER_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdio.h>
#include <string.h>
#include <AutoPID.h>
#include "configuration.h"

#include <sstream> // std::stringstream
#include <string>  // std::string

void initAutosteer();
void autosteerWorker(void *z);
extern uint8_t watchdogTimer;
extern double pidOutput;
extern AutoPID pid;

constexpr time_t Timeout = 1000;

#endif