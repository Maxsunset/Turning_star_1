/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_ssd1306_interface.c
 * @brief     driver ssd1306 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-03-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/03/30  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/10  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ssd1306_interface.h"
#include "main.h"
#include <stdarg.h>
#include <stdio.h>

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
// Pin definitions: PB10=SCL, PB11=SDA
#define OLED_SCL_GPIO   GPIOB
#define OLED_SCL_PIN    GPIO_PIN_10
#define OLED_SDA_GPIO   GPIOB
#define OLED_SDA_PIN    GPIO_PIN_11

// Simple microsecond delay
static void soft_i2c_delay_us(uint32_t us)
{
    uint32_t i;
    for(i = 0; i < us * 4; i++) { __NOP(); }
}

// Initialize pins (configured as open-drain output + internal pull-up)
uint8_t ssd1306_interface_iic_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = OLED_SCL_PIN | OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_SET);
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t ssd1306_interface_iic_deinit(void)
{
    return 0;
}

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
// Start signal
static void soft_i2c_start(void)
{
    HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_SET);
    soft_i2c_delay_us(2);
    HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_RESET);
    soft_i2c_delay_us(2);
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_RESET);
    soft_i2c_delay_us(2);
}

// Stop signal
static void soft_i2c_stop(void)
{
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_RESET);
    soft_i2c_delay_us(2);
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_SET);
    soft_i2c_delay_us(2);
    HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_SET);
    soft_i2c_delay_us(2);
}

// Send one byte
static uint8_t soft_i2c_send_byte(uint8_t dat)
{
    uint8_t i;
    uint8_t ack = 0;
    
    // Send 8 data bits MSB first
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x80)
            HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(OLED_SDA_GPIO, OLED_SDA_PIN, GPIO_PIN_RESET);
        
        soft_i2c_delay_us(1);
        HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_SET);
        soft_i2c_delay_us(2);
        HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_RESET);
        soft_i2c_delay_us(1);
        dat <<= 1;
    }
    
    // --- ACK detection ---
    // Release SDA line (set as input with internal pull-up)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(OLED_SDA_GPIO, &GPIO_InitStruct);
    
    soft_i2c_delay_us(2);
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_SET);  // SCL high, generate clock pulse
    soft_i2c_delay_us(2);
    
    // Read SDA: low means slave sent ACK, high means NACK
    if (HAL_GPIO_ReadPin(OLED_SDA_GPIO, OLED_SDA_PIN) == GPIO_PIN_SET)
    {
        ack = 1;   // No ACK received
    }
    
    HAL_GPIO_WritePin(OLED_SCL_GPIO, OLED_SCL_PIN, GPIO_PIN_RESET); // SCL low, end clock pulse
    soft_i2c_delay_us(2);
    
    // Restore SDA as open-drain output for further data transmission
    GPIO_InitStruct.Pin = OLED_SDA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OLED_SDA_GPIO, &GPIO_InitStruct);
    
    return ack;
}

// Replace the original hardware I2C write function
uint8_t ssd1306_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t i;
    
    if (len > 128) return 1;
    
    soft_i2c_start();
    
    // Send device address (with write bit)
    if (soft_i2c_send_byte(addr)) 
    {
        soft_i2c_stop();
        return 1;
    }
    
    // Send register/control byte
    if (soft_i2c_send_byte(reg))
    {
        soft_i2c_stop();
        return 1;
    }
    
    // Send data bytes
    for (i = 0; i < len; i++)
    {
        if (soft_i2c_send_byte(buf[i]))
        {
            soft_i2c_stop();
            return 1;
        }
    }
    
    soft_i2c_stop();
    return 0;
}
/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t ssd1306_interface_spi_init(void)
{
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t ssd1306_interface_spi_deinit(void)
{
    return 0;
}

/**
 * @brief     interface spi bus write
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t ssd1306_interface_spi_write_cmd(uint8_t *buf, uint16_t len)
{
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ssd1306_interface_delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ssd1306_interface_debug_print(const char *const fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    printf("%s", buffer);
}

/**
 * @brief  interface command && data gpio init
 * @return status code
 *         - 0 success
 *         - 1 gpio init failed
 * @note   none
 */
uint8_t ssd1306_interface_spi_cmd_data_gpio_init(void)
{
    return 0;
}

/**
 * @brief  interface command && data gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 gpio deinit failed
 * @note   none
 */
uint8_t ssd1306_interface_spi_cmd_data_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief     interface command && data gpio write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 gpio write failed
 * @note      none
 */
uint8_t ssd1306_interface_spi_cmd_data_gpio_write(uint8_t value)
{
    return 0;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 gpio init failed
 * @note   none
 */
uint8_t ssd1306_interface_reset_gpio_init(void)
{
    return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 gpio deinit failed
 * @note   none
 */
uint8_t ssd1306_interface_reset_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] value written value
 * @return    status code
 *            - 0 success
 *            - 1 gpio write failed
 * @note      none
 */
uint8_t ssd1306_interface_reset_gpio_write(uint8_t value)
{
    return 0;
}
