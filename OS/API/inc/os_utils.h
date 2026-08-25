// os_utils.h
#ifndef OS_UTILS_H
#define OS_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "os_pro_config.h"   // 你的项目配置文件（如果有）

/* =============================================================
   第一部分：统一类型定义（业务代码永远只看到这些）
   ============================================================= */

// 统一句柄（业务代码只持有这些 void*）
typedef void* OS_TaskHandle_t;
typedef void* OS_SemaphoreHandle_t;
typedef void* OS_MutexHandle_t;
typedef void* OS_QueueHandle_t;
typedef void* OS_EventGroupHandle_t;
// 统一的任务状态枚举
typedef enum {
    OS_TASK_STATE_RUNNING,   // 正在运行
    OS_TASK_STATE_READY,     // 就绪
    OS_TASK_STATE_BLOCKED,   // 阻塞（等待信号量、队列等）
    OS_TASK_STATE_SUSPENDED, // 挂起（vTaskSuspend / k_thread_suspend）
    OS_TASK_STATE_DELETED,   // 已删除
    OS_TASK_STATE_UNKNOWN    // 未知或不存在
} OS_TaskState_t;


// 统一基础类型
typedef uint32_t    OS_Priority_t;
typedef uint32_t    OS_StackDepth_t;   // 单位：FreeRTOS 为“字”，Zephyr 为“字节”（后端内部转换）
typedef int32_t     OS_Status_t;
typedef uint32_t    OS_TickMs_t;       // 统一用毫秒

// 统一返回值和超时
#define OS_SUCCESS        (0)
#define OS_FAIL           (-1)
#define OS_WAIT_FOREVER   (0xFFFFFFFFUL)
#define OS_NO_WAIT        (0)

// 统一任务入口函数类型
typedef void (*OS_TaskFunction_t)(void *param);

// 统一任务配置结构体（业务填这个结构体传入）
typedef struct {
    OS_TaskFunction_t func;        // 任务函数
    void             *param;       // 参数
    const char       *name;        // 任务名
    OS_StackDepth_t   stack_depth; // 栈大小
    OS_Priority_t     priority;    // 优先级（数值越高优先级越高，后端内部映射）
    OS_TaskHandle_t  *task_handle; // 传出句柄（可选，传 NULL 则忽略）
    uint8_t           is_suspended;
} OS_TaskConfig_t;

// 信号量配置（保留你原来的设计，可用于调试）
typedef struct {
    uint8_t type;   // 0: binary, 1: counting
    uint8_t id;     // 调试用 ID
    void*   handle; // 内部使用（业务无需关心）
} OS_Semaphore_t;

/* =============================================================
   第二部分：断言宏（保留你的原样）
   ============================================================= */
#define OS_UTILS_ASSERT_INT(x, info, err) \
    do { if(!(x)) { printf("[OS_UTILS_ASSERT]:%s. Occured at %s:%d, error code:%ld\r\n", info, __FILE__, __LINE__, (long)(err)); while(1){} } } while(0)

#define OS_UTILS_ASSERT_STR(x, info, err) \
    do { if(!(x)) { printf("[OS_UTILS_ASSERT]:%s. Occured at %s, %d, error info:%s\r\n", info, __FILE__, __LINE__, (err)); while(1){} } } while(0)

/* =============================================================
   第三部分：统一 API 声明（业务调用的函数原型）
   ============================================================= */

/* --- 任务 --- */
static inline OS_Status_t OS_CreateTask(OS_TaskConfig_t *cfg);
static inline OS_Status_t OS_CreateTasks(OS_TaskConfig_t cfgs[], int task_num);
static inline void OS_TaskDelay(OS_TickMs_t ms);
static inline OS_TickMs_t OS_GetTickMs(void);
static inline void OS_TaskDelete(OS_TaskHandle_t task);   // 新增
static inline void OS_StartScheduler(void);               // 新增
static inline void OS_TaskSuspend(OS_TaskHandle_t task);
static inline void OS_TaskResume(OS_TaskHandle_t task);
// 统一查询接口声明（放在 os_utils.h 的声明区）
static inline OS_TaskState_t OS_GetTaskState(OS_TaskHandle_t task);

/* --- 信号量 --- */
static inline OS_SemaphoreHandle_t OS_SemaphoreCreate(uint32_t max_count, uint32_t init_count);
static inline OS_Status_t OS_SemaphoreTake(OS_SemaphoreHandle_t sem, OS_TickMs_t timeout_ms);
static inline OS_Status_t OS_SemaphoreGive(OS_SemaphoreHandle_t sem);
static inline OS_Status_t OS_SemaphoreGiveFromISR(OS_SemaphoreHandle_t sem, int *pxHigherPriorityTaskWoken);

/* --- 互斥量 --- */
static inline OS_MutexHandle_t OS_MutexCreate(void);
static inline OS_Status_t OS_MutexTake(OS_MutexHandle_t mutex, OS_TickMs_t timeout_ms);
static inline OS_Status_t OS_MutexGive(OS_MutexHandle_t mutex);

/* --- 队列（固定长度消息队列） --- */
static inline OS_QueueHandle_t OS_QueueCreate(uint32_t queue_len, uint32_t item_size);
static inline OS_Status_t OS_QueueSend(OS_QueueHandle_t queue, const void *item, OS_TickMs_t timeout_ms);
static inline OS_Status_t OS_QueueReceive(OS_QueueHandle_t queue, void *buffer, OS_TickMs_t timeout_ms);
static inline uint32_t OS_QueueMessagesWaiting(OS_QueueHandle_t queue);

/* --- 事件组 --- */
static inline OS_EventGroupHandle_t OS_EventGroupCreate(void);
static inline uint32_t OS_EventGroupWaitBits(OS_EventGroupHandle_t evt, uint32_t bits_to_wait,
                                             int clear_on_exit, int wait_all, OS_TickMs_t timeout_ms);
static inline uint32_t OS_EventGroupSetBits(OS_EventGroupHandle_t evt, uint32_t bits);
static inline uint32_t OS_EventGroupClearBits(OS_EventGroupHandle_t evt, uint32_t bits);

/* --- 临界区（关/开调度） --- */
static inline void OS_EnterCritical(void);
static inline void OS_ExitCritical(void);

#ifndef OS_MS_TO_TICKS
    #define OS_MS_TO_TICKS(ms)  (ms)   // 裸机下 ms 就是 ms
#endif

/* =============================================================
   第四部分：根据宏开关，包含对应的后端实现（编译期绑定）
   ============================================================= */
#if defined(OS_BACKEND_FREERTOS) // 建议加在Runtime的 compile command 里面
    #include "os_backend_freertos_inline.h"
#elif defined(OS_BACKEND_ZEPHYR)
    #include "os_backend_zephyr_inline.h"
#elif defined(OS_BACK_END_RT_THREAD)
    #include "os_backend_rt_thread_inline.h"
#else
    #error "Please define OS_BACKEND_FREERTOS or OS_BACKEND_ZEPHYR in your build system (e.g. -DOS_BACKEND_FREERTOS)"
#endif

/* =============================================================
   第五部分：OS相关API
   ============================================================= */
void OS_TaskRegister(OS_TaskConfig_t* cfg);
void OS_TasksRegister(OS_TaskConfig_t cfg[],int num);
uint8_t OS_GetTaskCount(void);
void OS_GetTaskList(OS_TaskConfig_t** list);




#endif // OS_UTILS_H