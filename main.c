/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "driver_ssd1306.h"
#include "driver_ssd1306_interface.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t n = 0;
uint32_t r = 0;
uint32_t g = 0;
uint32_t b = 0;
static ssd1306_handle_t oled;

typedef enum {
	state_Off = 0,
	state_On = 1
} state_type;

typedef enum {
	state_Off_already = 0,
	state_Off_just = 1,
} state_Off_type;

typedef enum {
	state_On_already = 0,
	state_On_just = 1,
} state_On_type;

typedef enum {
	mode_Off = 0,
	mode_Breath = 1,
	mode_Rainbow = 2,
	mode_Measure = 3
} mode_type;

typedef enum {
	LED_On = 0,
	LED_Off = 1,
} LED_On_or_Off_type;

typedef enum {
	OLED_On = 0,
	OLED_Off = 1,
} OLED_On_or_Off_type;
state_type state = state_Off;
state_Off_type state_Off_mode = state_Off_already;
state_On_type state_On_mode = state_On_already;
mode_type mode = mode_Off;

static uint32_t triangle (uint32_t phase, uint32_t period, uint32_t max)
{
	uint32_t half = period / 2;
	uint32_t t = phase % period;
	
	if(t < half){
		return (t * max) / half;
	} else {
		return ((period - t) * max) / half;
	}
}

static void LED_lighted (LED_On_or_Off_type LED_On_or_Off)
{
	if (LED_On_or_Off == LED_On){
		HAL_GPIO_WritePin(On_Board_LED_GPIO_Port, On_Board_LED_Pin, GPIO_PIN_RESET);}
	else {
		HAL_GPIO_WritePin(On_Board_LED_GPIO_Port, On_Board_LED_Pin, GPIO_PIN_SET);}
}
static void OLED_lighted (OLED_On_or_Off_type OLED_On_or_Off)
{
	if (OLED_On_or_Off == OLED_On){
		ssd1306_set_display(&oled, SSD1306_DISPLAY_ON);}
	else {
		ssd1306_set_display(&oled, SSD1306_DISPLAY_OFF);}
}
static void RGB_lighted (uint32_t color_r, uint32_t color_g,uint32_t color_b)
{
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_2, color_r);
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3, color_g);
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_4, color_b);
}

static void Boot_animation (void)
{
	static uint32_t n1 = 0;
	while(1){
		if( n1 / 25 == 0 )
			RGB_lighted(500,0,0);
		else if( n1 / 25 == 1 )
			RGB_lighted(500-20*(n1-25),20*(n1-25),0);
		else if( n1 / 25 == 2 )
			RGB_lighted(0, 500, 0);
		else if( n1 / 25 == 3 )
			RGB_lighted(0,500-20*(n1-75),20*(n1-75));
		else if( n1 / 25 == 4 )
			RGB_lighted(0,0,500);
		else
			break;
		HAL_Delay(12);
		n1 = n1 + 1;}
}
static void Button_detector (GPIO_TypeDef *port, uint16_t pin)
{
	if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET)
	{
		HAL_Delay(20);
		if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET)
		{
			while (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET)
			{
				// Wait for Releasing
			}
			if (pin == Button_Switch_Pin)
			{
				if (state == state_Off){
					state = state_On;
					state_On_mode = state_On_just;}
				else {
					state = state_Off;
					state_Off_mode = state_Off_just;}
			}
			else if (pin == Button_Breath_Pin){
				mode = mode_Breath;}
			else if (pin == Button_Rainbow_Pin){
				mode = mode_Rainbow;}
			else if (pin == Button_Measure_Pin){
				mode = mode_Measure;}
		}
	}
}
		

	
static void RGB_Breath (void)
{
	r = triangle (n, 75, 999);
	g = 0;
	b = triangle (n, 75, 999);
	RGB_lighted (r,g,b);
}

static void RGB_Rainbow (void)
{
	r = triangle(n, 50, 999);
	g = triangle(n + 70, 75, 999);
	b = triangle(n + 150, 100, 999);
	RGB_lighted (r,g,b);
}

static uint32_t Potentiometer_Read (void)
{
	static uint32_t adc_val = 0;
	
	HAL_ADC_Start(&hadc1);	
	if (HAL_ADC_PollForConversion(&hadc1, 10) ==  HAL_OK)
		{
		adc_val = HAL_ADC_GetValue(&hadc1);
		}
	HAL_ADC_Stop(&hadc1);
		
	return adc_val;
}
static uint32_t RGB_Measure (void)
{
	uint32_t Brightness = Potentiometer_Read ();
	if (Brightness <= 2013)
	{
		r = 999 - (Brightness * 999 / 2013);
		g = Brightness * 999 / 2013;
		b = 0;
	}
	else
	{
		r = 0;
		g = 999 - ((Brightness - 2013) * 999 / 2013);
		b = (Brightness - 2013) * 999 / 2013;
	}
	RGB_lighted (r,g,b);
	return Brightness;
}
	
int fputc(int ch, FILE *f)
{
    uint8_t temp = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &temp, 1, 100);
    return ch;
}
// Add this function in main.c to send the standard SSD1306 initialization commands
static void ssd1306_configure(ssd1306_handle_t *handle)
{
    // Command sequence: (command, parameter) or single command
    // You can follow the order below, send commands one by one via ssd1306_write_cmd()
    // Or use an array plus a loop to send

    // 1. Turn off display, clear internal state
    ssd1306_write_cmd(handle, (uint8_t[]){0xAE}, 1);

    // 2. Set display clock divide
    ssd1306_write_cmd(handle, (uint8_t[]){0xD5, 0x80}, 2);

    // 3. Set multiplex ratio (64 lines)
    ssd1306_write_cmd(handle, (uint8_t[]){0xA8, 0x3F}, 2);

    // 4. Set display offset
    ssd1306_write_cmd(handle, (uint8_t[]){0xD3, 0x00}, 2);

    // 5. Set display start line
    ssd1306_write_cmd(handle, (uint8_t[]){0x40}, 1);

    // 6. Enable charge pump (internal VCC)
    ssd1306_write_cmd(handle, (uint8_t[]){0x8D, 0x14}, 2);

    // 7. Set memory addressing mode to page
    ssd1306_write_cmd(handle, (uint8_t[]){0x20, 0x02}, 2);

    // 8. Set column address remap (left-right flip)
    ssd1306_write_cmd(handle, (uint8_t[]){0xA1}, 1);

    // 9. Set row scan direction (up-down flip)
    ssd1306_write_cmd(handle, (uint8_t[]){0xC8}, 1);

    // 10. Set COM pins configuration
    ssd1306_write_cmd(handle, (uint8_t[]){0xDA, 0x12}, 2);

    // 11. Set contrast
    ssd1306_write_cmd(handle, (uint8_t[]){0x81, 0xCF}, 2);

    // 12. Set pre-charge period
    ssd1306_write_cmd(handle, (uint8_t[]){0xD9, 0xF1}, 2);

    // 13. Set VCOMH voltage
    ssd1306_write_cmd(handle, (uint8_t[]){0xDB, 0x40}, 2);

    // 14. Set normal display (not full bright)
    ssd1306_write_cmd(handle, (uint8_t[]){0xA4, 0xA6}, 2);

    // 15. Turn on display
    ssd1306_write_cmd(handle, (uint8_t[]){0xAF}, 1);
}
static void oled_init(void)
{
    DRIVER_SSD1306_LINK_INIT(&oled, ssd1306_handle_t);

    DRIVER_SSD1306_LINK_IIC_INIT(&oled, ssd1306_interface_iic_init);
    DRIVER_SSD1306_LINK_IIC_DEINIT(&oled, ssd1306_interface_iic_deinit);
    DRIVER_SSD1306_LINK_IIC_WRITE(&oled, ssd1306_interface_iic_write);
    DRIVER_SSD1306_LINK_DELAY_MS(&oled, ssd1306_interface_delay_ms);
    DRIVER_SSD1306_LINK_DEBUG_PRINT(&oled, ssd1306_interface_debug_print);
    DRIVER_SSD1306_LINK_SPI_INIT(&oled, ssd1306_interface_spi_init);
    DRIVER_SSD1306_LINK_SPI_DEINIT(&oled, ssd1306_interface_spi_deinit);
    DRIVER_SSD1306_LINK_SPI_WRITE_COMMAND(&oled, ssd1306_interface_spi_write_cmd);
    DRIVER_SSD1306_LINK_SPI_COMMAND_DATA_GPIO_INIT(&oled, ssd1306_interface_spi_cmd_data_gpio_init);
    DRIVER_SSD1306_LINK_SPI_COMMAND_DATA_GPIO_DEINIT(&oled, ssd1306_interface_spi_cmd_data_gpio_deinit);
    DRIVER_SSD1306_LINK_SPI_COMMAND_DATA_GPIO_WRITE(&oled, ssd1306_interface_spi_cmd_data_gpio_write);
    DRIVER_SSD1306_LINK_RESET_GPIO_INIT(&oled, ssd1306_interface_reset_gpio_init);
    DRIVER_SSD1306_LINK_RESET_GPIO_DEINIT(&oled, ssd1306_interface_reset_gpio_deinit);
    DRIVER_SSD1306_LINK_RESET_GPIO_WRITE(&oled, ssd1306_interface_reset_gpio_write);

    ssd1306_set_interface(&oled, SSD1306_INTERFACE_IIC);
    ssd1306_set_addr_pin(&oled, SSD1306_ADDR_SA0_0);

    ssd1306_init(&oled);
	ssd1306_configure(&oled);
	ssd1306_gram_update(&oled);
}
// Define 16x16 Chinese character font array, column-row mode, high bit first
const uint8_t chinese_font[6][32] = {
    // Nan
    {0x04,0xe4,0x24,0x24,0x64,0xa4,0x24,0x3f,0x24,0xa4,0x64,0x24,0x24,0xe4,0x04,0x00,
     0x00,0xff,0x00,0x08,0x09,0x09,0x09,0x7f,0x09,0x09,0x09,0x48,0x80,0x7f,0x00,0x00},
    // Jing
    {0x04,0x04,0x04,0xe4,0x24,0x24,0x25,0x26,0x24,0x24,0x24,0xe4,0x04,0x04,0x04,0x00,
     0x00,0x40,0x20,0x1b,0x02,0x42,0x82,0x7e,0x02,0x02,0x02,0x0b,0x10,0x60,0x00,0x00},
    // You
    {0x00,0xf8,0x08,0x08,0xff,0x08,0x08,0xf8,0x00,0xfe,0x02,0x22,0xda,0x06,0x00,0x00,
     0x00,0x7f,0x21,0x21,0x3f,0x21,0x21,0x7f,0x00,0xff,0x08,0x10,0x08,0x07,0x00,0x00},
    // Dian
    {0x00,0x00,0xf8,0x88,0x88,0x88,0x88,0xff,0x88,0x88,0x88,0x88,0xf8,0x00,0x00,0x00,
     0x00,0x00,0x1f,0x08,0x08,0x08,0x08,0x7f,0x88,0x88,0x88,0x88,0x9f,0x80,0xf0,0x00},
    // Da
    {0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xff,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,
     0x80,0x80,0x40,0x20,0x10,0x0c,0x03,0x00,0x03,0x0c,0x10,0x20,0x40,0x80,0x80,0x00},
    // Xue
    {0x40,0x30,0x11,0x96,0x90,0x90,0x91,0x96,0x90,0x90,0x98,0x14,0x13,0x50,0x30,0x00,
     0x04,0x04,0x04,0x04,0x04,0x44,0x84,0x7e,0x06,0x05,0x04,0x04,0x04,0x04,0x04,0x00}
};

// Function to draw a 16x16 Chinese character at (x, y)
void draw_chinese_16x16(uint8_t x, uint8_t y, const uint8_t *font)
{
    if ((x + 15 > 127) || (y + 15 > 63)) return;   // boundary check

    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t upper = font[i];        // upper half (y ~ y+7)
        uint8_t lower = font[16 + i];   // lower half (y+8 ~ y+15)

        for (uint8_t bit = 0; bit < 8; bit++)
        {
            // Note: using (1 << bit) so low bit corresponds to top
            if (upper & (1 << bit))
                ssd1306_gram_write_point(&oled, x + i, y + bit, 1);
            if (lower & (1 << bit))
                ssd1306_gram_write_point(&oled, x + i, y + 8 + bit, 1);
        }
    }
}

// Function to display all content
void display_oled_content(void)
{
    ssd1306_clear(&oled);
    
    // Display the English line
    ssd1306_gram_write_string(&oled, 0, 0, "Hello NJUPT!", 12, 1, SSD1306_FONT_16);
    
    // Display Chinese characters (y=16)
    draw_chinese_16x16(0, 16, chinese_font[0]);
    draw_chinese_16x16(16, 16, chinese_font[1]);
    draw_chinese_16x16(32, 16, chinese_font[2]);
    draw_chinese_16x16(48, 16, chinese_font[3]);
    draw_chinese_16x16(64, 16, chinese_font[4]);
    draw_chinese_16x16(80, 16, chinese_font[5]);
    
    // Update the display
    ssd1306_gram_update(&oled);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  //PA1
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);  //PA2
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);  //PA3
	oled_init();
	ssd1306_set_display(&oled, SSD1306_DISPLAY_OFF);
	display_oled_content();
	printf("Hello, I'm STM32!\r\n");
	Boot_animation ();
	RGB_lighted (0,0,0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Button_detector (Button_Switch_GPIO_Port, Button_Switch_Pin);	
	  Button_detector (Button_Breath_GPIO_Port, Button_Breath_Pin);
	  Button_detector (Button_Rainbow_GPIO_Port, Button_Rainbow_Pin);
	  Button_detector (Button_Measure_GPIO_Port,Button_Measure_Pin);
  
	  if (state == state_Off)
	  {
		  if (state_Off_mode == state_Off_just)
		  {
			  RGB_lighted (0,0,0);
			  LED_lighted (LED_Off);
			  OLED_lighted (OLED_Off);
			  mode = mode_Off;
			  state_Off_mode = state_Off_already;
		  }
		  else
		  {
			  // It has already been turned off; no need to close it again.
		  }
	  }
	  else
	  {
		  if (state_On_mode == state_On_just)
		  {
			  LED_lighted (LED_On);
			  OLED_lighted (OLED_On);
			  state_On_mode = state_On_already;
		  }
		  else
		  {
			  //They have already been turned off; no need to close them again.
		  }
		  if (mode == mode_Breath){
			  RGB_Breath();}
		  else if (mode == mode_Rainbow){
			  RGB_Rainbow();}
		  else if (mode == mode_Measure){
			  uint32_t brightness = RGB_Measure();
			  if (n % 25 == 0){				  
				  uint32_t volt_100 = brightness * 330U / 4095U;
				  printf("ADC = %u, Voltage = %u.%02u V\r\n", 
						 brightness, volt_100 / 100, volt_100 %100);}}
		  else {
			  RGB_lighted(0,0,0);}
	  }
	  n = (n + 1) % 10000;
	  HAL_Delay(20);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(On_Board_LED_GPIO_Port, On_Board_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : On_Board_LED_Pin */
  GPIO_InitStruct.Pin = On_Board_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(On_Board_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Button_Switch_Pin Button_Breath_Pin Button_Rainbow_Pin Button_Measure_Pin */
  GPIO_InitStruct.Pin = Button_Switch_Pin|Button_Breath_Pin|Button_Rainbow_Pin|Button_Measure_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
