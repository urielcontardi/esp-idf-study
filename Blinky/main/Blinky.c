/// \file		Blinky.c
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
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define BLINK_GPIO  4

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
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(blinkyTask, "blinkyTask", 2048, NULL, 5, NULL);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void _configureLed(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}