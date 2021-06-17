# ieBPF Cortex-M4

这部分就是在 STM32CubeIDE 中的工程代码，已经配置过相对的 include 路径，应该是即开即用的，只需要在 STM32CubeIDE 中导入就行了。

在 `./CM4/Core/Inc/stm32mp1xx_hal_conf.h` 中定义了 `HAL_UART_MODULE_ENABLED` 宏，当需要打印串口调试信息时，需要开启它；当板子未连接网线时，则要使用串口连接到 A7 端的 Linux 系统，需要注释掉该宏，避免调试输出和 RNDIS 网卡对串口的争用。