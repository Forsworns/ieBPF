# ieBPF Cortex-M4

这部分就是在 STM32CubeIDE 中的工程代码，已经配置过相对的 include 路径，应该是即开即用的，只需要在 STM32CubeIDE 中导入就行了。

在 `./CM4/Core/Inc/stm32mp1xx_hal_conf.h` 中定义了 `HAL_UART_MODULE_ENABLED` 宏，当需要打印串口调试信息时，需要开启它；当板子未连接网线时，则要使用串口连接到 A7 端的 Linux 系统，需要注释掉该宏，避免调试输出和 RNDIS 网卡对串口的争用。

如果是在 Debug 选项中选择生产模式，即通过 Linux 内核加载 M4 上的程序固件（或是板子是在工程模式，通过串口走 JTAG/SWD 烧写程序），那么程序和 `RemoteProc` 下的负责烧写这个程序固件的 `fw_cortex_m4.sh` 脚本，都会被发送到项目的 property 下的 `Remote Settings` 中设置的目录下。
