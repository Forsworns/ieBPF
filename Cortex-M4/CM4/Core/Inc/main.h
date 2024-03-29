/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* board */
#include "stm32mp1xx_hal.h"
#include "gpio.h"
#include "usart.h"
#include "ipcc.h"
/* openamp */
#include "openamp.h"
#include "virt_uart.h"
/* os */
// #include "cmsis_os.h" // use cmsis api
#include "tos_k.h" // use t_os api
#include "tos_hal.h"
/* 
#include "ff.h"
#include "tos_vfs.h"
#include "tos_fatfs_drv.h"
#include "tos_fatfs_vfs.h"
#include "tos_elfloader.h"
*/
/* bpf */
#include "ebpf.h"
#include "ubpf.h"
#include "ubpf_int.h"
#include "tasks.h"
#include <stdio.h>

  void Error_Handler(void);
  void SystemClock_Config(void);

  void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
