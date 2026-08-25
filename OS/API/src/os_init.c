#include "os_init.h"
#include "os_utils.h"

/*
*   Private Macro
*/
#define START_TASK_PRIO 1
#define START_STK_SIZE  128 

/*
*   Private Declaration
*/
static void startTask(void* pvParameters);
static OS_TaskHandle_t StartTaskHandle = NULL;

/*
*   Private Definition 
*/
static void startTask(void* pvParameters){
    OS_TaskConfig_t* task_list = NULL;
    OS_GetTaskList(&task_list);

    OS_EnterCritical();   // 统一临界区

    for(int i = 0; i < OS_GetTaskCount(); ++i){
        OS_TaskConfig_t* cur_task = &task_list[i];
        // 直接调用抽象接口创建任务
        OS_CreateTask(cur_task);
    }

    OS_TaskDelete(StartTaskHandle);   // 删除启动任务

    OS_ExitCritical();    // 退出临界区
}

/*
*   API
*/
void OS_Start(void){
    printf("OS Start\r\n");

    OS_TaskConfig_t start_task_config = {
        .func = startTask,
        .param = NULL,
        .name = "StartTask",
        .stack_depth = START_STK_SIZE,   // 注意字段名
        .priority = START_TASK_PRIO,     // 注意字段名
        .task_handle = &StartTaskHandle
    };

    OS_CreateTask(&start_task_config);      // 也可以换成 OS_OS_CreateTask，但保留原业务接口名
    OS_StartScheduler();                 // 统一启动调度器

    // printf("start the os successfully!\r\n");
}


