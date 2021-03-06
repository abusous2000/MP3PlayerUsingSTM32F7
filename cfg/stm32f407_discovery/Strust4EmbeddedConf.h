/*
 * Strust4Embedded.h
 *
 *  Created on: Dec 12, 2019
 *      Author: abusous2000
 */

#ifndef CFG_STRUST4EMBEDDEDCONF_H_
#define CFG_STRUST4EMBEDDEDCONF_H_

#define USE_USBCFG
#define INCLUDE_CS43L22         1
//#define MAX_ACTION_EVENTS                6
#define BLINKER_THD_STACK_SIZE             1024//512

/*
 * Pins Used by Ethernet
 *  PA1  - ETH_RMII_REF_CLK
 *  PA2  - ETH_RMII_MDIO
 *	PB11 - GPIOB_PIN11_ETH_TX_EN
 *	PB12 - GPIOB_PIN12_ETH_TX0
 *	PB13 - GPIOB_PIN13_ETH_TX1
 *  PC1  - ETH_RMII_MDC
 *  PC4  - ETH_RMII_RXD0
 *  PC5  - ETH_RMII_RXD1
 *
 *  Pins used by Audio module for I2S3 & I2C1
 *  #define AUDIO_RESET_LINE 				PAL_LINE(GPIOD, 4U)//PD4
 *  #define AUDIO_SCL_LINE 					PAL_LINE(GPIOB, 6U)//PB6
 *  #define AUDIO_SDA_LINE 					PAL_LINE(GPIOB, 9U)//PB9
 *  #define AUDIO_I2S_PIN_MODE          	PAL_STM32_OTYPE_PUSHPULL | PAL_MODE_ALTERNATE(AUDIO_I2S_AF) | PAL_STM32_OSPEED_MID2
 *  #define AUDIO_I2S_SCK_LINE 		    	PAL_LINE(GPIOC, 10U)//PC10
 *  #define AUDIO_I2S_SD_LINE 		   		PAL_LINE(GPIOC, 12U)//PC12
 *  #define AUDIO_I2S_WS_LINE 		    	PAL_LINE(GPIOA, 4U) //PA4
 *  #define AUDIO_I2S_MCK_LINE 		    	PAL_LINE(GPIOC, 7U) //PC7
 *
 */

#ifndef S4E_USE_SDCARD
#define S4E_USE_SDCARD 			1
#endif

#ifndef USE_LCD_TFT
#define USE_LCD_TFT 			0
#endif

#ifndef S4E_USE_JOYSTICK
#define S4E_USE_JOYSTICK        0
#endif

#ifndef S4E_USE_BLINKER_THD
#define S4E_USE_BLINKER_THD     1
#endif

#ifndef S4E_USE_WIFI_MODULE_THD
#define S4E_USE_WIFI_MODULE_THD 0
#endif

#ifndef S4E_USE_SSD1306_LCD
#define S4E_USE_SSD1306_LCD     0
#endif

#ifndef S4E_USE_POT
#define S4E_USE_POT             1

#define TIM_TGRO_EVENT_EXTSEL 	0x08 //See p. 386 in RM
#define ADC_POT_TIM             GPTD3 ///See p. 386 in RM
#define ADC_POT                 ADCD1
#define ADC_CHANNEL_IN 			ADC_CHANNEL_IN3//See Table 10 in user manual or datasheet
#define POT_GPIO_PIN            PAL_LINE(GPIOA, 3U)//PA3

#endif

#ifndef S4E_USE_PWM
#define S4E_USE_PWM             0
#define PWM_LINE1 				PAL_LINE(GPIOC, 6U)//PC6
#define PWM_LINE2 				PAL_LINE(GPIOC, 7U)//PC7
#define PWM_LINE1_CH 			0
#define PWM_LINE2_CH 			1
#define PWM_LINE_AF   			3
#define PWM_DRIVER 				PWMD8
#endif


#ifndef S4E_USE_BUZZER
#define S4E_USE_BUZZER			0
#endif

#ifndef S4E_USE_RGB
#define S4E_USE_RGB             0
#endif
#ifndef S4E_USE_MQTT
#define S4E_USE_MQTT            1
#endif

#ifndef S4E_USE_WEB_SERVER
#define S4E_USE_WEB_SERVER      0
#endif

#ifndef S4E_USE_FAULT_HANDLER
#define S4E_USE_FAULT_HANDLER   1
#endif

#if S4E_USE_WEB_SERVER != 0 || S4E_USE_MQTT  != 0
#define S4E_USE_ETHERNET		1
#else
#define S4E_USE_ETHERNET		0
#endif

#define SSID_KEY						"ssid"
#define SSID_VALUE						"ROR194"
#define APP_NAME_KEY					"appName"
#define APP_NAME_VALUE					"MP3Player w/ S4E"
#define PWD_KEY							"pwd"
#define PWD_VALUE						"salah987"
#define RELOAD_HOST_KEY					"reloadHost"
#define RELOAD_HOST_VALUE				"raw.githubusercontent.com"
#define RELOAD_URL_KEY					"reloadURL"
#define RELOAD_URL_VALUE				"/abusous2000/MP3PlayerUsingSTM32F7AndChibiOS/master/source/ESP8266WiFiModule/defaultPage.html"
#define FP_KEY							"fp"
#define FP_VALUE						"CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33"
#define NUM_OF_WORKER_THD_KEY			"numOfWorkerThds"
#define NUM_OF_WORKER_THD_VALUE 		"1"
#define DEFAULT_NUM_OF_WORKER_THDS 		1
#define MAX_THD_NAME_SIZE          		25
#define MAX_NUM_OF_WORKER_THDS			4
//Note that for this board, serial output uses PA2 & PA3 (w/ AF 7) which is in conflict with Ethernet with PA2
#define PORTAB_USB                      USBD1
#define PORTAB_SDU                      SDU1
#define PORTAB_SD 					    SDU1
#define PORTAB_SD_VCP				    SD6

#define MQTT_CMD_SUBSCRIBE_TOPIC   		"dev/cmd/STM32F407/mp3Player"
#define DEFAULT_MQTT_PUBLISH_TOPIC 		"dev/update/STM32F407/mp3Player"
#define MQTT_CLIENT_ID                  "Struts4EmbeddedWithSTM32F407"
#define MQTT_PUBLISH_POPULATE_DD_TOPIC  "dev/update/STM32F407/mp3Player/populateTracksDropDown"


#define LINE_LED_GREEN                  PAL_LINE(GPIOD, GPIOD_LED4)//PD12
#define LINE_LED_RED                    PAL_LINE(GPIOD, GPIOD_LED5)//PD14

//needed for For STM32L4
//#define MMC_SPI_PIN_MODE                PAL_MODE_ALTERNATE(MMC_SPI_AF) 	  |  PAL_STM32_PUPDR_PULLUP | PAL_STM32_OSPEED_HIGHEST
#if 1
#define PROTAB_MMCSPI                   SPID2
#define MMC_SPI_AF						5
#define SD_CARD_NSS_LINE 				PAL_LINE(GPIOE, 5U) //PE5
#define SD_CARD_SCK_LINE 				PAL_LINE(GPIOB, 10U)//PB10
#define SD_CARD_MISO_LINE 				PAL_LINE(GPIOC, 2U) //PC2
#define SD_CARD_MOSI_LINE 				PAL_LINE(GPIOC, 3U) //PC3

#else

#define PROTAB_MMCSPI                   SPID1
#define MMC_SPI_AF						5
#define SD_CARD_NSS_LINE 				PAL_LINE(GPIOC, 4U) //PC4
#define SD_CARD_SCK_LINE 				PAL_LINE(GPIOA, 5U) //PA5
#define SD_CARD_MISO_LINE 				PAL_LINE(GPIOA, 6U) //PA6
#define SD_CARD_MOSI_LINE 				PAL_LINE(GPIOA, 7U) //PA7
#endif
#define MMC_SPI_PIN_MODE                PAL_MODE_ALTERNATE(MMC_SPI_AF) 	  |  PAL_STM32_OSPEED_HIGHEST
#define MMC_SPI_PIN_MODE_MISO           PAL_MODE_ALTERNATE(MMC_SPI_AF)

//#define MPU_SCL_PIN						PAL_LINE(GPIOB, 10U)//LINE_ARD_D15
//#define MPU_SDA_PIN						PAL_LINE(GPIOB, 11U)//LINE_ARD_D14
//#define MPU_I2C_AF						4
//#define MPU_I2CD						I2CD2
//#define LINE_ARD_D2                   PAL_LINE(GPIOA, 10U)
//#define MPU_INT_PORT					GPIOA
//#define MPU_INT_PIN						10

#ifndef LINE_JOY_UP
#define USER_BUTTON                     PAL_LINE(GPIOA, 0U)//PA0
#define USER_BUTTON_EVENT               NEXT_TRACK_AE_NAME
#endif


#endif /* CFG_STRUST4EMBEDDED_H_ */
