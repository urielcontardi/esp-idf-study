/// \file		displayHandler.c
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "tft_driver.h"
#include "product_pins.h"

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                           DEFINES AND MACROS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

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

/**
 * @brief Task to update LVGL
 *
 * This task is responsible to periodically call lv_task_handler() to update
 * the LVGL library.
 *
 * @param arg A pointer to the lvglTask structure
 */
static void lvglTask(void *arg);

/**
 * @brief Flush the content of the internal graphic buffer(s) to the display
 *
 * This function is a callback function of LVGL that will be called when the
 * content of the internal graphic buffer(s) needs to be flushed to the display.
 * It is called by LVGL when the content of the internal graphic buffer(s) is
 * changed and the buffer(s) needs to be flushed to the display.
 *
 * @param drv The driver structure
 * @param area The area of the internal graphic buffer(s) that needs to be
 *             flushed
 * @param color_map The color map of the internal graphic buffer(s)
 *
 */
static void _lvglFlushCallback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

/**
 * @brief Tell LVGL how many milliseconds has elapsed
 *
 * This function is a callback function of LVGL that will be called every
 * LVGL_TICK_PERIOD_MS milliseconds.
 *
 * @param arg
 */
static void _lvglTick(void *arg);

static void _configureLabel(void);

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                      STATIC VARIABLES AND CONSTANTS                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

static const char *TAG = "lvgl";

// Mutex for lvgl
static SemaphoreHandle_t lvglMutex = NULL;
static SemaphoreHandle_t adcDataMutex;

// Contains callback functions
lv_disp_drv_t disp_drv;

// Contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_draw_buf_t disp_buf;

// Plot Data
static int adcData;
static lv_obj_t *labelPlot;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            EXPORTED FUNCTIONS                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

void displayHandlerInit(void)
{
    ESP_LOGI(TAG, "------ Initialize DISPLAY ------ ");
    display_init();

    ESP_LOGI(TAG, "------ Initialize LVGL library ------ ");
    lv_init();

    // Alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = (lv_color_t *) heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    assert(buf2);

    // Display Buffer Initialization
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, AMOLED_HEIGHT * 20);

    // Display Driver Initizalization
    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = AMOLED_HEIGHT;
    disp_drv.ver_res = AMOLED_WIDTH;
    disp_drv.flush_cb = _lvglFlushCallback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.full_refresh = DISPLAY_FULLRESH;
    lv_disp_drv_register(&disp_drv);

    // Timer initialization
    // Create a timer to periodically call lv_tick_inc
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &_lvglTick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = false
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    // Mutex for lvgl
    lvglMutex = xSemaphoreCreateRecursiveMutex();
    assert(lvglMutex);

    // Mutex for adc data
    adcDataMutex = xSemaphoreCreateMutex();
    assert(adcDataMutex);

    // Task Creation
    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvglTask, "LVGL", LVGL_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
}

bool lvglLock(int timeout_ms)
{
    // Convert timeout in milliseconds to FreeRTOS ticks
    // If `timeout_ms` is set to -1, the program will block until the condition is met
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvglMutex, timeout_ticks) == pdTRUE;
}

void lvglUnlock(void)
{
    xSemaphoreGiveRecursive(lvglMutex);
}

void displayHandlerUpdateData(int value) 
{
    xSemaphoreTake(adcDataMutex, portMAX_DELAY);
    adcData = value;
    xSemaphoreGive(adcDataMutex);
}

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                              LOCAL FUNCTIONS                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void lvglTask(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    _configureLabel();

    // Update label
    char buf[32];

    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (lvglLock(-1)) {
            lv_timer_handler();     // Process LVGL tasks

            xSemaphoreTake(adcDataMutex, portMAX_DELAY);
            snprintf(buf, sizeof(buf), "ADC Value: %d", adcData);
            xSemaphoreGive(adcDataMutex);

            lv_label_set_text(labelPlot, buf);

            lvglUnlock();           // Release the mutex
        }

        vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_MAX_DELAY_MS));
    }
}

void _configureLabel(void)
{
    // Get current screen
    lv_obj_t *screen = lv_scr_act();  

    // Configure label
    labelPlot = lv_label_create(screen);
    lv_obj_align(labelPlot, LV_ALIGN_CENTER, 0, 0); 
    
}

void _lvglFlushCallback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    display_push_colors(offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, (uint16_t *)color_map);
}

void _lvglTick(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
