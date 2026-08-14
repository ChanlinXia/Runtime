#include "os_utils.h"

static TaskConfig_t task_list[OS_TASK_NUM]={0};
static uint8_t task_count = 0;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // 处理栈溢出，比如记录日志、进入安全状态等
    // 至少提供一个空函数体，否则链接会报错
    OS_UTILS_ASSERT_STR(0, "Task Stack OverFlow",pcTaskName);
}

void OS_TaskRegister(TaskConfig_t* cfg){
    // static uint8_t task_count = 0;
    OS_UTILS_ASSERT_INT(task_count < OS_TASK_NUM, "Task Register Overflow", task_count);
    task_list[task_count++] = *cfg;
}

void OS_GetTaskList(TaskConfig_t** list){
    *list = task_list;
    // *count = task_count;
}

uint8_t OS_GetTaskCount(void){
    return task_count;
}