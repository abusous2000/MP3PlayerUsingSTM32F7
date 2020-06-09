/*
 * hal_sai.h
 *
 *  Created on: Jul 21, 2019
 *      Author: abusous2000
 */
#ifndef HAL_SAI_H
#define HAL_SAI_H
#include "hal.h"
#define STM32_SAI_USE_SAI_BLOCK_A			1

#ifdef BOARD_ST_STM32F769I_DISCOVERY
#define STM32_SAI_DMA_PRIORITY         		3
#define STM32_SAI_TX_DMA_STREAM           	STM32_DMA_STREAM_ID(2, 1)//See P.226 in RM
#define STM32_SAI_TX_DMA_CHN               	0

#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
#define STM32_SAI_DMA_PRIORITY         		3
#define STM32_SAI_TX_DMA_STREAM           	STM32_DMA_STREAM_ID(2, 4)//See P.226 in RM
#define STM32_SAI_TX_DMA_CHN               	3

#else
#error "SAI Config DMA params not defined for board"
#endif

typedef struct SAIDriver SAIDriver;
typedef void (*saicallback_t)(SAIDriver *saip, size_t offset, size_t n);
typedef struct
{
  uint32_t 		GCR;
  uint32_t 		CR1;
  uint32_t 		CR2;
  uint32_t 		FRCR;
  uint32_t 		SLOTR;
  uint32_t 		SR;
  const void    *tx_buffer;
  size_t        size;
  saicallback_t end_cb;
}SAIConfig_TypeDef;
typedef enum {
  SAI_UNINIT = 0,
  SAI_STOP,
  SAI_READY,
  SAI_ACTIVE,
  SAI_COMPLETE,
  SAI_ERROR,
  SAI_INIT,
} saistate_t;

struct SAIDriver {
		saistate_t          state;
		SAI_TypeDef         *sai;//SAI
  SAI_Block_TypeDef			*saiBlock;//SAI_Block_A
  SAIConfig_TypeDef   		*config;
  const stm32_dma_stream_t  *dmatx;
  	    uint32_t            txdmamode;
};


#define _sai_isr_half_code(saip) {                                          \
  if ((saip)->config->end_cb != NULL) {                                     \
    (saip)->config->end_cb(saip, 0, (saip)->config->size / 2);              \
  }                                                                         \
}

#define _sai_isr_full_code(saip) {                                           \
  if ((saip)->config->end_cb) {                                             \
    (saip)->state = SAI_COMPLETE;                                           \
    (saip)->config->end_cb(saip,                                            \
                           (saip)->config->size / 2,                        \
                           (saip)->config->size / 2);                       \
    if ((saip)->state == SAI_COMPLETE)                                      \
      (saip)->state = SAI_READY;                                            \
  }                                                                         \
  else                                                                      \
    (saip)->state = SAI_READY;                                              \
}
void saiObjectInit(SAIDriver *saip);
void sai_lld_init(void);
void saiStart(SAIDriver *saip, SAIConfig_TypeDef *saiConf);
void saiStop(SAIDriver *saip);
void sai_lld_start(SAIDriver *saip);
void saiStartExchange(SAIDriver *saip);
void sai_lld_start_exchange(SAIDriver *saip);
void saiStartExchange(SAIDriver *saip);
extern SAIDriver SAID;
#endif
