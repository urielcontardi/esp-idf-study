/// \file		main.cpp
///
/// \brief		Main file
///
/// \author		Uriel Abe Contardi (urielcontardi@hotmail.com)
/// \date		08-09-2024
///
/// \version	1.0
///
/// \note		Revisions:
/// 			08-09-2024 <urielcontardi@hotmail.com>
/// 			First revision.
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
#include "power_driver.h"
#include "demos/lv_demos.h"
#include "tft_driver.h"
#include "product_pins.h"

static const char *TAG = "main";

#define LVGL_TICK_PERIOD_MS 2
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 1
#define LVGL_TASK_STACK_SIZE (4 * 1024)
#define LVGL_TASK_PRIORITY 2

static SemaphoreHandle_t lvglMutex = NULL;

static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)

extern "C" {
    lv_disp_drv_t disp_drv;      // contains callback functions
}

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
static void lvglFlushCallback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    display_push_colors(offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, (uint16_t *)color_map);
}

static void lvglTick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
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

static void lvglTask(void *arg)
{
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;

    // Create a screen and a label
    lv_obj_t *screen = lv_scr_act();  // Get the active screen

    // Draw a rectangle
    lv_obj_t *rect = lv_obj_create(screen);  // Create a rectangle object
    lv_obj_set_size(rect, 100, 50);          // Set size (width x height)
    lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0); // Align to center of the screen
    lv_obj_set_style_bg_color(rect, LV_COLOR_MAKE(0, 0, 255), 0); // Set background color (red)

    // Draw some text
    lv_obj_t *label = lv_label_create(screen);  // Create a label object
    lv_label_set_text(label, "Ola Uriel!");   // Set the text
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60); // Align below the rectangle

    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        if (lvglLock(-1)) {
            task_delay_ms = lv_timer_handler();  // Process LVGL tasks
            lvglUnlock();              // Release the mutex
        }
        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));  // Delay to control task execution rate
    }
}

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "------ Initialize PMU.");
    if (!power_driver_init()) {
        ESP_LOGE(TAG, "ERROR :No find PMU ....");
    }

    ESP_LOGI(TAG, "------ Initialize DISPLAY.");
    display_init();


    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();


    // alloc draw buffers used by LVGL
    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = (lv_color_t *) heap_caps_malloc(AMOLED_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, AMOLED_HEIGHT * 20);


    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = AMOLED_HEIGHT;
    disp_drv.ver_res = AMOLED_WIDTH;
    disp_drv.flush_cb = lvglFlushCallback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.full_refresh = DISPLAY_FULLRESH;
    lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvglTick,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = false
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    lvglMutex = xSemaphoreCreateRecursiveMutex();
    assert(lvglMutex);

    // tskIDLE_PRIORITY,
    ESP_LOGI(TAG, "Create LVGL task");
    xTaskCreate(lvglTask, "LVGL", LVGL_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

}
