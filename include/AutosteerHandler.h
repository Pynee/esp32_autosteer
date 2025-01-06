#ifndef AUTOSTEER_H
#define AUTOSTEER_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdio.h>
#include <string.h>
#include <AutoPID.h>
#include "configuration.h"
#include "GlobalVariables.h"

#include <sstream> // std::stringstream
#include <string>  // std::string

class AutosteerHandler
{
private:
    static void startTaskImpl(void *);
    void autosteerTask(void *z);
    double pidOutput = 0;
    AutoPID pid;

public:
    AutosteerHandler();
    void init();
};

#endif