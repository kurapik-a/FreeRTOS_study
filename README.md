# STM32F407 标准外设库工程模板

目录与 F1 模板保持一致：

- Start：启动文件与系统时钟文件
- Library：STM32F4 标准外设库
- System：放置通用软件模块
- Hardware：放置外设驱动模块
- User：放置 main、应用逻辑和中断文件

当前工程不包含任何 LED、按键、串口、PWM 或其他外设功能。
在 Keil 中打开 `STM32F407_Template.uvprojx` 后即可直接开始开发。

---

## 移植说明：FreeRTOS 游戏手表（已并入本工程）

本工程已并入百问网 HAL 版 FreeRTOS 游戏手表项目（原工程 `34_improve_mpu6050/freertos_template`，CubeMX+HAL+CMSIS-OS v2）的标准库移植版。改动要点：

### 目录
- `FreeRTOS/`：FreeRTOS 内核 V10.3.1（tasks/queue/list/timers/event_groups/stream_buffer + CM4F 移植层 port.c + heap_4.c），从原工程原样复制
- `Hardware/`：BSP 板级层（标准库实现）：bsp_gpio / bsp_i2c / bsp_spi / bsp_usart / bsp_tim / bsp_adc / bsp_exti
- `Drivers/`：19 个外设驱动 + 字库，已全部去除 HAL 依赖（低层访问改为标准库/寄存器，算法逻辑与原版一致）
- `nwatch/`：游戏层（打砖块 game1、三车接力 game2、图形库 draw、音效 beep），仅去掉了 cmsis_os.h 依赖
- `User/`：main.c（外设初始化 + 启动调度器）、app_freertos.c（任务创建，原 freertos.c）、FreeRTOSConfig.h、music.c

### 与 HAL 版的主要差异
1. 时钟：HSE 25MHz -> 168MHz（原 HAL 版用内部 HSI 16MHz，主频同为 168MHz）
2. 去掉 CMSIS-OS v2：osPriorityNormal 以数值 24 代替；defaultTask（启动即自删）不再创建
3. SysTick/PendSV/SVC 由 FreeRTOS 移植层接管（FreeRTOSConfig.h 中的映射宏）
4. HAL 毫秒时基（TIM4 中断）不再需要：ms 取自 FreeRTOS tick，TIM4 配置为 1MHz 自由计数提供 µs 级时间戳
5. USART1 的 DMA 未搬运（原工程驱动实际均为轮询收发）
6. C 编译选项追加了 `--c99`（game1.c 的 draw_string 使用变长数组）

### 引脚映射（与 HAL 版一致）
- I2C1（OLED 0x78 + MPU6050 0xD0）：PB8/PB9；I2C2 预留：PB10/PB11
- SPI1（W25Q64，CS=PC1）：PB3/PB4/PB5
- USART1：PA9/PA10，115200
- 红外接收：PA6（EXTI 双边沿）；旋转编码器：PF8（EXTI 上升沿）/PF7/PF6
- MPU6050 INT：PC3（EXTI 上升沿）；DHT11/DS18B20 单总线：PC2（开漏）
- 无源蜂鸣器：PA7（TIM3_CH2）；彩灯：PA5/PA1/PA2（TIM2）；LED：PF9/PF10
- SR04：PE2(Trig)/PE4(Echo)；红外避障：PC5；光敏 ADC：PA3（ADC1_CH3）

### 使用提示
- 在 Keil 中打开工程直接 Rebuild 即可；所有 EXTI/驱动中断优先级为 5（= configMAX_SYSCALL_INTERRUPT_PRIORITY）
- 空闲钩子会经串口打印各任务 CPU 占用率（vTaskGetRunTimeStats），无需外设操作
- driver_ir_sender（TIM9_CH2=PA3）与光敏 ADC（PA3）共用引脚，不可同时使用（与原版一致）
