/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define On_Board_LED_Pin GPIO_PIN_13
#define On_Board_LED_GPIO_Port GPIOC
#define RGB_Red_Pin GPIO_PIN_1
#define RGB_Red_GPIO_Port GPIOA
#define RGB_Green_Pin GPIO_PIN_2
#define RGB_Green_GPIO_Port GPIOA
#define RGB_Blue_Pin GPIO_PIN_3
#define RGB_Blue_GPIO_Port GPIOA
#define Button_Switch_Pin GPIO_PIN_4
#define Button_Switch_GPIO_Port GPIOA
#define Button_Breath_Pin GPIO_PIN_5
#define Button_Breath_GPIO_Port GPIOA
#define Button_Rainbow_Pin GPIO_PIN_6
#define Button_Rainbow_GPIO_Port GPIOA
#define Button_Measure_Pin GPIO_PIN_7
#define Button_Measure_GPIO_Port GPIOA
#define Potentiometer_Pin GPIO_PIN_0
#define Potentiometer_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
