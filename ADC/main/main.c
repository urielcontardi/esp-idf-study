/// \file		main.c
///
/// \brief
///
/// \author		Uriel Abe Contardi (urielcontardi@hotmail.com)
/// \date		07-09-2024
///
/// \version	1.0
///
/// \note		Revisions:
/// 			07-09-2024 <urielcontardi@hotmail.com>
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
#include <esp_adc_cal.h>

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define BLINK_GPIO  4

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
static void _configureLed(void);
static void _configureADC(void);
static inline void _printHighWaterMark(const char * const task_name);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static const char *TAG = "AppBlinky";

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void blinkyTask(void *pvParameter)
{
    // Configure LED GPIO
    _configureLed();

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void micTask(void *pvParameter)
{
    _configureADC();

    while (1)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
        int micValue = adc1_get_raw(MIC_CHANNEL);
        //ESP_LOGI(TAG, "micValue: %d", micValue);
        printf("%d\n", micValue);
    }

}

void app_main(void)
{
    xTaskCreate(blinkyTask, "blinkyTask", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL);
    xTaskCreate(micTask, "micTask", configMINIMAL_STACK_SIZE*6, NULL, 5, NULL);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void _configureLed(void)
{
    ESP_LOGI(TAG, "Configured GPIO LED to blink!");
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void _configureADC(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MIC_CHANNEL, ADC_ATTEN_DB_11);
}

static inline void _printHighWaterMark(const char * const task_name)
{
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(task_name, "High Water Mark: %u\n", uxHighWaterMark);
}
