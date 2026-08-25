#include "FreeRTOS.h"
#include "os_utils.h"

void vApplicationStackOverflowHook(struct tskTaskControlBlock * xTask, char *pcTaskName)
{
    // 处理栈溢出，比如记录日志、进入安全状态等
    // 至少提供一个空函数体，否则链接会报错
    OS_UTILS_ASSERT_STR(0, "Task Stack OverFlow",pcTaskName);
}