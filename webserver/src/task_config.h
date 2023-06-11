#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

// Defining these here for brevity. Include
// This file before AsyncTCP.h to ensure this configuration takes priority
#ifndef CONFIG_ASYNC_TCP_RUNNING_CORE
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#define CONFIG_ASYNC_TCP_USE_WDT 1
#endif

#include "FreeRTOS.h"

// Task priorities - higher number = higher prioritiy
const UBaseType_t PRIORITY_BLE_SCALES_MAINTAINANCE = 1;
const UBaseType_t PRIORITY_WEBSERVER_MAINTENANCE = 1;
const UBaseType_t PRIORITY_WIFI_MAINTENANCE  = 1;
const UBaseType_t PRIORITY_STM_COMMS  = 3;
const UBaseType_t PRIORITY_ASYNC_TCP = 3; // this is hardcoded in the AsyncTCP library. Adding it here for completeness

// Task cores. [-1] means fluid task not pinned to 1 core
const UBaseType_t CORE_BLE_SCALES_MAINTAINANCE = 0;
const UBaseType_t CORE_WIFI_MAINTENANCE  = 0;
const UBaseType_t CORE_WEBSERVER_MAINTENANCE = 0;
const UBaseType_t CORE_STM_COMMS  = 0;
const UBaseType_t CORE_ASYNC_TCP = CONFIG_ASYNC_TCP_RUNNING_CORE;

#endif
