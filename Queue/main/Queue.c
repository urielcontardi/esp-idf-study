/// \file		Queue.c
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

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define QUEUE_LENGTH 5    // Tamanho da queue
#define QUEUE_ITEM_SIZE sizeof(int)  // Tamanho de cada item na queue (um inteiro)

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
static void senderTask(void *arg);
static void receiverTask(void *arg);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static const char *TAG = "main";
static TaskHandle_t senderTaskHandle = NULL;
static TaskHandle_t receiverTaskHandle = NULL;
static QueueHandle_t queue;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void app_main()
{
    // Create Queue
    queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    if (queue == NULL)
        ESP_LOGE(TAG, "Falha ao criar a queue");

    // Create tasks
    xTaskCreate(senderTask, "senderTask", 4096, NULL, 10, &senderTaskHandle);
    xTaskCreatePinnedToCore(receiverTask, "receiverTask", 4096, NULL, 10, &receiverTaskHandle, 1);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void senderTask(void *arg)
{
    // Variables
    int counter = 0;

    while (1)
    {
        // Increments
        counter++;
        
        // Envia o valor para a queue
        ESP_LOGI(TAG, "Enviando: %d", counter);
        if (xQueueSend(queue, &counter, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Falha ao enviar para a queue");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

void receiverTask(void *arg)
{
    // Variables
    int rcvValue;

    while (1)
    {
        // Wait for queue
        if (xQueueReceive(queue, &rcvValue, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "Recebido: %d", rcvValue);
        } else {
            ESP_LOGE(TAG, "Falha ao receber da queue");
        }
    }
}