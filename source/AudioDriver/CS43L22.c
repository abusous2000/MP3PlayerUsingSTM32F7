/*
 * CS43L22.c
 *
 *  Created on: Jul 9, 2020
 *      Author: abusous2000
 */
#include "Strust4EmbeddedConf.h"
#ifdef INCLUDE_CS43L22
#include "audio.h"
#include "CS43L22.h"

static uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
static uint32_t codecVolumeCtrl(uint16_t DeviceAddr, uint8_t Volume);
static uint32_t codecReset(int16_t DeviceAddr);
static uint32_t Codec_Mute(uint16_t DeviceAddr, uint32_t Cmd){(void)DeviceAddr;
  uint32_t counter = 0;

  /* Set the Mute mode */
  if (Cmd == AUDIO_MUTE_ON)
  {
    counter += Codec_WriteRegister(0x04, 0xFF);
  }
  else /* AUDIO_MUTE_OFF Disable the Mute */
  {
    counter += Codec_WriteRegister(0x04, 0xAF);
  }

  return counter;
}

static uint32_t codecInit(uint16_t DeviceAddr, uint16_t OutputInputDevice, uint8_t Volume, uint32_t AudioFreq){(void)OutputInputDevice;(void)AudioFreq;
  uint32_t counter = 0;

  /* Reset the Codec Registers */

  AUDIO_IO_Init();

  codecReset(DeviceAddr);
  /* Keep Codec powered OFF */
  counter += Codec_WriteRegister(0x02, 0x01);

  counter += Codec_WriteRegister(0x04, 0xAF); /* SPK always OFF & HP always ON */

  /* Clock configuration: Auto detection */
  counter += Codec_WriteRegister(0x05, 0x81);

  /* Set the Slave Mode and the audio Standard */
  counter += Codec_WriteRegister(0x06, CODEC_STANDARD);

  /* Set the Master volume */
  codecVolumeCtrl(DeviceAddr,Volume);

  /* Power on the Codec */
  counter += Codec_WriteRegister(0x02, 0x9E);

  /* Additional configuration for the CODEC. These configurations are done to reduce
      the time needed for the Codec to power off. If these configurations are removed,
      then a long delay should be added between powering off the Codec and switching
      off the I2S peripheral MCLK clock (which is the operating clock for Codec).
      If this delay is not inserted, then the codec will not shut down properly and
      it results in high noise after shut down. */

  /* Disable the analog soft ramp */
  counter += Codec_WriteRegister(0x0A, 0x00);
  /* Disable the limiter attack level */
  counter += Codec_WriteRegister(0x27, 0x00);
  /* Adjust Bass and Treble levels */
  counter += Codec_WriteRegister(0x1F, 0x0F);
  /* Adjust PCM volume level */
  counter += Codec_WriteRegister(0x1A, 0x0A);
  counter += Codec_WriteRegister(0x1B, 0x0A);

  /* Configure the I2S peripheral & start the driver*/
  AUDIO_IO_ReInit(AudioFreq);

  /* Return communication control value */
  return counter;
}

static uint32_t codecVolumeCtrl(uint16_t DeviceAddr, uint8_t Volume1){(void)DeviceAddr;
  uint32_t counter = 0;
  uint8_t Volume = VOLUME_CONVERT(Volume1);
  if (Volume > 0xE6)  {
    /* Set the Master volume */
    counter += Codec_WriteRegister(0x20, Volume - 0xE7);
    counter += Codec_WriteRegister(0x21, Volume - 0xE7);
  }
  else  {
    /* Set the Master volume */
    counter += Codec_WriteRegister(0x20, Volume + 0x19);
    counter += Codec_WriteRegister(0x21, Volume + 0x19);
  }

  return counter;
}

static uint32_t codecReset(int16_t DeviceAddr){(void)DeviceAddr;
  /* Power Down the codec */
  palClearLine(AUDIO_RESET_LINE);
  /* wait for a delay to insure registers erasing */
  chThdSleepMilliseconds(10);
  /* Power on the codec */
  palSetLine(AUDIO_RESET_LINE);

  return 0;
}
static uint8_t Codec_read_byte(uint8_t reg){
	uint32_t result = 0;
	uint8_t  txbuf[1];
	uint8_t  rxbuf[1] = {0};

	txbuf[0]=reg;
	i2cAcquireBus(&I2CD_CODEC);
	result=i2cMasterTransmitTimeout(&I2CD_CODEC, AUDIO_I2C_ADDRESS, txbuf, 1 , &rxbuf[0], 1, 1000);
	i2cReleaseBus(&I2CD_CODEC);
	UNUSED(result);
	return rxbuf[0];
}

static uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue){
  uint32_t result = 0;
  uint8_t  txbuf[2];

  txbuf[0]=RegisterAddr; txbuf[1]=RegisterValue;
  result=i2cMasterTransmitTimeout(&I2CD_CODEC, AUDIO_I2C_ADDRESS, txbuf, 2 , NULL, 0, 1000);

  /* Return the verifying value: 0 (Passed) or any other (Failed) */
  return result;
}
static uint32_t cs43l22_ReadID(uint16_t DeviceAddr){(void)DeviceAddr;
  uint8_t Value;
  AUDIO_IO_Init();
  Value = Codec_read_byte(CS43L22_CHIPID_ADDR);
  Value = (Value & CS43L22_ID_MASK);

  return((uint32_t) Value);
}
uint32_t cs43l22_Play(uint16_t DeviceAddr, uint16_t* pBuffer, uint16_t Size){(void)Size;(void)pBuffer;
  uint32_t counter = 0;

  /* Resumes the audio file playing */
  /* Unmute the output first */
  counter += Codec_Mute(DeviceAddr, AUDIO_MUTE_OFF);

  counter += Codec_WriteRegister(0x02, 0x9E);

  return counter;
}

CodecDriverITF_Typedef CS43L22_drv =
{
  .Init=codecInit,
  .ReadID=cs43l22_ReadID,
  .SetVolume=codecVolumeCtrl,
  .SetMute=Codec_Mute,
  .Play=cs43l22_Play,
  .Reset=codecReset,
  .address=AUDIO_I2C_ADDRESS
};
CodecDriverITF_Typedef *getCodecDriver(void){
    return &CS43L22_drv;
}

#endif
