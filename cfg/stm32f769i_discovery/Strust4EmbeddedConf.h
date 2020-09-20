/*
 * Strust4Embedded.h
 *
 *  Created on: Dec 12, 2019
 *      Author: abusous2000
 */

#ifndef CFG_STRUST4EMBEDDEDCONF_H_
#define CFG_STRUST4EMBEDDEDCONF_H_

//#define MAX_ACTION_EVENTS             6
#define INCLUDE_WM8994                  1
#define PROCESS_ACTION_EVENTS_THD_WA   	1024
#define BLINKER_THD_STACK_SIZE          1024//512
#define LCD_THD_STACK_SIZE             	512
#define AUDIO_THD_WA_STACK_SIZE   		0x2000


#define S4E_USE_SDCARD 					1
#define USE_LCD_TFT 					1

#define S4E_USE_JOYSTICK        		0
#define S4E_USE_BLINKER_THD     		1
#define S4E_USE_WIFI_MODULE_THD 		0

#define S4E_USE_SSD1306_LCD     		0
#define S4E_USE_POT             		0
#define S4E_USE_PWM             		0
#define S4E_USE_RGB             		0
#define S4E_USE_MQTT            		1
#define S4E_USE_WEB_SERVER      		0
#define S4E_USE_FAULT_HANDLER   		1

#if S4E_USE_WEB_SERVER != 0 || S4E_USE_MQTT  != 0
#define S4E_USE_ETHERNET				1
#else
#define S4E_USE_ETHERNET				0
#endif
#define USER_BUTTON		                PAL_LINE(GPIOA, GPIOA_BUTTON_USER)
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

#ifndef PORTAB_SD
#define PORTAB_SD                      SD1
#endif

#define MQTT_CMD_SUBSCRIBE_TOPIC    	"dev/cmd/STM32F769i/mp3Player"
#define DEFAULT_MQTT_PUBLISH_TOPIC      "dev/update/STM32F769i/mp3Player"
#define MQTT_CLIENT_ID                  "Struts4EmbeddedWithStm32F769i"
#define MQTT_PUBLISH_POPULATE_DD_TOPIC  "dev/update/STM32F769i/mp3Player/populateTracksDropDown"

#define SSD1306_I2C_AF       			4
#define SSD1306_I2C_SCL_PORT 			GPIOB
#define SSD1306_I2C_SCL_PIN  			GPIOB_ARD_D15
#define SSD1306_I2C_SDA_PORT 			GPIOB
#define SSD1306_I2C_SDA_PIN  			GPIOB_ARD_D14
#define SSD1306_SA0_PORT 				GPIOJ
#define SSD1306_SA0_PIN    				GPIOJ_ARD_D8
#define SSD1306_RESET_PORT				GPIOJ
#define SSD1306_RESET_PIN				GPIOJ_ARD_D7

#define TIM_TGRO_EVENT_EXTSEL 		    12 //See table 98 in RM on pages 451 & 471 for details
#define ADC_POT                     	ADCD2
#define ADC_CHANNEL_IN 					ADC_CHANNEL_IN6

#define PPM_FRAME_DECODER				0
#define PPM_DECODING_DEBUG				0
#define FREQUENCY_USED          		50000
#define RC_MIN_VALUE                    30
#define RC_MID_VALUE                    55
#define RC_MAX_VALUE                    80
#define MAX_FRAMES_TO_COLLECT     		10
#if 0
#define PORTABLE_PWM_LINE 				LINE_ARD_D5 //PC8
#define PORTABLE_PWMD 					PWMD8
#define PORTABLE_PWM_AF  				2
#define PORTABLE_PWM_CHANNEL			2
#define CH1_CB 							{PWM_OUTPUT_DISABLED, NULL}
#define CH2_CB 							{PWM_OUTPUT_DISABLED, NULL}
#define CH3_CB 							{PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb}
#define CH4_CB 							{PWM_OUTPUT_DISABLED, NULL}
#endif

//#define PORTABLE_ICU_LINE 				LINE_ARD_D1 //PC6-In conflict with LoRa UART LINE_ARD_D1
#define PORTABLE_ICU_LINE 				LINE_ARD_A0 //PA6
#define PORTABLE_ICUD 					ICUD3
#define PORTABLE_ICU_AF  				2
#define PORTABLE_ICU_CHANNEL 			ICU_CHANNEL_1

#define LINE_LED_RED            		LINE_LED1_RED
#define LINE_LED_GREEN          		LINE_LED2_GREEN

//This is the receiver/client
#define S4E_USE_EBYTE_LORA              0
#define EBYTE_LORA_SERVER               0
#define PORTAB_EBYTE_LORA_SD 			SD6

#define EBYTE_LORA_TX 	    			LINE_ARD_D1 //PG14
#define EBYTE_LORA_RX 	    			LINE_ARD_D0 //PG9
#define EBYTE_LORA_M0 	    			LINE_ARD_D4
#define EBYTE_LORA_M1 	    			LINE_ARD_D5
#define EBYTE_LORA_AUX 	    			LINE_ARD_D6

#define EBYTE_LORA_AF       			8


#define DEFAULT_OPTION_FIXED_TRANS		1
#if DEFAULT_OPTION_FIXED_TRANS != 0
#define DEFAULT_ADDRESS_HIGH			2
#define DEFAULT_ADDRESS_LOW				0
#define DEFAULT_CHANNEL					6
#endif

#define DEFAULT_TO_ADDRESS_HIGH			1
#define DEFAULT_TO_ADDRESS_LOW			0
#define DEFAULT_TO_CHANNE				5

#define EBYTE_LORA_HOST_ID              2
#define EBYTE_LORA_SAVE_PARAMS		    1
#define EBYTE_LORA_100MW

#define GO_TO_SLEEP_MACROS      		SCB->SCR 	|= SCB_SCR_SLEEPDEEP_Msk;\
										PWR->CR1  	|= (PWR_CR1_PDDS | PWR_CR1_LPDS | PWR_CR1_CSBF);\
										PWR->CR2  	|= (PWR_CR2_CWUPF1 | PWR_CR2_CWUPF6);\
										PWR->CSR2  	|= (PWR_CSR2_WUPF1 | PWR_CSR2_EWUP1);\
										RTC->ISR 	&= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF | RTC_ISR_WUTF | RTC_ISR_TAMP1F |\
														RTC_ISR_TSOVF | RTC_ISR_TSF);
#define RTC_ALARM_1_FLAGS2   			RTC_ALRM_MSK4  |\
										RTC_ALRM_MSK3  |\
										RTC_ALRM_MSK2  |\
										RTC_ALRM_ST(0) |\
										RTC_ALRM_SU(0)

#define RTC_ALARM_2_FLAGS2  			RTC_ALRM_MSK4  |\
										RTC_ALRM_MSK3  |\
										RTC_ALRM_MSK2  |\
										RTC_ALRM_ST(5) |\
										RTC_ALRM_SU(1)
#define WAKEUP_HARD_REST_CHECK          (uint32_t)(PWR->CSR1 & PWR_CSR1_SBF)
#define CLEAR_WAKEUP_FLAG			    PWR->CR1  	|= (PWR_CR1_CSBF)



#define BACKUP_CCM_RAM_SECTION 			ram5

#define S4E_USE_IR_RECEIVER				1
#endif /* CFG_STRUST4EMBEDDED_H_ */
