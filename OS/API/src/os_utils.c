#include "os_utils.h"

static OS_TaskConfig_t task_list[OS_TASK_NUM]={0};
static uint8_t task_count = 0;

void OS_TaskRegister(OS_TaskConfig_t* cfg){
    // static uint8_t task_count = 0;
    OS_UTILS_ASSERT_INT(task_count < OS_TASK_NUM, "Task Register Overflow", task_count);
    task_list[task_count++] = *cfg;
}

void OS_TasksRegister(OS_TaskConfig_t cfg[],int num){
    // OS_UTILS_ASSERT_INT(task_count+num < OS_TASK_NUM, "Task Register Overflow", task_count);
    for(int i=0;i<num;++i){
        OS_TaskRegister(&cfg[i]);
    } 
}


void OS_GetTaskList(OS_TaskConfig_t** list){
    *list = task_list;
    // *count = task_count;
}

uint8_t OS_GetTaskCount(void){
    return task_count;
}