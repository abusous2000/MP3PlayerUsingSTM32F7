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

#ifndef S4E_USE_SDCARD
#define S4E_USE_SDCARD 			1
#endif

#ifndef USE_LCD_TFT
#define USE_LCD_TFT 			1
#endif

#ifndef S4E_USE_JOYSTICK
#define S4E_USE_JOYSTICK        0
#endif

#ifndef S4E_USE_BLINKER_THD
#define S4E_USE_BLINKER_THD     1
#endif

#ifndef S4E_USE_WIFI_MODULE_THD
#define S4E_USE_WIFI_MODULE_THD 1
#endif

#ifndef S4E_USE_SSD1306_LCD
#define S4E_USE_SSD1306_LCD     0
#endif

#ifndef S4E_USE_POT
#define S4E_USE_POT             0
#endif

#ifndef S4E_USE_PWM
#define S4E_USE_PWM             0
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
#endif /* CFG_STRUST4EMBEDDED_H_ */
