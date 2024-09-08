idf_component_register(SRCS
    "main.c"
    "power_driver.c"
    "tft_driver.c"
    "displayHandler.c"
    "adcHandler.c"
    INCLUDE_DIRS
        "."  
        "${IDF_PATH}/components/esp_lcd/rgb/include"  # Diretório onde está esp_lcd_panel_rgb.h
    REQUIRES
        "esp_lcd" 

)