/**
 * @author  Tilen Majerle
 * @email   tilen@majerle.eu
 * @website http://stm32f4-discovery.com
 * @link    http://stm32f4-discovery.com/2015/08/hal-library-23-touch-for-stm32fxxx/
 * @version v1.0
 * @ide     Keil uVision
 * @license GNU GPL v3
 * @brief   FT5336 low level library
 *	
\verbatim
   ----------------------------------------------------------------------
    Copyright (C) Tilen Majerle, 2015
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
     
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
\endverbatim
 */
#ifndef TM_TOUCH_FT5336_H
#define TM_TOUCH_FT5336_H 100

/* C++ detection */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup TM_STM32Fxxx_HAL_Libraries
 * @{
 */

/**
 * @defgroup TM_TOUCH_FT5336
 * @brief    FT5336 low level library - http://stm32f4-discovery.com/2015/08/hal-library-23-touch-for-stm32fxxx/
 * @{
 *
 * This is a low-level driver for FT5336 touch screen controller.
 *
 * To use this library, check @ref TM_TOUCH library on how to use it properly.
 *
 * \par Select custom I2C
 *
 * This touch controller is used on STM32F7-Discovery board and settings for I2C are for this board by default.
 * If you have own board and you need custom pins for I2C, open defines.h file and add/edit following lines.
 *
\code
//Select I2C for FT5336 conroller
#define TOUCH_FT5336_I2C      I2C3
#define TOUCH_FT5336_I2C_PP   TM_I2C_PinsPack_2
\endcode
 *
 * @note  Check @ref TM_I2C library for corresponding pins for selected I2C peripheral and pinspack
 *
 * \par Changelog
 *
\verbatim
 Version 1.0
  - First release
\endverbatim
 *
 * \par Dependencies
 *
\verbatim
 - STM32Fxxx HAL
 - defines.h
 - TM TOUCH
 - TM I2C
 - TM GPIO
\endverbatim
 */
#include "stm32fxxx_hal.h"
#include "defines.h"
#include "tm_stm32_touch.h"
#include "tm_stm32_i2c.h"
#include "tm_stm32_gpio.h"

/**
 * @defgroup TM_TOUCH_FT5336_Macros
 * @brief    Library defines
 * @{
 */

/* Touch I2C settings */
#ifndef TOUCH_FT5336_I2C
#define TOUCH_FT5336_I2C        I2C3
#define TOUCH_FT5336_I2C_PP     TM_I2C_PinsPack_2
#endif

/* I2C device address */
#define TOUCH_FT5336_I2C_DEV    0x70

/**
 * @}
 */
 
/**
 * @defgroup TM_TOUCH_FT5336_Typedefs
 * @brief    Library Typedefs
 * @{
 */
/* Typedefs here */
/**
 * @}
 */

/**
 * @defgroup TM_TOUCH_FT5336_Functions
 * @brief    Library Functions
 * @{
 */

/**
 * @brief  Initializes FT5336 touch controller
 * @param  *TS: Pointer to @ref TM_TOUCH_t structure with settings
 * @retval Touch status:
 *            - 0: OK
 *            - > 0: Error
 */
uint8_t TM_TOUCH_FT5336_Init(TM_TOUCH_t* TS);

/**
 * @brief  Reads touch data from FT5336 touch controller
 * @param  *TS: Pointer to @ref TM_TOUCH_t to save data into
 * @retval Touch status:
 *            - 0: OK
 *            - > 0: Error
 */
uint8_t TM_TOUCH_FT5336_Read(TM_TOUCH_t* TS);

/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif
