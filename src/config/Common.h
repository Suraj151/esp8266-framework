/*************************** Common Config page *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/
#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

#include <utility/DataTypeDef.h>

/**
 * @define common time durations
 */
#define MILLISECOND_DURATION_1000   1000
#define MILLISECOND_DURATION_5000   5000
#define MILLISECOND_DURATION_10000  10000

/**
 * @define default username/ssid and password
 */
#if defined(ENABLE_HTTP_SERVER) || defined(ENABLE_AUTH_SERVICE) || defined(ENABLE_WIFI_SERVICE)
#define USER            "pdiStack"
#define PASSPHRASE      "pdiStack@123"
#endif

/**
 * @define general http parameters
 */
#define HTTP_HOST_ADDR_MAX_SIZE 100
#define HTTP_REQUEST_DURATION   MILLISECOND_DURATION_10000
#define HTTP_REQUEST_RETRY      1

/**
 * max tasks and callbacks
 */
#ifndef MAX_SCHEDULABLE_TASKS
#define MAX_SCHEDULABLE_TASKS	25
#endif
#define MAX_FACTORY_RESET_CALLBACKS	MAX_SCHEDULABLE_TASKS

#ifndef TASK_PRIORITY_WEIGHT_MIN
#define TASK_PRIORITY_WEIGHT_MIN 50
#endif
#ifndef TASK_PRIORITY_WEIGHT_MAX
#define TASK_PRIORITY_WEIGHT_MAX 100
#endif

#define TASK_WEIGHT_LEVELS (MAX_TASK_PRIORITY + 1)  /* one ladder step per priority */
#define TASK_WEIGHT_NOMINAL 1024
#define TASK_WEIGHT_SPAN 9            /* natural log of the ladder's lightest to heaviest */
#define TASK_WEIGHT_STEP_NUM (TASK_WEIGHT_LEVELS + TASK_WEIGHT_SPAN)
#define TASK_WEIGHT_STEP_DEN TASK_WEIGHT_LEVELS
#define TASK_WEIGHT_DEADLINE_BOOST 4  /* a deadline task is charged this much less */

/**
 * highest task id handed out before ids start again from one
 */
#ifndef MAX_TASK_ID
#define MAX_TASK_ID	32000
#endif


#endif
