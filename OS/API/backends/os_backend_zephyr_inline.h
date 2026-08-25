// os_backend_zephyr_inline.h
#ifndef OS_BACKEND_ZEPHYR_INLINE_H
#define OS_BACKEND_ZEPHYR_INLINE_H

#include <zephyr/kernel.h>

/* ==================== 静态对象池大小 ==================== */
#ifndef OS_ZEPHYR_MAX_OBJECTS
#define OS_ZEPHYR_MAX_OBJECTS  16
#endif

/* ==================== 线程栈池 ==================== */
// Zephyr 需要用静态数组作为线程栈，单位是字节
static char __thread_stack_pool[OS_ZEPHYR_MAX_OBJECTS][CONFIG_MINIMAL_STACK_SIZE * 4] __attribute__((aligned(8)));
static struct k_thread __thread_pool[OS_ZEPHYR_MAX_OBJECTS];
static uint8_t __thread_idx = 0;

/* ==================== 信号量池 ==================== */
static struct k_sem __sem_pool[OS_ZEPHYR_MAX_OBJECTS];
static uint8_t __sem_idx = 0;

/* ==================== 互斥量池 ==================== */
static struct k_mutex __mutex_pool[OS_ZEPHYR_MAX_OBJECTS];
static uint8_t __mutex_idx = 0;

/* ==================== 队列（消息队列）池 ==================== */
// 每个队列预设 4 条消息，每条消息 32 字节（可根据需要调整）
#define OS_ZEPHYR_MSGQ_ITEMS  4
#define OS_ZEPHYR_MSGQ_SIZE   32
static struct k_msgq __queue_pool[OS_ZEPHYR_MAX_OBJECTS];
static char __queue_buffer_pool[OS_ZEPHYR_MAX_OBJECTS][OS_ZEPHYR_MSGQ_ITEMS * OS_ZEPHYR_MSGQ_SIZE];
static uint8_t __queue_idx = 0;

/* ==================== 事件组池 ==================== */
static struct k_event __event_pool[OS_ZEPHYR_MAX_OBJECTS];
static uint8_t __event_idx = 0;

/* ==================== 优先级映射（Zephyr 数值越小优先级越高） ==================== */
// 这里将用户传入的高数值（如 5）映射到 Zephyr 的低数值（如 CONFIG_NUM_PREEMPT_PRIORITIES - 5 - 1）
// 用户优先级范围建议：0 ~ (CONFIG_NUM_PREEMPT_PRIORITIES - 1)
#define OS_PRIO_TO_ZEPHYR(p)  (CONFIG_NUM_PREEMPT_PRIORITIES - (p) - 1)

/* ==================== 任务 ==================== */
static inline OS_Status_t OS_CreateTask(OS_TaskConfig_t *cfg) {
    if (__thread_idx >= OS_ZEPHYR_MAX_OBJECTS) {
        return OS_FAIL;
    }

    // 将用户栈大小（字节）与最小栈取较大值
    size_t stack_sz = cfg->stack_depth;
    if (stack_sz < CONFIG_MINIMAL_STACK_SIZE) {
        stack_sz = CONFIG_MINIMAL_STACK_SIZE;
    }

    k_tid_t tid = k_thread_create(
        &__thread_pool[__thread_idx],               // 线程控制块
        __thread_stack_pool[__thread_idx],          // 栈内存
        sizeof(__thread_stack_pool[__thread_idx]),  // 栈大小（字节）
        (k_thread_entry_t)cfg->func,
        cfg->param,
        NULL,                                       // 协程参数（不用）
        OS_PRIO_TO_ZEPHYR(cfg->priority),           // Zephyr 优先级
        0,                                          // 选项
        K_NO_WAIT                                   // 立即启动
    );

    if (tid) {
        if (cfg->task_handle) {
            *(OS_TaskHandle_t*)cfg->task_handle = (OS_TaskHandle_t)tid;
        }
        __thread_idx++;
        return OS_SUCCESS;
    }
    return OS_FAIL;
}

static inline OS_Status_t OS_CreateTasks(OS_TaskConfig_t cfgs[], int task_num) {
    k_sched_lock();   // 锁调度器，批量创建
    OS_Status_t status = OS_SUCCESS;
    for (int i = 0; i < task_num; ++i) {
        if (OS_CreateTask(&cfgs[i]) != OS_SUCCESS) {
            status = OS_FAIL;
            break;
        }
    }
    k_sched_unlock();
    return status;
}

static inline void OS_TaskDelay(OS_TickMs_t ms) {
    k_timeout_t to = (ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(ms);
    k_sleep(to);
}

static inline OS_TickMs_t OS_GetTickMs(void) {
    return (OS_TickMs_t)k_uptime_get();
}

/* ==================== 任务删除 ==================== */
static inline void OS_TaskDelete(OS_TaskHandle_t task) {
    // Zephyr 中删除线程：k_thread_abort
    k_thread_abort((k_tid_t)task);
}

/* ==================== 启动调度器 ==================== */
static inline void OS_StartScheduler(void) {
    // Zephyr 在多线程环境下，调度器在初始化后自动运行，通常不需要显式启动
    // 但某些情况下可能需要调用 k_thread_start，这里留空
    // 或者你可以在这里放一些初始化代码
}

/* ==================== 信号量 ==================== */
static inline OS_SemaphoreHandle_t OS_SemaphoreCreate(uint32_t max_count, uint32_t init_count) {
    if (__sem_idx >= OS_ZEPHYR_MAX_OBJECTS) return NULL;
    struct k_sem *sem = &__sem_pool[__sem_idx++];
    k_sem_init(sem, init_count, max_count);
    return (OS_SemaphoreHandle_t)sem;
}

static inline OS_Status_t OS_SemaphoreTake(OS_SemaphoreHandle_t sem, OS_TickMs_t timeout_ms) {
    k_timeout_t to = (timeout_ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(timeout_ms);
    return (k_sem_take((struct k_sem*)sem, to) == 0) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_SemaphoreGive(OS_SemaphoreHandle_t sem) {
    return (k_sem_give((struct k_sem*)sem) == 0) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_SemaphoreGiveFromISR(OS_SemaphoreHandle_t sem, int *pxHigherPriorityTaskWoken) {
    // Zephyr 在 ISR 中给信号量需要特殊处理，这里简化，直接调用
    // 如有需要可改用 k_sem_give_from_isr，需传入一个 bool 标志
    return (k_sem_give((struct k_sem*)sem) == 0) ? OS_SUCCESS : OS_FAIL;
}

/* ==================== 互斥量 ==================== */
static inline OS_MutexHandle_t OS_MutexCreate(void) {
    if (__mutex_idx >= OS_ZEPHYR_MAX_OBJECTS) return NULL;
    struct k_mutex *mutex = &__mutex_pool[__mutex_idx++];
    k_mutex_init(mutex);
    return (OS_MutexHandle_t)mutex;
}

static inline OS_Status_t OS_MutexTake(OS_MutexHandle_t mutex, OS_TickMs_t timeout_ms) {
    k_timeout_t to = (timeout_ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(timeout_ms);
    return (k_mutex_lock((struct k_mutex*)mutex, to) == 0) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_MutexGive(OS_MutexHandle_t mutex) {
    return (k_mutex_unlock((struct k_mutex*)mutex) == 0) ? OS_SUCCESS : OS_FAIL;
}

/* ==================== 队列（消息队列） ==================== */
static inline OS_QueueHandle_t OS_QueueCreate(uint32_t queue_len, uint32_t item_size) {
    if (__queue_idx >= OS_ZEPHYR_MAX_OBJECTS) return NULL;
    // 注意：Zephyr 的 k_msgq 在创建时需要指定缓冲区和大小
    // 这里简化，使用预设的缓冲区大小和条目数，若实际不满足，建议外部用户调整宏
    struct k_msgq *q = &__queue_pool[__queue_idx];
    k_msgq_init(q, __queue_buffer_pool[__queue_idx], OS_ZEPHYR_MSGQ_SIZE, OS_ZEPHYR_MSGQ_ITEMS);
    __queue_idx++;
    return (OS_QueueHandle_t)q;
}

static inline OS_Status_t OS_QueueSend(OS_QueueHandle_t queue, const void *item, OS_TickMs_t timeout_ms) {
    k_timeout_t to = (timeout_ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(timeout_ms);
    return (k_msgq_put((struct k_msgq*)queue, item, to) == 0) ? OS_SUCCESS : OS_FAIL;
}

static inline OS_Status_t OS_QueueReceive(OS_QueueHandle_t queue, void *buffer, OS_TickMs_t timeout_ms) {
    k_timeout_t to = (timeout_ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(timeout_ms);
    return (k_msgq_get((struct k_msgq*)queue, buffer, to) == 0) ? OS_SUCCESS : OS_FAIL;
}

static inline uint32_t OS_QueueMessagesWaiting(OS_QueueHandle_t queue) {
    return (uint32_t)k_msgq_num_used((struct k_msgq*)queue);
}

/* ==================== 事件组 ==================== */
static inline OS_EventGroupHandle_t OS_EventGroupCreate(void) {
    if (__event_idx >= OS_ZEPHYR_MAX_OBJECTS) return NULL;
    struct k_event *evt = &__event_pool[__event_idx++];
    k_event_init(evt);
    return (OS_EventGroupHandle_t)evt;
}

static inline uint32_t OS_EventGroupWaitBits(OS_EventGroupHandle_t evt, uint32_t bits_to_wait,
                                             int clear_on_exit, int wait_all, OS_TickMs_t timeout_ms) {
    k_timeout_t to = (timeout_ms == OS_WAIT_FOREVER) ? K_FOREVER : K_MSEC(timeout_ms);
    uint32_t ret = k_event_wait((struct k_event*)evt, bits_to_wait, (bool)wait_all, to);
    // Zephyr 的 k_event_wait 返回的是剩余事件位，clear_on_exit 由 wait 内部根据 wait_all 自动处理
    // 若需手动清除，可以调用 k_event_clear
    if (clear_on_exit && !wait_all) {
        // 清除已等待到的位
        k_event_clear((struct k_event*)evt, bits_to_wait & ret);
    }
    return ret;
}

static inline uint32_t OS_EventGroupSetBits(OS_EventGroupHandle_t evt, uint32_t bits) {
    return (uint32_t)k_event_set((struct k_event*)evt, bits);
}

static inline uint32_t OS_EventGroupClearBits(OS_EventGroupHandle_t evt, uint32_t bits) {
    return (uint32_t)k_event_clear((struct k_event*)evt, bits);
}

/* ==================== 临界区（锁调度） ==================== */
// 注意：Zephyr 的 k_sched_lock 类似于 FreeRTOS 的 vTaskSuspendAll，不会关中断，只防止任务切换
static inline void OS_EnterCritical(void) {
    k_sched_lock();
}

static inline void OS_ExitCritical(void) {
    k_sched_unlock();
}

#endif