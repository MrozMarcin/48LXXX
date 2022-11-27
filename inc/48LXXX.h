#ifndef MEM_48LXXX_DRIVER_H
#define MEM_48LXXX_DRIVER_H

#include "stm32l1XX_hal.h"
#include "stdbool.h"

/* INSTRUCTION CODES*/
#define MEM_48LXXX_WREN					(0x06)
#define MEM_48LXXX_WRDI					(0x04)

#define MEM_48LXXX_WRITE				(0x02)
#define MEM_48LXXX_READ		 			(0x03)
#define MEM_48LXXX_SECWRITE			(0x12)
#define MEM_48LXXX_SECREAD		 	(0x13)

#define MEM_48LXXX_WRSR					(0x01)
#define MEM_48LXXX_RDSR					(0x05)

#define MEM_48LXXX_STORE				(0x08)
#define MEM_48LXXX_RECALL				(0x09)

#define MEM_48LXXX_WRNUR				(0xC2)
#define MEM_48LXXX_RDNUR				(0xC3)

#define MEM_48LXXX_HIBERNATE		(0xB9)

#define MEM_48LXXX_WAIT_TIME_MAX	0xFFFF

typedef struct
{
		SPI_HandleTypeDef *spiHandle;
		bool isAvalible;
		GPIO_TypeDef *spiCsPort;
		uint16_t spiCsPin;
}MEM_48LXXX;

int8_t MEM_48LXXX_Init(MEM_48LXXX *dev, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef *CS_GPIO_Port, uint16_t CS_GPIO_Pin);
bool MEM_48LXXX_is_present(MEM_48LXXX *dev);
int8_t MEM_48LXXX_PrintOutRegisters(MEM_48LXXX *dev);

int8_t MEM_48LXXX_writeValue(MEM_48LXXX *dev, uint32_t datasetAddress, uint8_t value);
int8_t MEM_48LXXX_writeDataset(MEM_48LXXX *dev, uint32_t datasetAddress, uint16_t *dataset, uint16_t datasetSize);
int8_t MEM_48LXXX_readValue(MEM_48LXXX *dev, uint32_t datasetAddress, uint8_t *value);
int8_t MEM_48LXXX_readDataset(MEM_48LXXX *dev, uint32_t datasetAddress, uint16_t *dataset, uint16_t datasetSize, bool clearDatasetAfterRedaout);
int8_t MEM_48LXXX_getStatus(MEM_48LXXX *dev, uint8_t *readout);

int8_t MEM_48LXXX_enableWriteOperation(MEM_48LXXX *dev);
int8_t MEM_48LXXX_disableWriteOperation(MEM_48LXXX *dev);

int8_t MEM_48LXXX_clearMemory(MEM_48LXXX *dev, uint32_t datasetAddress, uint32_t bytesToClear);

#endif /* MEM_48LXXX_DRIVER_H */
