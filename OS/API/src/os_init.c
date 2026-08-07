
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
static void OSTask_Print(void* pvParameters);

static TaskHandle_t StartTaskHandle = NULL;
static TaskHandle_t PrintTaskHandle = NULL;

// static TaskFunction_t startTask;
/*
*   Private Defination 
*/
static void startTask(void* pvParameters){

    TaskConfig_t task_list[]={
        // Print Task
        (TaskConfig_t){
            .name = "/test/print",
            .prio = 1,
            .func = OSTask_Print,
            .taskHandle = &PrintTaskHandle,
            .param = NULL,
            .stkSize = 128,
        }
    };
    taskENTER_CRITICAL();

    for(int i=0;i< sizeof(task_list) / sizeof(TaskConfig_t);++i){
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

static void OSTask_Print(void* pvParameters){
    static uint32_t count = 0;
    while(1){
        printf("OSTask_Print is ongoing:%lu\r\n", (unsigned long)count++);
        vTaskDelay(1000);
    }
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

    printf("start the os successfully!\r\n");
}



