idf_component_register(SRCS
    "main.cpp"
    "power_driver.cpp"
    "tft_driver.c"
    INCLUDE_DIRS
        "."  
        "${IDF_PATH}/components/esp_lcd/rgb/include"  # Diretório onde está esp_lcd_panel_rgb.h
    REQUIRES
        esp_lcd 
)