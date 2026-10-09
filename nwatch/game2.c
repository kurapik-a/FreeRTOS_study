/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "timers.h"                     // 软件定时器（原由 cmsis_os.h 间接引入）

#include "draw.h"
#include "resources.h"

#include "driver_lcd.h"
#include "driver_ir_receiver.h"
#include "driver_rotary_encoder.h"
#include "driver_mpu6050.h"
#include "driver_timer.h"

#define NOINVERT	false
#define INVERT		true


#define CAR_COUNT	3
#define CAR_WIDTH	12
#define CAR_LENGTH	15
#define ROAD_SPEED	6

static SemaphoreHandle_t g_xSemTicks; 
static uint32_t g_xres, g_yres, g_bpp;
static uint8_t *g_framebuffer;
static EventGroupHandle_t g_xEventCar;

static TaskHandle_t g_TaskHandleCar2;
static TaskHandle_t g_TaskHandleCar3;

struct car {
	int x;
	int y;
	int control_key;
};

struct car g_cars[3] = {
	{0, 0, IR_KEY_1},
	{0, 17, IR_KEY_2},
	{0, 34, IR_KEY_3},
};

static const byte carImg[] ={
	0x40,0xF8,0xEC,0x2C,0x2C,0x38,0xF0,0x10,0xD0,0x30,0xE8,0x4C,0x4C,0x9C,0xF0,
	0x02,0x1F,0x37,0x34,0x34,0x1C,0x0F,0x08,0x0B,0x0C,0x17,0x32,0x32,0x39,0x0F,
};

static const byte clearImg[30] ={0};

static const byte roadMarking[] ={
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
};

#if 0
void car_test(void)
{
	g_framebuffer = LCD_GetFrameBuffer(&g_xres, &g_yres, &g_bpp);
	draw_init();
	draw_end();
	
	draw_bitmap(0, 0, carImg, 15, 16, NOINVERT, 0);
	draw_flushArea(0, 0, 15, 16);
	
	draw_bitmap(0, 16, roadMarking, 8, 1, NOINVERT, 0);
	draw_flushArea(0, 16, 8, 1);

	while (1);
}
#endif

static void ShowCar(struct car *pcar)
{
	draw_bitmap(pcar->x, pcar->y, carImg, 15, 16, NOINVERT, 0);
	draw_flushArea(pcar->x, pcar->y, 15, 16);
}

static void HideCar(struct car *pcar)
{
	draw_bitmap(pcar->x, pcar->y, clearImg, 15, 16, NOINVERT, 0);
	draw_flushArea(pcar->x, pcar->y, 15, 16);
}

static void Car1Task(void *params)
{
	struct car *pcar = params;
	struct ir_data idata;
	
	/* 创建自己的队列 */
	QueueHandle_t xQueueIR = xQueueCreate(10, sizeof(struct ir_data));
	
	/* 注册队列 */
	RegisterQueueHandle(xQueueIR);

	/* 显示汽车 */
	ShowCar(pcar);
	
	/* 获得信号量 */
	//xSemaphoreTake(g_xSemTicks, portMAX_DELAY);
	
	while (1)
	{
		/* 读取按键值:读队列 */
		//xQueueReceive(xQueueIR, &idata, portMAX_DELAY);
		
		/* 控制汽车往右移动 */
		//if (idata.val == pcar->control_key)
		{
			if (pcar->x < g_xres - CAR_LENGTH)
			{
				/* 隐藏汽车 */
				HideCar(pcar); 
				
				/* 调整位置 */
				pcar->x += 1;
				if (pcar->x > g_xres - CAR_LENGTH)
				{
					pcar->x = g_xres - CAR_LENGTH;
				}
				
				/* 重新显示汽车 */
				ShowCar(pcar);
				
				vTaskDelay(50);
				
				if (pcar->x == g_xres - CAR_LENGTH)
				{
					/* 释放信号量 */
					//xSemaphoreGive(g_xSemTicks);
					
					/* 设置事件组: bit0 */
					//xEventGroupSetBits(g_xEventCar, (1<<0));
					
					/* 发出任务通知给car2,car3 */
					xTaskNotifyGive(g_TaskHandleCar2);
					
					xTaskNotify(g_TaskHandleCar3, 100, eSetValueWithOverwrite);
					
					vTaskDelete(NULL);
				}
			}
		}
	}
}

static void Car2Task(void *params)
{
	struct car *pcar = params;
	struct ir_data idata;
		
	/* 创建自己的队列 */
	QueueHandle_t xQueueIR = xQueueCreate(10, sizeof(struct ir_data));
	
	/* 注册队列 */
	RegisterQueueHandle(xQueueIR);

	/* 显示汽车 */
	ShowCar(pcar);
	
	/* 获得信号量 */
	//xSemaphoreTake(g_xSemTicks, portMAX_DELAY);
	
	/* 等待事件:bit0 */
	//xEventGroupWaitBits(g_xEventCar, (1<<0), pdTRUE, pdFALSE, portMAX_DELAY);
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	
	while (1)
	{
		/* 读取按键值:读队列 */
		//xQueueReceive(xQueueIR, &idata, portMAX_DELAY);
		
		/* 控制汽车往右移动 */
		//if (idata.val == pcar->control_key)
		{
			if (pcar->x < g_xres - CAR_LENGTH)
			{
				/* 隐藏汽车 */
				HideCar(pcar);
				
				/* 调整位置 */
				pcar->x += 1;
				if (pcar->x > g_xres - CAR_LENGTH)
				{
					pcar->x = g_xres - CAR_LENGTH;
				}
				
				/* 重新显示汽车 */
				ShowCar(pcar);
				
				vTaskDelay(100);
				//mdelay(50);
				
				if (pcar->x == g_xres - CAR_LENGTH)
				{
					/* 释放信号量 */
					//xSemaphoreGive(g_xSemTicks);

					/* 设置事件组: bit1 */
					//xEventGroupSetBits(g_xEventCar, (1<<1));
					
					vTaskDelete(NULL);
				}
			}
		}
	}
}


static void Car3Task(void *params)
{
	struct car *pcar = params;
	struct ir_data idata;
	uint32_t val;
	
	
	/* 创建自己的队列 */
	QueueHandle_t xQueueIR = xQueueCreate(10, sizeof(struct ir_data));
	
	/* 注册队列 */
	RegisterQueueHandle(xQueueIR);

	/* 显示汽车 */
	ShowCar(pcar);
	
	/* 获得信号量 */
	//xSemaphoreTake(g_xSemTicks, portMAX_DELAY);

	/* 等待事件:bit0 and bit1 */
	//xEventGroupWaitBits(g_xEventCar, (1<<0)|(1<<1), pdTRUE, pdTRUE, portMAX_DELAY);
	do 
	{
		xTaskNotifyWait(~0, ~0, &val, portMAX_DELAY);
	} while (val != 100);
		
	
	while (1)
	{
		/* 读取按键值:读队列 */
		//xQueueReceive(xQueueIR, &idata, portMAX_DELAY);
		
		/* 控制汽车往右移动 */
		//if (idata.val == pcar->control_key)
		{
			if (pcar->x < g_xres - CAR_LENGTH)
			{
				/* 隐藏汽车 */
				HideCar(pcar);
				
				/* 调整位置 */
				pcar->x += 1;
				if (pcar->x > g_xres - CAR_LENGTH)
				{
					pcar->x = g_xres - CAR_LENGTH;
				}
				
				/* 重新显示汽车 */
				ShowCar(pcar);
				
				//vTaskDelay(50);
				mdelay(50);
				
				if (pcar->x == g_xres - CAR_LENGTH)
				{
					/* 释放信号量 */
					//xSemaphoreGive(g_xSemTicks);
					vTaskDelete(NULL);
				}
			}
		}
	}
}


void car_game(void)
{
	int x;
	int i, j;
	g_framebuffer = LCD_GetFrameBuffer(&g_xres, &g_yres, &g_bpp);
	draw_init();
	draw_end();
	
	//g_xSemTicks = xSemaphoreCreateCounting(1, 1);
	//g_xSemTicks = xSemaphoreCreateMutex();
	g_xEventCar = xEventGroupCreate();

	/* 画出路标 */
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 8; j++)
		{
			draw_bitmap(16*j, 16+17*i, roadMarking, 8, 1, NOINVERT, 0);
			draw_flushArea(16*j, 16+17*i, 8, 1);
		}
	}
	
	/* 创建3个汽车任务 */
#if 0	
	for (i = 0; i < 3; i++)
	{
		draw_bitmap(g_cars[i].x, g_cars[i].y, carImg, 15, 16, NOINVERT, 0);
		draw_flushArea(g_cars[i].x, g_cars[i].y, 15, 16);
	}
#endif
    xTaskCreate(Car1Task, "car1", 128, &g_cars[0], 24, NULL);
    xTaskCreate(Car2Task, "car2", 128, &g_cars[1], 24+2, &g_TaskHandleCar2);
    xTaskCreate(Car3Task, "car3", 128, &g_cars[2], 24+2, &g_TaskHandleCar3);	
}
