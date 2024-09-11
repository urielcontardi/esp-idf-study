/// \file		main.c
///
/// \brief	
///
/// \author		Uriel Abe Contardi (urielcontardi@hotmail.com)
/// \date		10-09-2024
///
/// \version	1.0
///
/// \note		Revisions:
/// 			10-09-2024 <urielcontardi@hotmail.com>
/// 			First revision.
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                               INCLUDES                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define BUTTON_PIN GPIO_NUM_0
#define DEBOUNCE_TIME_MS 50

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
static void IRAM_ATTR buttonISRHandler(void* arg);
static void buttonTask(void* arg);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static TaskHandle_t buttonTaskHandle = NULL;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void app_main() {
    
    // Install GPIO interrupt service
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    // Create Button task
    xTaskCreate(buttonTask, "buttonTask", 2048, NULL, 10, &buttonTaskHandle);

    // Install interrupt service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, buttonISRHandler, NULL);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

static void IRAM_ATTR buttonISRHandler(void* arg) 
{
    // Disable interrupt
    gpio_intr_disable(BUTTON_PIN);

    // Notify task
    xTaskNotifyFromISR(buttonTaskHandle, 0, eNoAction, NULL);
}

void buttonTask(void* arg) 
{
    while (1) {
        // Wait for Notify from Interrupt
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Print
        ESP_LOGI("BUTTON", "Botão pressionado! GPIO: %d", BUTTON_PIN);

        // Wait for debounce time and enable interrupt
        //vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
        gpio_intr_enable(BUTTON_PIN);
    }
}