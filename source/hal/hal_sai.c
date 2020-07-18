/*
 * hal-sai.c
 *
 *  Created on: Jul 21, 2019
 *      Author: abusous2000
 */
#include "ch.h"
#include "hal.h"
#include "hal_sai.h"

#ifndef STM32F407xx

#if STM32_SAI_USE_SAI_BLOCK_A >0
SAIDriver SAID;
#endif
static void sai_lld_serve_tx_interrupt(SAIDriver *saip, uint32_t flags) {

  (void)saip;
  /* Callbacks handling, note it is portable code defined in the high
     level driver.*/
  if ((flags & STM32_DMA_ISR_TCIF) != 0) {
    /* Transfer complete processing.*/
	  _sai_isr_full_code(saip);
  }
  else if ((flags & STM32_DMA_ISR_HTIF) != 0) {
    /* Half transfer processing.*/
	  _sai_isr_half_code(saip);
  }
}
void saiObjectInit(SAIDriver *saip) {

	saip->sai  	 = NULL;
	saip->state  = SAI_UNINIT;
	saip->config = NULL;
}
void sai_lld_init(void) {
#ifdef STM32_SAI_USE_SAI_BLOCK_A
  if ( SAID.state != SAI_STOP)
	saiStop(&SAID);
  saiObjectInit(&SAID);
#ifdef BOARD_ST_STM32F769I_DISCOVERY
  SAID.sai       = SAI1;
  SAID.saiBlock  = SAI1_Block_A;
#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
  SAID.sai       = SAI2;
  SAID.saiBlock  = SAI2_Block_A;
#endif

  SAID.dmatx     =  STM32_DMA_STREAM(STM32_SAI_TX_DMA_STREAM);
  SAID.txdmamode =  STM32_DMA_CR_CHSEL(STM32_SAI_TX_DMA_CHN) |
                    STM32_DMA_CR_PL(STM32_SAI_DMA_PRIORITY) |
                    STM32_DMA_CR_PSIZE_HWORD |
                    STM32_DMA_CR_MSIZE_HWORD |
                    STM32_DMA_CR_DIR_M2P |
                    STM32_DMA_CR_MINC |
                    STM32_DMA_CR_CIRC |
                    STM32_DMA_CR_HTIE |
                    STM32_DMA_CR_TCIE |
                    STM32_DMA_CR_DMEIE |
                    STM32_DMA_CR_TEIE;
#else
  SAID.sai       = NULL;
  I2SD1.dmatx     = NULL;
  I2SD1.txdmamode = 0;
#endif
  SAID.state = SAI_INIT;
}
#define rccEnableSAI1(lp) rccEnableAPB2(RCC_APB2ENR_SAI1EN, lp)
#define rccEnableSAI2(lp) rccEnableAPB2(RCC_APB2ENR_SAI2EN, lp)
void saiStart(SAIDriver *saip, SAIConfig_TypeDef *saiConf){
	if ( saip->state == SAI_UNINIT)
		sai_lld_init();
	chSysLock();
	saip->config = saiConf;
	sai_lld_start(saip);
	chSysUnlock();

}
void saiStop(SAIDriver *saip){
	if ( saip->state == SAI_STOP || saip->state == SAI_UNINIT)
		return;
	chSysLock();
#ifdef BOARD_ST_STM32F769I_DISCOVERY
	rccEnableSAI1(false);
#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
	rccEnableSAI2(false);
#endif
	if ( saip->dmatx != NULL)
	   dmaStreamFreeI(saip->dmatx);
	saip->dmatx = NULL;
	saip->saiBlock->CR1 = 0;
	saip->saiBlock->CR2 = 0;
	saiObjectInit(saip);
	chSysUnlock();
}

void sai_lld_stop_exchange(SAIDriver *saip) {

  /* Stop transfer.*/
  saip->saiBlock->IMR &=  ~SAI_xIMR_WCKCFGIE;
}
void saiStopExchange(SAIDriver *saip){
	sai_lld_stop_exchange(saip);
}
void sai_lld_start(SAIDriver *saip) {
	  /* If in stopped state then enables the SAI and DMA clocks.*/
	  if (saip->state == SAI_STOP || saip->state == SAI_INIT) {
#if STM32_SAI_USE_SAI_BLOCK_A == TRUE
	    if (&SAID == saip) {
	    	sai_lld_stop_exchange(saip);
			saip->saiBlock->CR1 &=  ~SAI_xCR1_SAIEN;
			/* Enabling SAI unit clock.*/
#ifdef BOARD_ST_STM32F769I_DISCOVERY
			rccEnableSAI1(true);
#elif defined(BOARD_ST_STM32F746G_DISCOVERY)
			rccEnableSAI2(true);
#endif
			saip->dmatx = dmaStreamAllocI(STM32_SAI_TX_DMA_STREAM, STM32_SAI_DMA_PRIORITY,
										  (stm32_dmaisr_t)sai_lld_serve_tx_interrupt,(void *)saip);
			osalDbgAssert(saip->dmatx != NULL, "unable to allocate stream");

			/* CRs settings are done here because those never changes until
			   the driver is stopped.*/
			uint32_t tmp = SAI_xCR1_MODE | SAI_xCR1_PRTCFG |  SAI_xCR1_DS |      \
					       SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR | SAI_xCR1_SYNCEN | \
					       SAI_xCR1_MONO | SAI_xCR1_OUTDRIV  | SAI_xCR1_DMAEN |  \
					       SAI_xCR1_NODIV | SAI_xCR1_MCKDIV;//0xfa3fef
			saip->sai->GCR |= saip->config->GCR;
			saip->saiBlock->CR1 &= ~tmp;
//			tmp =0;
//			tmp = SAI_xCR2_FTH | SAI_xCR2_FFLUSH | SAI_xCR2_COMP | SAI_xCR2_CPL;
//			saip->saiBlock->CR2   &= ~tmp;
			saip->saiBlock->FRCR  = 0;
			saip->saiBlock->SLOTR = 0;
			saip->saiBlock->CR2   = 0;
			saip->saiBlock->CR2   |= saip->config->CR2;
			saip->saiBlock->FRCR  |= saip->config->FRCR;
			saip->saiBlock->SLOTR |= saip->config->SLOTR;
			saip->saiBlock->SR     = saip->config->SR;
			saip->saiBlock->CR1   |= saip->config->CR1 | SAI_xCR1_DMAEN |  SAI_xCR1_SAIEN;
			saip->state 		   = SAI_READY;
	    }
#endif//#if STM32_SAI_USE_SAI_BLOCK_A == TRUE
	  }
}
void saiStartExchange(SAIDriver *saip) {
	chSysLock();
	sai_lld_start_exchange(saip);
	chSysUnlock();
}
void sai_lld_start_exchange(SAIDriver *saip) {
  size_t size = saip->config->size;
  /* TX DMA setup.*/
  if (NULL != saip->dmatx) {
    dmaStreamSetMode(saip->dmatx, saip->txdmamode);
    dmaStreamSetPeripheral(saip->dmatx, &saip->saiBlock->DR);
    dmaStreamSetMemory0(saip->dmatx, saip->config->tx_buffer);
    dmaStreamSetTransactionSize(saip->dmatx, size);
    dmaStreamEnable(saip->dmatx);
  }

  /* Starting transfer.*/
  saip->saiBlock->IMR |=  SAI_xIMR_WCKCFGIE;
}
#endif
