#include "haptic_drv.h"

// Function to write a value to a DRV2605 register using I2C
HAL_StatusTypeDef DRV2605_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(hi2c, DRV2605_ADDR, reg, 1, &value, 1, 100);
}

// Function to read a value from a DRV2605 register using I2C
HAL_StatusTypeDef DRV2605_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t *value)
{
  return HAL_I2C_Mem_Read(hi2c, DRV2605_ADDR, reg, 1, value, 1, 100);
}

// Function to check the DRV2605 device status
// Bit 7 is Diag_result expect 1
// Bit 6 is overcurrent expect 0
// Bit 5 is Standby expect 1 after init
HAL_StatusTypeDef DRV2605_CheckStatus(I2C_HandleTypeDef *hi2c, uint8_t *device_id) {
  return DRV2605_ReadRegister(hi2c, DRV2605_REG_STATUS, device_id);  // 0x0D is the Status Register
}

// Function to initialize the DRV2605
HAL_StatusTypeDef DRV2605_Init(I2C_HandleTypeDef *hi2c)
{
  HAL_StatusTypeDef status;
  
  // Exit standby mode
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_MODE, 0x00);
  if (status != HAL_OK) return status;

  // No real-time-playback
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_RTPIN, 0x00);
  if (status != HAL_OK) return status;

  // Strong Click
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_WAVESEQ1, 0x01);
  if (status != HAL_OK) return status;

  // End Sequence
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_WAVESEQ2, 0x00);
  if (status != HAL_OK) return status;

  // No Overdrive
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_OVERDRIVE, 0x00);
  if (status != HAL_OK) return status;

  // Setup Settings
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_SUSTAINPOS, 0x00);
  if (status != HAL_OK) return status;
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_SUSTAINNEG, 0x00);
  if (status != HAL_OK) return status;
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_BREAK, 0x00);
  if (status != HAL_OK) return status;
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_AUDIOMAX, 0x64);
  if (status != HAL_OK) return status;

  uint8_t value;
  // ERM (Eccentric Rotating Mass) Setting, we are using rotating discs
  status = DRV2605_ReadRegister(hi2c, DRV2605_REG_FEEDBACK, &value);
  if (status != HAL_OK) return status;
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_FEEDBACK, value & 0x7F);
  if (status != HAL_OK) return status;

  status = DRV2605_ReadRegister(hi2c, DRV2605_REG_CONTROL3, &value);
  if (status != HAL_OK) return status;
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_CONTROL3, value | 0x20);

  // Set Library to 1
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_LIBRARY, 0x01);
  if (status != HAL_OK) return status;

  // Exit standby mode
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_MODE, DRV2605_MODE_INTTRIG);
  if (status != HAL_OK) return status;

  return status;
}

// Function to play a vibration effect
HAL_StatusTypeDef DRV2605_PlayEffect(I2C_HandleTypeDef *hi2c, uint8_t effect)
{
  HAL_StatusTypeDef status;

  // Load effect into waveform sequencer register
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_WAVESEQ1, effect);
  if (status != HAL_OK) return status;

  // Set next waveform slot to 0 to indicate end of sequence
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_WAVESEQ1 + 1, 0x00);
  if (status != HAL_OK) return status;

  // Enable GO bit to start vibration
  return DRV2605_WriteRegister(hi2c, DRV2605_REG_GO, 0x01);
}

// Function to stop a vibration effect
HAL_StatusTypeDef DRV2605_Stop(I2C_HandleTypeDef *hi2c)
{
  HAL_StatusTypeDef status;

  // Disable GO bit to stop vibration
  status = DRV2605_WriteRegister(hi2c, DRV2605_REG_GO, 0x00);
  return status;
}