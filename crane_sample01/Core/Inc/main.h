/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EN_DRV_Pin GPIO_PIN_2
#define EN_DRV_GPIO_Port GPIOE
#define outDown_Pin GPIO_PIN_3
#define outDown_GPIO_Port GPIOE
#define LCD_DISP_EN_Pin GPIO_PIN_4
#define LCD_DISP_EN_GPIO_Port GPIOE
#define TP_IRQ_Pin GPIO_PIN_5
#define TP_IRQ_GPIO_Port GPIOE
#define TP_IRQ_EXTI_IRQn EXTI5_IRQn
#define LCD_BL_CTRL_Pin GPIO_PIN_6
#define LCD_BL_CTRL_GPIO_Port GPIOE
#define USER_BUTTON_Pin GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define outUp_Pin GPIO_PIN_2
#define outUp_GPIO_Port GPIOF
#define VSYNC_FREQ_Pin GPIO_PIN_0
#define VSYNC_FREQ_GPIO_Port GPIOC
#define RENDER_TIME_Pin GPIO_PIN_10
#define RENDER_TIME_GPIO_Port GPIOC
#define FRAME_RATE_Pin GPIO_PIN_11
#define FRAME_RATE_GPIO_Port GPIOC
#define MCU_ACTIVE_Pin GPIO_PIN_12
#define MCU_ACTIVE_GPIO_Port GPIOC
#define USER_LD2_RED_Pin GPIO_PIN_2
#define USER_LD2_RED_GPIO_Port GPIOD
#define USER_LD3_GREEN_Pin GPIO_PIN_4
#define USER_LD3_GREEN_GPIO_Port GPIOD
#define DIR_Pin GPIO_PIN_5
#define DIR_GPIO_Port GPIOD
#define STEP_Pin GPIO_PIN_7
#define STEP_GPIO_Port GPIOD
#define outBack_Pin GPIO_PIN_9
#define outBack_GPIO_Port GPIOG
#define outFront_Pin GPIO_PIN_10
#define outFront_GPIO_Port GPIOG
#define outRight_Pin GPIO_PIN_12
#define outRight_GPIO_Port GPIOG
#define outLeft_Pin GPIO_PIN_15
#define outLeft_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
