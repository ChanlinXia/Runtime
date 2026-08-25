// os_backend_freertos_inline.h
#ifndef OS_BACKEND_FREERTOS_INLINE_H
#define OS_BACKEND_FREERTOS_INLINE_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

// 注意：先取消之前的定义，再重新定义
#undef OS_MS_TO_TICKS
#define OS_MS_TO_TICKS(ms)  pdMS_TO_TICKS(ms)


/* ==================== 任务 ==================== */
static inline OS_Status_t OS_CreateTask(OS_TaskConfig_t *cfg) {
    TaskHandle_t h = NULL;
    BaseType_t ret = xTaskCreate(
        (TaskFunction_t)cfg->func,
        cfg->name,
        (configSTACK_DEPTH_TYPE)cfg->stack_depth,  // FreeRTOS 的单位是“字”
        cfg->param,
        (UBaseType_t)cfg->priority,
        &h
    );

    if (ret == pdPASS && cfg->is_suspended) {
        vTaskSuspend(h);   // 创建后立即挂起
    }

    if (cfg->task_handle) {
        *(OS_TaskHandle_t*)cfg->task_handle = (OS_TaskHandle_t)h;
    }
    return (ret == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_CreateTasks(OS_TaskConfig_t cfgs[], int task_num) {
    vTaskSuspendAll();   // 挂起调度器，批量创建
    OS_Status_t status = OS_SUCCESS;
    for (int i = 0; i < task_num; ++i) {
        if (OS_CreateTask(&cfgs[i]) != OS_SUCCESS) {
            status = OS_FAIL;
            break;
        }
    }
    xTaskResumeAll();
    return status;
}

static inline void OS_TaskDelay(OS_TickMs_t ms) {
    TickType_t ticks = (ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(ms);
    vTaskDelay(ticks);
}

static inline OS_TickMs_t OS_GetTickMs(void) {
    return (OS_TickMs_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static inline void OS_TaskDelete(OS_TaskHandle_t task) {
    vTaskDelete((TaskHandle_t)task);
}


static inline void OS_StartScheduler(void) {
    vTaskStartScheduler();
}

static inline void OS_TaskSuspend(OS_TaskHandle_t task) {
    vTaskSuspend((TaskHandle_t)task);
}

// static inline void OS_TaskSuspendFromISR(OS_TaskHandle_t task) {
//     vTaskSuspendFromISR((TaskHandle_t)task);
// }

static inline void OS_TaskResume(OS_TaskHandle_t task) {
    vTaskResume((TaskHandle_t)task);
}

static inline void OS_TaskResumeFromISR(OS_TaskHandle_t task) {
    xTaskResumeFromISR((TaskHandle_t)task);
}


/* ==================== 信号量 ==================== */
static inline OS_SemaphoreHandle_t OS_SemaphoreCreate(uint32_t max_count, uint32_t init_count) {
    SemaphoreHandle_t sem;
    if (max_count == 1) {
        sem = xSemaphoreCreateBinary();
        if (sem && init_count > 0) {
            xSemaphoreGive(sem);  // 初始给一次
        }
    } else {
        sem = xSemaphoreCreateCounting(max_count, init_count);
    }
    return (OS_SemaphoreHandle_t)sem;
}

static inline OS_Status_t OS_SemaphoreTake(OS_SemaphoreHandle_t sem, OS_TickMs_t timeout_ms) {
    TickType_t ticks = (timeout_ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTake((SemaphoreHandle_t)sem, ticks) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_SemaphoreGive(OS_SemaphoreHandle_t sem) {
    return (xSemaphoreGive((SemaphoreHandle_t)sem) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_SemaphoreGiveFromISR(OS_SemaphoreHandle_t sem, int *pxHigherPriorityTaskWoken) {
    return (xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, (BaseType_t*)pxHigherPriorityTaskWoken) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

/* ==================== 互斥量 ==================== */
static inline OS_MutexHandle_t OS_MutexCreate(void) {
    return (OS_MutexHandle_t)xSemaphoreCreateMutex();
}

static inline OS_Status_t OS_MutexTake(OS_MutexHandle_t mutex, OS_TickMs_t timeout_ms) {
    TickType_t ticks = (timeout_ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTake((SemaphoreHandle_t)mutex, ticks) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_MutexGive(OS_MutexHandle_t mutex) {
    return (xSemaphoreGive((SemaphoreHandle_t)mutex) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

/* ==================== 队列 ==================== */
static inline OS_QueueHandle_t OS_QueueCreate(uint32_t queue_len, uint32_t item_size) {
    return (OS_QueueHandle_t)xQueueCreate(queue_len, item_size);
}

static inline OS_Status_t OS_QueueSend(OS_QueueHandle_t queue, const void *item, OS_TickMs_t timeout_ms) {
    TickType_t ticks = (timeout_ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xQueueSend((QueueHandle_t)queue, item, ticks) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_QueueReceive(OS_QueueHandle_t queue, void *buffer, OS_TickMs_t timeout_ms) {
    TickType_t ticks = (timeout_ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xQueueReceive((QueueHandle_t)queue, buffer, ticks) == pdPASS) ? OS_SUCCESS : OS_FAIL;
}

static inline uint32_t OS_QueueMessagesWaiting(OS_QueueHandle_t queue) {
    return (uint32_t)uxQueueMessagesWaiting((QueueHandle_t)queue);
}

/* ==================== 事件组 ==================== */
static inline OS_EventGroupHandle_t OS_EventGroupCreate(void) {
    return (OS_EventGroupHandle_t)xEventGroupCreate();
}

static inline uint32_t OS_EventGroupWaitBits(OS_EventGroupHandle_t evt, uint32_t bits_to_wait,
                                             int clear_on_exit, int wait_all, OS_TickMs_t timeout_ms) {
    TickType_t ticks = (timeout_ms == OS_WAIT_FOREVER) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (uint32_t)xEventGroupWaitBits(
        (EventGroupHandle_t)evt,
        bits_to_wait,
        (BaseType_t)clear_on_exit,
        (BaseType_t)wait_all,
        ticks
    );
}

static inline uint32_t OS_EventGroupSetBits(OS_EventGroupHandle_t evt, uint32_t bits) {
    return (uint32_t)xEventGroupSetBits((EventGroupHandle_t)evt, bits);
}

static inline uint32_t OS_EventGroupClearBits(OS_EventGroupHandle_t evt, uint32_t bits) {
    return (uint32_t)xEventGroupClearBits((EventGroupHandle_t)evt, bits);
}

/* ==================== 临界区 ==================== */
static inline void OS_EnterCritical(void) {
    taskENTER_CRITICAL();
}

static inline void OS_ExitCritical(void) {
    taskEXIT_CRITICAL();
}

#endif