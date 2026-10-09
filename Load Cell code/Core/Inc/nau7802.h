/*
 * nau7802.h
 *
 *  Created on: Oct 1, 2026
 *      Author: ruber
 */

#ifndef INC_NAU7802_H_
#define INC_NAU7802_H_

#include "main.h"
#include <stdint.h>

/* NAU7802 I2C Address */
#define NAU7802_ADDR            (0x2A << 1)

/* Register Addresses */
#define NAU7802_PU_CTRL         0x00
#define NAU7802_CTRL1           0x01
#define NAU7802_CTRL2           0x02

#define NAU7802_ADCO_B2         0x12
#define NAU7802_ADCO_B1         0x13
#define NAU7802_ADCO_B0         0x14

#define NAU7802_ADC_CTRL1       0x15


/* PU_CTRL Register Bits */
#define NAU7802_RR_BIT          (1 << 0)
#define NAU7802_PUD_BIT         (1 << 1)
#define NAU7802_PUA_BIT         (1 << 2)
#define NAU7802_PUR_BIT         (1 << 3)
#define NAU7802_CS_BIT          (1 << 4)
#define NAU7802_CR_BIT          (1 << 5)
#define NAU7802_OSCS_BIT        (1 << 6)
#define NAU7802_AVDDS_BIT       (1 << 7)


/* Function Prototypes */
HAL_StatusTypeDef NAU7802_Init(void);

HAL_StatusTypeDef NAU7802_ReadRaw(int32_t *value);



#endif /* INC_NAU7802_H_ */
