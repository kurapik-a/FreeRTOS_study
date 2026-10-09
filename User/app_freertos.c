/*
 * app_freertos.c - 任务创建与内核对象（从 HAL 工程 Core/Src/freertos.c 移植）
 *
 * 与原版的差异：
 *   1. 去掉 CMSIS-OS v2（defaultTask 的 osThreadNew 也一并去掉——
 *      原版 StartDefaultTask 第一行就 vTaskDelete(NULL) 自删，没有实际作用）
 *   2. osPriorityNormal 用数值 24 代替（CMSIS-OS v2 中 osPriorityNormal==24）
 *   3. 补上 vApplicationGetIdleTaskMemory / vApplicationGetTimerTaskMemory
 *      （原工程由 cmsis_os2.c 的弱定义提供）
 */
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"
#include "timers.h"

#include "driver_led.h"
#include "driver_lcd.h"
#include "driver_mpu6050.h"
#include "driver_timer.h"
#include "driver_dht11.h"
#include "driver_ir_receiver.h"
#include "driver_rotary_encoder.h"
#include "bsp_tim.h"

/* 静态创建的 LightTask（栈 + TCB） */
static StackType_t g_pucStackOfLightTask[128];
static StaticTask_t g_TCBofLightTask;

void game1_task(void *params);

/* I2C 总线互斥量：OLED 刷新与 MPU6050 读取共用 I2C1 */
static SemaphoreHandle_t g_xI2CMutex;
static char pcWriteBuffer[200];

void GetI2C(void)
{
    xSemaphoreTake(g_xI2CMutex, portMAX_DELAY);
}

void PutI2C(void)
{
    xSemaphoreGive(g_xI2CMutex);
}

/* 空闲钩子：打印各任务 CPU 占用率（调试用） */
void vApplicationIdleHook(void)
{
    int i;
    vTaskGetRunTimeStats(pcWriteBuffer);
    for (i = 0; i < 16; i++)
        printf("-");
    printf("\n\r");
    printf("%s\n\r", pcWriteBuffer);
}

/* configSUPPORT_STATIC_ALLOCATION=1 时必须提供：空闲任务内存 */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

/* configUSE_TIMERS=1 时必须提供：定时器服务任务内存 */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}

void MX_FREERTOS_Init(void)
{
    g_xI2CMutex = xSemaphoreCreateMutex();

    LCD_Init();
    LCD_Clear();

    MPU6050_Init();
    IRReceiver_Init();
    RotaryEncoder_Init();
    LCD_PrintString(0, 0, "Starting");

    /* 创建游戏任务（原 osPriorityNormal == 24） */
    xTaskCreate(game1_task, "GameTask", 128, NULL, 24, NULL);

    extern void PlayMusic(void *params);
    extern void car_game(void);
    //car_game();                                            /* 三车接力演示，默认不启用 */
    //xTaskCreate(PlayMusic, "MusicTask", 128, NULL, 25, NULL);

    /* 静态创建 LED 任务（栈/TCB 为静态内存，演示 xTaskCreateStatic） */
    xTaskCreateStatic(Led_Test, "LightTask", 128, NULL, 24,
                      g_pucStackOfLightTask, &g_TCBofLightTask);
}
