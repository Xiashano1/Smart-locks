idf_component_register(SRCS "lcd.c" "GUI.c" "touch.c" "main.c" "test.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio esp_driver_spi nvs_flash)

# 解决旧代码中结构体初始化、类型转换和绝对值计算的编译器警告
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=unterminated-string-initialization
    -Wno-error=missing-braces
    -Wno-error=pointer-sign
    -Wno-error=absolute-value
)