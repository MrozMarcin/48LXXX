#include "48LXXX.h"

#define MEM_48LM01 // MEM_48L512 //

#if defined MEM_48LM01
#define MEM_SIZE_48LXXX 131072
#elif defined MEM_48L512
#define MEM_SIZE_48LXXX 65536
#endif

static int8_t MEM_48LXXX_WriteRegisters(MEM_48LXXX *dev, uint8_t *data, uint32_t length);
static int8_t MEM_48LXXX_WriteRegister(MEM_48LXXX *dev, uint8_t *data);
static int8_t MEM_48LXXX_ReadRegisters(MEM_48LXXX *dev, uint8_t *data, uint32_t length);
static int8_t MEM_48LXXX_ReadRegister(MEM_48LXXX *dev, uint8_t *data);

static int8_t MEM_48LXXX_waitUntillReady(MEM_48LXXX *dev);

inline static void MEM_48LXXX_Delay(uint32_t msec);
inline static void MEM_48LXXX_CS_Low(MEM_48LXXX *dev);
inline static void MEM_48LXXX_CS_High(MEM_48LXXX *dev);

int8_t MEM_48LXXX_Init(MEM_48LXXX *dev, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef *CS_GPIO_Port, uint16_t CS_GPIO_Pin)
{
	if(dev == NULL || spiHandle == NULL)
	{
		return -1;
	}
	
	dev->spiHandle = spiHandle;
	dev->spiCsPort = CS_GPIO_Port;
	dev->spiCsPin = CS_GPIO_Pin;
	
	MEM_48LXXX_CS_High(dev);
	
	dev->isAvalible = MEM_48LXXX_is_present(dev);		
	MEM_48LXXX_clearMemory(dev, 0x00000, MEM_SIZE_48LXXX);

	return 0;
}

bool MEM_48LXXX_is_present(MEM_48LXXX *dev)
{
	if(dev == NULL)
	{
		return false;
	}
	
	MEM_48LXXX_waitUntillReady(dev);
	
	int8_t status = MEM_48LXXX_enableWriteOperation(dev);
	if(status != 0)
	{
		return false;
	}
	
	const uint8_t value = 0x55;
	uint8_t readout = 0x00;

	status = MEM_48LXXX_writeValue(dev, 0x00000, value);
	if(status != 0)
	{
		return false;
	}
	
	status = MEM_48LXXX_readValue(dev, 0x00000, &readout);
	if(status != 0)
	{
		return false;
	}
	
	if (readout == value)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static int8_t MEM_48LXXX_waitUntillReady(MEM_48LXXX *dev)
{
	if(dev == NULL)
	{
		return -1;
	}
	int8_t status = 0x00;
	uint8_t status_readout = 0x00;
	
	for(uint8_t i = 0; i < 5; i++)
	{
		status = MEM_48LXXX_getStatus(dev, &status_readout);
		if(status != 0)
		{
			return -1;
		}
		if((status_readout & (0 << 0)) == (0 << 0))
		{
			break;
		}
	}
	
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_enableWriteOperation(MEM_48LXXX *dev)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	uint8_t cmd = MEM_48LXXX_WREN;
	MEM_48LXXX_CS_Low(dev);
	int8_t status = MEM_48LXXX_WriteRegister(dev, &cmd);
	MEM_48LXXX_CS_High(dev);

	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
	
int8_t MEM_48LXXX_disableWriteOperation(MEM_48LXXX *dev)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	uint8_t cmd = MEM_48LXXX_WRDI;
	MEM_48LXXX_CS_Low(dev);
	int8_t status = MEM_48LXXX_WriteRegister(dev, &cmd);
	MEM_48LXXX_CS_High(dev);

	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_getStatus(MEM_48LXXX *dev, uint8_t *readout)
{
	if(dev == NULL || readout == NULL)
	{
		return -1;
	}
	
	uint8_t cmd = MEM_48LXXX_RDSR;
	MEM_48LXXX_CS_Low(dev);
	int8_t status = MEM_48LXXX_WriteRegister(dev, &cmd);
	
	if(status != 0)
	{
		return -1;
	}	
	
	status = MEM_48LXXX_ReadRegister(dev, readout);
	MEM_48LXXX_CS_High(dev);
	
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_clearMemory(MEM_48LXXX *dev, uint32_t datasetAddress, uint32_t bytesToClear)
{
	if(dev == NULL || bytesToClear == 0)
	{
		return -1;
	}
	
	const uint16_t ramChunk = 256; 
	int8_t status = 0x00;
	int8_t dummyDataset[256];
	memset(dummyDataset, 0, 256);
	
	uint8_t address[3] =  {(datasetAddress & 0x00FF0000) >> 16, (datasetAddress & 0x0000FF00) >> 8, (datasetAddress & 0x000000FF)};
	
	MEM_48LXXX_waitUntillReady(dev);
	
	status = MEM_48LXXX_enableWriteOperation(dev);
	if(status != 0)
	{
		return -1;
	}
	
	uint8_t cmd = MEM_48LXXX_WRITE;	
	MEM_48LXXX_CS_Low(dev);
	status = MEM_48LXXX_WriteRegister(dev, &cmd);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}
	status = MEM_48LXXX_WriteRegisters(dev, address, 3);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}
	if(bytesToClear <= ramChunk)
	{
		status = MEM_48LXXX_WriteRegisters(dev, (uint8_t *) &dummyDataset, bytesToClear);
		if(status != 0)
		{
			MEM_48LXXX_CS_High(dev);
			return -1;
		}
	}
	else
	{
		uint8_t chunks = bytesToClear / ramChunk;
		uint8_t last_package_size = bytesToClear % ramChunk;
		
		for(uint16_t i = 0; i < chunks; i++)
		{
			status = MEM_48LXXX_WriteRegisters(dev, (uint8_t *) &dummyDataset, ramChunk);
			if(status != 0)
			{
				MEM_48LXXX_CS_High(dev);
				return -1;
			}
		}
		if(last_package_size > 0)
		{
			status = MEM_48LXXX_WriteRegisters(dev, (uint8_t *) &dummyDataset, last_package_size);
			if(status != 0)
			{
				MEM_48LXXX_CS_High(dev);
				return -1;
			}	
		}
	}
	MEM_48LXXX_CS_High(dev);

	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_writeValue(MEM_48LXXX *dev, uint32_t datasetAddress, uint8_t value)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	MEM_48LXXX_waitUntillReady(dev);
	
	int8_t status = 0x00;
	uint8_t cmd = MEM_48LXXX_WRITE;
	
	status = MEM_48LXXX_enableWriteOperation(dev);
	if(status != 0)
	{
		return -1;
	}		
	
	MEM_48LXXX_CS_Low(dev);
	status = MEM_48LXXX_WriteRegister(dev, &cmd);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	uint8_t address[3] =  {(datasetAddress & 0x00FF0000) >> 16, (datasetAddress & 0x0000FF00) >> 8, (datasetAddress & 0x000000FF)};
	status = MEM_48LXXX_WriteRegisters(dev, address, 3);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	status = MEM_48LXXX_WriteRegisters(dev, (uint8_t *)&value, 1);
	MEM_48LXXX_CS_High(dev);
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_readValue(MEM_48LXXX *dev, uint32_t datasetAddress, uint8_t *value)
{
	if(dev == NULL)
	{
		return -1;
	}
	
	MEM_48LXXX_waitUntillReady(dev);
	
	uint8_t cmd = MEM_48LXXX_READ;
	MEM_48LXXX_CS_Low(dev);
	int8_t status = MEM_48LXXX_WriteRegister(dev, &cmd);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	uint8_t address[3] =  {(datasetAddress & 0x00FF0000) >> 16, (datasetAddress & 0x0000FF00) >> 8, (datasetAddress & 0x000000FF)};
	status = MEM_48LXXX_WriteRegisters(dev, address, 3);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	status = MEM_48LXXX_ReadRegisters(dev, (uint8_t *)value, 1);
	MEM_48LXXX_CS_High(dev);
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_writeDataset(MEM_48LXXX *dev, uint32_t datasetAddress, uint16_t *dataset, uint16_t datasetSize)
{
	if(dev == NULL || dataset == NULL || datasetSize == 0)
	{
		return -1;
	}
	
	MEM_48LXXX_waitUntillReady(dev);
	
	int8_t status = 0x00;
	uint8_t cmd = MEM_48LXXX_WRITE;
	
	status = MEM_48LXXX_enableWriteOperation(dev);
	if(status != 0)
	{
		return -1;
	}		
	MEM_48LXXX_CS_Low(dev);
	status = MEM_48LXXX_WriteRegister(dev, &cmd);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	uint8_t address[3] =  {(datasetAddress & 0x00FF0000) >> 16, (datasetAddress & 0x0000FF00) >> 8, (datasetAddress & 0x000000FF)};
	status = MEM_48LXXX_WriteRegisters(dev, address, 3);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	status = MEM_48LXXX_WriteRegisters(dev, (uint8_t *)dataset, datasetSize*2);
	MEM_48LXXX_CS_High(dev);

	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int8_t MEM_48LXXX_readDataset(MEM_48LXXX *dev, uint32_t datasetAddress, uint16_t *dataset, uint16_t datasetSize, bool clearDatasetAfterRedaout)
{
	if(dev == NULL || dataset == NULL || datasetSize == 0)
	{
		return -1;
	}
	
	MEM_48LXXX_waitUntillReady(dev);
	
	uint8_t cmd = MEM_48LXXX_READ;
	MEM_48LXXX_CS_Low(dev);
	int8_t status = MEM_48LXXX_WriteRegister(dev, &cmd);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}	
	uint8_t address[3] =  {(datasetAddress & 0x00FF0000) >> 16, (datasetAddress & 0x0000FF00) >> 8, (datasetAddress & 0x000000FF)};
	status = MEM_48LXXX_WriteRegisters(dev, address, 3);
	if(status != 0)
	{
		MEM_48LXXX_CS_High(dev);
		return -1;
	}
	status = MEM_48LXXX_ReadRegisters(dev, (uint8_t *)dataset, datasetSize*2);
	MEM_48LXXX_CS_High(dev);

	if(clearDatasetAfterRedaout)
	{
		status = MEM_48LXXX_clearMemory(dev, datasetAddress, datasetSize*2);
		if(status != 0)
		{
			return -1;
		}
	}
	
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//int8_t MEM_48LXXX_PrintOutRegisters(MEM_48LXXX *dev)
//{
//	if(dev == NULL)
//	{
//		return -1;
//	}
//	#include "stdio.h"
//	for(uint16_t reg_no = 0; reg_no < MEM_SIZE_23XX1024; reg_no++)
//	{
//		uint8_t value_8 = 0x00;
//		if(MEM_48LXXX_ReadRegister(dev, reg_no, &value_8)==HAL_OK)
//		{
//			MEM_48LXXX_Delay(10);
//			printf("MEM %X val %X \n", reg_no, value_8);
//		}
//	}
//}

static int8_t MEM_48LXXX_WriteRegisters(MEM_48LXXX *dev, uint8_t *data, uint32_t length)
{
	if(data == NULL || length == 0 || length > MEM_SIZE_48LXXX)
	{
		return -1;
	}
	HAL_StatusTypeDef status;
	if(length >= 0xFFFF)
	{
		uint16_t chunks = length / 0xFFFF;
		uint16_t last_package_size = length % 0xFFFF;
		
		for(uint16_t i = 0; i < chunks; i++)
		{
			status =  HAL_SPI_Transmit(dev->spiHandle, data, 0xFFFF, MEM_48LXXX_WAIT_TIME_MAX);
			if(status != 0)
			{
				return -1;
			}
		}
		if(last_package_size>0)
		{
			status =  HAL_SPI_Transmit(dev->spiHandle, data, last_package_size, MEM_48LXXX_WAIT_TIME_MAX);
		}
	}
	else
	{
		status =  HAL_SPI_Transmit(dev->spiHandle, data, length, MEM_48LXXX_WAIT_TIME_MAX);
	}
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

static int8_t MEM_48LXXX_WriteRegister(MEM_48LXXX *dev, uint8_t *data)
{
	if(data == NULL)
	{
		return -1;
	}
	HAL_StatusTypeDef status = HAL_SPI_Transmit(dev->spiHandle, data, 1, MEM_48LXXX_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

static int8_t MEM_48LXXX_ReadRegisters(MEM_48LXXX *dev, uint8_t *data, uint32_t length)
{
	if(data == NULL || length == 0 || length > MEM_SIZE_48LXXX)
	{
		return -1;
	}
	HAL_StatusTypeDef status;
	if(length >= 0xFFFF)
	{
		uint16_t chunks = length / 0xFFFF;
		uint16_t last_package_size = length % 0xFFFF;
		
		for(uint16_t i = 0; i < chunks; i++)
		{
			status =  HAL_SPI_Receive(dev->spiHandle, data, 0xFFFF, MEM_48LXXX_WAIT_TIME_MAX);
			if(status != 0)
			{
				return -1;
			}
		}
		if(last_package_size>0)
		{
			status =  HAL_SPI_Receive(dev->spiHandle, data, last_package_size, MEM_48LXXX_WAIT_TIME_MAX);
		}
	}
	else
	{
		status =  HAL_SPI_Receive(dev->spiHandle, data, length, MEM_48LXXX_WAIT_TIME_MAX);
	}
	if(status != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

static int8_t MEM_48LXXX_ReadRegister(MEM_48LXXX *dev, uint8_t *data)
{
	if(data == NULL)
	{
		return -1;
	}
	HAL_StatusTypeDef status = HAL_SPI_Receive(dev->spiHandle, data, 1, MEM_48LXXX_WAIT_TIME_MAX);
	if(status != HAL_OK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief  Creates time delay
 * @param  msec number of miliseconds to wait
 * @retval none
 */
inline static void MEM_48LXXX_Delay(uint32_t msec)
{
	HAL_Delay(msec);
	//osDelay(msec);
}

/**
 * @brief  Creates time delay
 * @param  msec number of miliseconds to wait
 * @retval none
 */
inline static void MEM_48LXXX_CS_Low(MEM_48LXXX *dev)
{
    dev->spiCsPort->BSRR = (uint32_t)dev->spiCsPin << 16 ;
}

/**
 * @brief  Creates time delay
 * @param  msec number of miliseconds to wait
 * @retval none
 */
inline static void MEM_48LXXX_CS_High(MEM_48LXXX *dev)
{
		dev->spiCsPort->BSRR = (uint32_t)dev->spiCsPin;
}
