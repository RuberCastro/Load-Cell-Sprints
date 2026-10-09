/*
 * nau7802.c
 *
 *  Created on: Oct 1, 2026
 *      Author: ruber
 */

#include "nau7802.h"


// I2C1 is created in main.c */
extern I2C_HandleTypeDef hi2c1;


// Write one byte to an NAU7802 register
HAL_StatusTypeDef NAU7802_WriteReg(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c1,
                             NAU7802_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &value,
                             1,
                             HAL_MAX_DELAY);
}


//Read one byte from an NAU7802 register
HAL_StatusTypeDef NAU7802_ReadReg(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c1,
                            NAU7802_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            value,
                            1,
                            HAL_MAX_DELAY);
}


// Initialize NAU7802
HAL_StatusTypeDef NAU7802_Init(void)
{
    uint8_t status;
    uint32_t startTime;


    /* Reset device */
    if (NAU7802_WriteReg(NAU7802_PU_CTRL,
                         NAU7802_RR_BIT) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Clear reset and power up digital section */
    if (NAU7802_WriteReg(NAU7802_PU_CTRL,
                         NAU7802_PUD_BIT) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Wait until PUR bit becomes 1 */
    startTime = HAL_GetTick();

    do
    {
        if (NAU7802_ReadReg(NAU7802_PU_CTRL, &status) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (status & NAU7802_PUR_BIT)
        {
            break;
        }

    } while ((HAL_GetTick() - startTime) < 100);


    if (!(status & NAU7802_PUR_BIT))
    {
        return HAL_TIMEOUT;
    }


    /* CTRL1:
     * LDO = 3.3 V
     * Gain = 128
     */
    if (NAU7802_WriteReg(NAU7802_CTRL1, 0x27) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* CTRL2:
     * Channel 1
     * 80 samples/sec
     */
    if (NAU7802_WriteReg(NAU7802_CTRL2, 0x30) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* ADC configuration register */
    if (NAU7802_WriteReg(NAU7802_ADC_CTRL1, 0x30) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Power analog + digital sections */
    if (NAU7802_WriteReg(NAU7802_PU_CTRL,
                         NAU7802_AVDDS_BIT |
                         NAU7802_PUA_BIT |
                         NAU7802_PUD_BIT) != HAL_OK)
    {
        return HAL_ERROR;
    }


    return HAL_OK;
}


/* Read raw 24-bit ADC value*/
HAL_StatusTypeDef NAU7802_ReadRaw(int32_t *value)
{
    uint8_t status;
    uint8_t data[3];
    uint32_t startTime;


    if (value == NULL)
    {
        return HAL_ERROR;
    }


    /* Wait for conversion ready */
    startTime = HAL_GetTick();

    do
    {
        if (NAU7802_ReadReg(NAU7802_PU_CTRL, &status) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (status & NAU7802_CR_BIT)
        {
            break;
        }

    } while ((HAL_GetTick() - startTime) < 100);


    if (!(status & NAU7802_CR_BIT))
    {
        return HAL_TIMEOUT;
    }


    /* Read registers 0x12, 0x13, 0x14 */
    if (HAL_I2C_Mem_Read(&hi2c1,
                         NAU7802_ADDR,
                         NAU7802_ADCO_B2,
                         I2C_MEMADD_SIZE_8BIT,
                         data,
                         3,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Combine the three bytes */
    *value = ((int32_t)data[0] << 16) |
             ((int32_t)data[1] << 8)  |
              (int32_t)data[2];


    /* Sign extend 24-bit value to 32-bit */
    if (*value & 0x800000)
    {
        *value |= 0xFF000000;
    }


    return HAL_OK;
}
