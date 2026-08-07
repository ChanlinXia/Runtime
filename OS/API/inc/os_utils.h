/*
* @file os_utils.h
* @brief OS utils functions for running OS, only for FreeRTOS now
* @note This file is part of the 
*
*/

#ifndef OS_UTILS_H
#define OS_UTILS_H

#include "os_defs.h"

#define OS_UTILS_ASSERT_INT(x,info,err)\
    do{\
        if(!(x)){\
            printf("[OS_UTILS_ASSERT]:%s. Occured at %s:%d,error code:%ld\r\n",info,__FILE__,__LINE__,(err));\
            while(1){}\
        }\
    }while(0)

#define OS_UTILS_ASSERT_STR(x,info,err)\
    do{\
        if(!(x)){\
            printf("[OS_UTILS_ASSERT]:%s. Occured at %s,%d,error info:%s\r\n",info,__FILE__,__LINE__,(err));\
            while(1){}\
        }\
    }while(0)

/*
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName,
                            const configSTACK_DEPTH_TYPE uxStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask )*/

typedef struct task_config_t
{
  UBaseType_t   prio;                //优先级
  configSTACK_DEPTH_TYPE   stkSize;  //栈区大小
  TaskFunction_t  func;              //任务函数指针
  void* param;                       //参数指针
  char *name;                        //任务名
  TaskHandle_t* taskHandle;          //任务句柄
}TaskConfig_t;


/*
*   Create when OS is running
*/
static inline void CreateTask(TaskConfig_t* cfg){
    BaseType_t ret_val = pdPASS;

    vTaskSuspendAll(); 

    ret_val = xTaskCreate(
        cfg->func,
        cfg->name,
        cfg->stkSize,
        cfg->param,
        cfg->prio,
        cfg->taskHandle
    );

    xTaskResumeAll();

    OS_UTILS_ASSERT_STR(ret_val == pdPASS,"Failed to CreateTask",cfg->name);
}

static inline void CreateTasks(TaskConfig_t cfgs[],int task_num){

    vTaskSuspendAll(); 

    for(int i=0;i< task_num;++i){
        CreateTask(&cfgs[i]);
    }

    xTaskResumeAll();
}


#endif