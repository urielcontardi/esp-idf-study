/// \file		adcHandler.c
///
/// \brief	
///
/// \author		Uriel Abe Contardi (urielcontardi@hotmail.com)
/// \date		08-09-2024
///
/// \version	1.0
///
/// \note		Revisions:
/// 			08-09-2024 <urielcontardi@hotmail.com>
/// 			First revision.
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                               INCLUDES                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/adc.h>
#include "displayHandler.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// ADC1 channel 4 is GPIO32 
#define MIC_CHANNEL    ADC1_CHANNEL_4

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      LOCAL TYPEDEFS AND STRUCTURES                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                        LOCAL FUNCTIONS PROTOTYPES                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static void _configureADC(void);
static void micTask(void *pvParameter);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static const char *TAG = "ADC";

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void adcHandlerInit(void)
{
    ESP_LOGI(TAG, "Initializing ADC Handler");
    xTaskCreate(micTask, "micTask", configMINIMAL_STACK_SIZE*6, NULL, 5, NULL);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void _configureADC(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MIC_CHANNEL, ADC_ATTEN_DB_11);
}

void micTask(void *pvParameter)
{
    _configureADC();

    while (1)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        int micValue = adc1_get_raw(MIC_CHANNEL);
        displayHandlerUpdateData(micValue);
        printf("%d\n", micValue);
    }

}