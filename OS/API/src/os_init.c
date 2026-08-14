
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
// static TaskFunction_t startTask;
static void startTask(void* pvParameters);

// extern TaskConfig_t task_list[];
static TaskHandle_t StartTaskHandle = NULL;

/*
*   Private Defination 
*/

// static TaskFunction_t startTask;
static void startTask(void* pvParameters){
    TaskConfig_t* task_list=NULL;
    OS_GetTaskList(&task_list);

    // OS_UTILS_ASSERT_INT(task_list != NULL, "task_list is NULL", 0);

    // TaskConfig_t task_list[]={
    //     // Print Task
    //     (TaskConfig_t){
    //         .name = "/test/print",
    //         .prio = 1,
    //         .func = OSTask_Print,
    //         .taskHandle = &PrintTaskHandle,
    //         .param = NULL,
    //         .stkSize = 128,
    //     },
    //     // Tap Count Task
    //     // (TaskConfig_t){
    //     //     .name = "/test/tap_count",
    //     //     .prio = 2,
    //     //     .func = OSTask_TapCount,
    //     //     .taskHandle = &TapCountTaskHandle,
    //     //     .param = NULL,
    //     //     .stkSize = 128,
    //     // }
    // };
    taskENTER_CRITICAL();

    for(int i=0;i< OS_GetTaskCount();++i){
        TaskConfig_t* cur_task = &task_list[i];
        xTaskCreate(
            cur_task->func,
            cur_task->name,
            cur_task->stkSize,
            cur_task->param,
            cur_task->prio,
            cur_task->taskHandle
            );
    }

    vTaskDelete(StartTaskHandle);

    taskEXIT_CRITICAL();

    // return NULL;
}



/*
*   API
*/
void OSStart(void){

    printf("OS Start\r\n");

    TaskConfig_t start_task_config = {
        .prio = START_TASK_PRIO,
        .stkSize = START_STK_SIZE,
        .name = "StartTask",
        .taskHandle = &StartTaskHandle,
        .param = NULL,
        .func = (TaskFunction_t)startTask
    };

    CreateTask(&start_task_config);
    // OS_UTILS_ASSERT(, err);
    vTaskStartScheduler();

    // printf("start the os successfully!\r\n");
}



