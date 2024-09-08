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
#include <mpu6050.h>

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define BLINK_GPIO  4

#define MPU6050_ADDR MPU6050_I2C_ADDRESS_LOW
#define I2C_SDA_GPIO 21
#define I2C_SCL_GPIO 22

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      LOCAL TYPEDEFS AND STRUCTURES                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

typedef struct {
    mpu6050_dev_t device;
    float temp;
    mpu6050_acceleration_t accel;
    mpu6050_rotation_t rotation;
} MPU6050_t;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                        LOCAL FUNCTIONS PROTOTYPES                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static void _configureLed(void);
static void _configureMPU6050(void);
static inline void _printHighWaterMark(const char * const task_name);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static const char *TAG = "AppBlinky";
static MPU6050_t mpu6050 = {0};

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

void mpu6050Task(void *pvParameters)
{
    _configureMPU6050();

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_ERROR_CHECK(mpu6050_get_temperature(&mpu6050.device, &mpu6050.temp));
        ESP_ERROR_CHECK(mpu6050_get_motion(&mpu6050.device, &mpu6050.accel, &mpu6050.rotation));

        ESP_LOGI(TAG, "**********************************************************************");
        ESP_LOGI(TAG, "Acceleration: x=%.4f   y=%.4f   z=%.4f", mpu6050.accel.x, mpu6050.accel.y, mpu6050.accel.z);
        ESP_LOGI(TAG, "Rotation:     x=%.4f   y=%.4f   z=%.4f", mpu6050.rotation.x, mpu6050.rotation.y, mpu6050.rotation.z);
        ESP_LOGI(TAG, "Temperature:  %.1f", mpu6050.temp);
    }
}

void app_main(void)
{
    xTaskCreate(blinkyTask, "blinkyTask", configMINIMAL_STACK_SIZE*2, NULL, 5, NULL);
    xTaskCreate(mpu6050Task, "mpu6050Task", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
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

void _configureMPU6050(void)
{
    ESP_ERROR_CHECK(i2cdev_init());
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_ERROR_CHECK(mpu6050_init_desc(&mpu6050.device, 
                                      MPU6050_ADDR, 
                                      0, 
                                      I2C_SDA_GPIO, 
                                      I2C_SCL_GPIO)
                                      );
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        esp_err_t res = i2c_dev_probe(&mpu6050.device.i2c_dev, I2C_DEV_WRITE);
        if (res == ESP_OK)
        {
            ESP_LOGI(TAG, "Found MPU60x0 device");
            break;
        }
        ESP_LOGE(TAG, "MPU60x0 not found");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(mpu6050_init(&mpu6050.device));

}

static inline void _printHighWaterMark(const char * const task_name)
{
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(task_name, "High Water Mark: %u\n", uxHighWaterMark);
}
