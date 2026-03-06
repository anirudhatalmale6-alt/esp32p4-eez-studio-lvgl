
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_idf_version.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_lcd_jd9365.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_vendor.h"

// EEZ Studio generated UI (from main/eez_ui/)
#include "ui.h"

static const char *TAG = "jd9365_app";

// Resolution and parameters: 800x1280, 24bpp RGB888, 2 lane DSI
#define PANEL_MIPI_DSI_DPI_CLK_MHZ 35
#define PANEL_MIPI_DSI_LCD_H_RES   800
#define PANEL_MIPI_DSI_LCD_V_RES   1280
#define PANEL_MIPI_DSI_LCD_HSYNC   20
#define PANEL_MIPI_DSI_LCD_HBP     20
#define PANEL_MIPI_DSI_LCD_HFP     20
#define PANEL_MIPI_DSI_LCD_VSYNC   2
#define PANEL_MIPI_DSI_LCD_VBP     14
#define PANEL_MIPI_DSI_LCD_VFP     22

#define BSP_LCD_DRAW_BUFF_SIZE 400*640
#define BSP_LCD_DRAW_BUFF_DOUBLE 0

// Display settings
#define LCD_BIT_PER_PIXEL       (16)
#define PIN_LCD_RST             (16)
#define PIN_BK_LIGHT            (-1)
#define MIPI_DSI_LANE_NUM       (2)
#define MIPI_DSI_PHY_LDO_CHAN   (3)
#define MIPI_DSI_PHY_LDO_MV    (2500)

// Touch settings
#define TOUCH_I2C_NUM           (0)
#define TOUCH_I2C_CLK_HZ        (10000)
#define TOUCH_I2C_SCL           (GPIO_NUM_8)
#define TOUCH_I2C_SDA           (GPIO_NUM_7)
#define TOUCH_I2C_RST           (-1)

// Touch handles
static i2c_master_bus_handle_t i2c_handle = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

#if LCD_BIT_PER_PIXEL == 24
#define MIPI_DPI_PX_FORMAT      (LCD_COLOR_PIXEL_FORMAT_RGB888)
#elif LCD_BIT_PER_PIXEL == 18
#define MIPI_DPI_PX_FORMAT      (LCD_COLOR_PIXEL_FORMAT_RGB666)
#elif LCD_BIT_PER_PIXEL == 16
#define MIPI_DPI_PX_FORMAT      (LCD_COLOR_PIXEL_FORMAT_RGB565)
#endif

// CH700WX09A-JD9365DA 800x1280 2 lane custom initialization table
// Format: { cmd, (uint8_t[]){ data }, data_bytes, delay_ms }
static const jd9365_lcd_init_cmd_t lcd_init_cmds_ch700wx09a[] = {
    {0xE1, (uint8_t[]){0x93}, 1, 0},
    {0xE2, (uint8_t[]){0x65}, 1, 0},
    {0xE3, (uint8_t[]){0xF8}, 1, 0},
    {0x80, (uint8_t[]){0x01}, 1, 0},    // 01: 2 lane, 03: 4 lane

    {0xE0, (uint8_t[]){0x01}, 1, 0},
    {0x00, (uint8_t[]){0x00}, 1, 0},
    {0x01, (uint8_t[]){0x67}, 1, 0},
    {0x03, (uint8_t[]){0x10}, 1, 0},
    {0x04, (uint8_t[]){0x6F}, 1, 0},

    {0x0C, (uint8_t[]){0x74}, 1, 0},

    {0x17, (uint8_t[]){0x00}, 1, 0},
    {0x18, (uint8_t[]){0xD7}, 1, 0},
    {0x19, (uint8_t[]){0x01}, 1, 0},
    {0x1A, (uint8_t[]){0x00}, 1, 0},
    {0x1B, (uint8_t[]){0xD7}, 1, 0},
    {0x1C, (uint8_t[]){0x01}, 1, 0},

    {0x24, (uint8_t[]){0xFE}, 1, 0},

    {0x37, (uint8_t[]){0x09}, 1, 0},

    {0x38, (uint8_t[]){0x04}, 1, 0},
    {0x39, (uint8_t[]){0x08}, 1, 0},
    {0x3A, (uint8_t[]){0x12}, 1, 0},
    {0x3C, (uint8_t[]){0x73}, 1, 0},
    {0x3D, (uint8_t[]){0xFF}, 1, 0},
    {0x3E, (uint8_t[]){0xFF}, 1, 0},
    {0x3F, (uint8_t[]){0x7F}, 1, 0},

    {0x40, (uint8_t[]){0x06}, 1, 0},
    {0x41, (uint8_t[]){0xA0}, 1, 0},
    {0x42, (uint8_t[]){0x7E}, 1, 0},
    {0x43, (uint8_t[]){0x16}, 1, 0},
    {0x44, (uint8_t[]){0x0F}, 1, 0},
    {0x45, (uint8_t[]){0x28}, 1, 0},

    {0x55, (uint8_t[]){0x02}, 1, 0},
    {0x57, (uint8_t[]){0x69}, 1, 0},
    {0x59, (uint8_t[]){0x0A}, 1, 0},
    {0x5A, (uint8_t[]){0x1A}, 1, 0},
    {0x5B, (uint8_t[]){0x1A}, 1, 0},

    {0x5D, (uint8_t[]){0x70}, 1, 0},
    {0x5E, (uint8_t[]){0x54}, 1, 0},
    {0x5F, (uint8_t[]){0x43}, 1, 0},
    {0x60, (uint8_t[]){0x36}, 1, 0},
    {0x61, (uint8_t[]){0x31}, 1, 0},
    {0x62, (uint8_t[]){0x23}, 1, 0},
    {0x63, (uint8_t[]){0x25}, 1, 0},
    {0x64, (uint8_t[]){0x10}, 1, 0},
    {0x65, (uint8_t[]){0x29}, 1, 0},
    {0x66, (uint8_t[]){0x28}, 1, 0},
    {0x67, (uint8_t[]){0x28}, 1, 0},
    {0x68, (uint8_t[]){0x48}, 1, 0},
    {0x69, (uint8_t[]){0x38}, 1, 0},
    {0x6A, (uint8_t[]){0x3F}, 1, 0},
    {0x6B, (uint8_t[]){0x31}, 1, 0},
    {0x6C, (uint8_t[]){0x2C}, 1, 0},
    {0x6D, (uint8_t[]){0x1F}, 1, 0},
    {0x6E, (uint8_t[]){0x0E}, 1, 0},
    {0x6F, (uint8_t[]){0x02}, 1, 0},

    {0x70, (uint8_t[]){0x70}, 1, 0},
    {0x71, (uint8_t[]){0x54}, 1, 0},
    {0x72, (uint8_t[]){0x43}, 1, 0},
    {0x73, (uint8_t[]){0x36}, 1, 0},
    {0x74, (uint8_t[]){0x31}, 1, 0},
    {0x75, (uint8_t[]){0x23}, 1, 0},
    {0x76, (uint8_t[]){0x25}, 1, 0},
    {0x77, (uint8_t[]){0x10}, 1, 0},
    {0x78, (uint8_t[]){0x29}, 1, 0},
    {0x79, (uint8_t[]){0x28}, 1, 0},
    {0x7A, (uint8_t[]){0x28}, 1, 0},
    {0x7B, (uint8_t[]){0x48}, 1, 0},
    {0x7C, (uint8_t[]){0x38}, 1, 0},
    {0x7D, (uint8_t[]){0x3F}, 1, 0},
    {0x7E, (uint8_t[]){0x31}, 1, 0},
    {0x7F, (uint8_t[]){0x2C}, 1, 0},
    {0x80, (uint8_t[]){0x1F}, 1, 0},
    {0x81, (uint8_t[]){0x0E}, 1, 0},
    {0x82, (uint8_t[]){0x02}, 1, 0},

    {0xE0, (uint8_t[]){0x02}, 1, 0},

    {0x00, (uint8_t[]){0x5F}, 1, 0},
    {0x01, (uint8_t[]){0x50}, 1, 0},
    {0x02, (uint8_t[]){0x48}, 1, 0},
    {0x03, (uint8_t[]){0x4A}, 1, 0},
    {0x04, (uint8_t[]){0x44}, 1, 0},
    {0x05, (uint8_t[]){0x46}, 1, 0},
    {0x06, (uint8_t[]){0x75}, 1, 0},
    {0x07, (uint8_t[]){0x5E}, 1, 0},
    {0x08, (uint8_t[]){0x5E}, 1, 0},
    {0x09, (uint8_t[]){0x5F}, 1, 0},
    {0x0A, (uint8_t[]){0x5F}, 1, 0},
    {0x0B, (uint8_t[]){0x5F}, 1, 0},
    {0x0C, (uint8_t[]){0x5F}, 1, 0},
    {0x0D, (uint8_t[]){0x5F}, 1, 0},
    {0x0E, (uint8_t[]){0x5F}, 1, 0},
    {0x0F, (uint8_t[]){0x55}, 1, 0},
    {0x10, (uint8_t[]){0x55}, 1, 0},
    {0x11, (uint8_t[]){0x40}, 1, 0},
    {0x12, (uint8_t[]){0x40}, 1, 0},
    {0x13, (uint8_t[]){0x42}, 1, 0},
    {0x14, (uint8_t[]){0x42}, 1, 0},
    {0x15, (uint8_t[]){0x5F}, 1, 0},

    {0x16, (uint8_t[]){0x5F}, 1, 0},
    {0x17, (uint8_t[]){0x50}, 1, 0},
    {0x18, (uint8_t[]){0x49}, 1, 0},
    {0x19, (uint8_t[]){0x4B}, 1, 0},
    {0x1A, (uint8_t[]){0x45}, 1, 0},
    {0x1B, (uint8_t[]){0x47}, 1, 0},
    {0x1C, (uint8_t[]){0x75}, 1, 0},
    {0x1D, (uint8_t[]){0x5E}, 1, 0},
    {0x1E, (uint8_t[]){0x5E}, 1, 0},
    {0x1F, (uint8_t[]){0x5F}, 1, 0},
    {0x20, (uint8_t[]){0x5F}, 1, 0},
    {0x21, (uint8_t[]){0x5F}, 1, 0},
    {0x22, (uint8_t[]){0x5F}, 1, 0},
    {0x23, (uint8_t[]){0x5F}, 1, 0},
    {0x24, (uint8_t[]){0x5F}, 1, 0},
    {0x25, (uint8_t[]){0x55}, 1, 0},
    {0x26, (uint8_t[]){0x55}, 1, 0},
    {0x27, (uint8_t[]){0x41}, 1, 0},
    {0x28, (uint8_t[]){0x41}, 1, 0},
    {0x29, (uint8_t[]){0x43}, 1, 0},
    {0x2A, (uint8_t[]){0x43}, 1, 0},
    {0x2B, (uint8_t[]){0x5F}, 1, 0},

    {0x2C, (uint8_t[]){0x1F}, 1, 0},
    {0x2D, (uint8_t[]){0x10}, 1, 0},
    {0x2E, (uint8_t[]){0x07}, 1, 0},
    {0x2F, (uint8_t[]){0x05}, 1, 0},
    {0x30, (uint8_t[]){0x0B}, 1, 0},
    {0x31, (uint8_t[]){0x09}, 1, 0},
    {0x32, (uint8_t[]){0x35}, 1, 0},
    {0x33, (uint8_t[]){0x1F}, 1, 0},
    {0x34, (uint8_t[]){0x1F}, 1, 0},
    {0x35, (uint8_t[]){0x1F}, 1, 0},
    {0x36, (uint8_t[]){0x1F}, 1, 0},
    {0x37, (uint8_t[]){0x1E}, 1, 0},
    {0x38, (uint8_t[]){0x1E}, 1, 0},
    {0x39, (uint8_t[]){0x1F}, 1, 0},
    {0x3A, (uint8_t[]){0x1F}, 1, 0},
    {0x3B, (uint8_t[]){0x15}, 1, 0},
    {0x3C, (uint8_t[]){0x15}, 1, 0},
    {0x3D, (uint8_t[]){0x03}, 1, 0},
    {0x3E, (uint8_t[]){0x03}, 1, 0},
    {0x3F, (uint8_t[]){0x01}, 1, 0},
    {0x40, (uint8_t[]){0x01}, 1, 0},
    {0x41, (uint8_t[]){0x1F}, 1, 0},

    {0x42, (uint8_t[]){0x1F}, 1, 0},
    {0x43, (uint8_t[]){0x10}, 1, 0},
    {0x44, (uint8_t[]){0x06}, 1, 0},
    {0x45, (uint8_t[]){0x04}, 1, 0},
    {0x46, (uint8_t[]){0x0A}, 1, 0},
    {0x47, (uint8_t[]){0x08}, 1, 0},
    {0x48, (uint8_t[]){0x35}, 1, 0},
    {0x49, (uint8_t[]){0x1F}, 1, 0},
    {0x4A, (uint8_t[]){0x1F}, 1, 0},
    {0x4B, (uint8_t[]){0x1F}, 1, 0},
    {0x4C, (uint8_t[]){0x1F}, 1, 0},
    {0x4D, (uint8_t[]){0x1E}, 1, 0},
    {0x4E, (uint8_t[]){0x1E}, 1, 0},
    {0x4F, (uint8_t[]){0x1F}, 1, 0},
    {0x50, (uint8_t[]){0x1F}, 1, 0},
    {0x51, (uint8_t[]){0x15}, 1, 0},
    {0x52, (uint8_t[]){0x15}, 1, 0},
    {0x53, (uint8_t[]){0x02}, 1, 0},
    {0x54, (uint8_t[]){0x02}, 1, 0},
    {0x55, (uint8_t[]){0x00}, 1, 0},
    {0x56, (uint8_t[]){0x00}, 1, 0},
    {0x57, (uint8_t[]){0x1F}, 1, 0},

    {0x58, (uint8_t[]){0x40}, 1, 0},
    {0x5B, (uint8_t[]){0x30}, 1, 0},
    {0x5C, (uint8_t[]){0x05}, 1, 0},
    {0x5D, (uint8_t[]){0x30}, 1, 0},
    {0x5E, (uint8_t[]){0x01}, 1, 0},
    {0x5F, (uint8_t[]){0x02}, 1, 0},
    {0x60, (uint8_t[]){0x30}, 1, 0},
    {0x61, (uint8_t[]){0x01}, 1, 0},
    {0x62, (uint8_t[]){0x02}, 1, 0},
    {0x63, (uint8_t[]){0x03}, 1, 0},
    {0x64, (uint8_t[]){0x6A}, 1, 0},
    {0x65, (uint8_t[]){0x45}, 1, 0},
    {0x66, (uint8_t[]){0x14}, 1, 0},
    {0x67, (uint8_t[]){0x73}, 1, 0},
    {0x68, (uint8_t[]){0x09}, 1, 0},
    {0x69, (uint8_t[]){0x03}, 1, 0},
    {0x6A, (uint8_t[]){0x6A}, 1, 0},
    {0x6B, (uint8_t[]){0x08}, 1, 0},
    {0x6C, (uint8_t[]){0x00}, 1, 0},
    {0x6D, (uint8_t[]){0x04}, 1, 0},
    {0x6E, (uint8_t[]){0x00}, 1, 0},
    {0x6F, (uint8_t[]){0x88}, 1, 0},
    {0x77, (uint8_t[]){0xD5}, 1, 0},
    {0x79, (uint8_t[]){0x1A}, 1, 0},
    {0x7A, (uint8_t[]){0x00}, 1, 0},
    {0x7D, (uint8_t[]){0x03}, 1, 0},
    {0x7E, (uint8_t[]){0x6A}, 1, 0},

    {0xE0, (uint8_t[]){0x04}, 1, 0},
    {0x00, (uint8_t[]){0x0E}, 1, 0},
    {0x02, (uint8_t[]){0xB3}, 1, 0},
    {0x09, (uint8_t[]){0x60}, 1, 0},
    {0x0E, (uint8_t[]){0x4A}, 1, 0},

    {0xE0, (uint8_t[]){0x00}, 1, 0},

    {0x11, (uint8_t[]){0x00}, 1, 120},  // Sleep Out + 120ms
    {0x29, (uint8_t[]){0x00}, 1, 5},    // Display On + 5ms
};

static esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_dsi_bus_handle_t mipi_dsi_bus = NULL;
static esp_lcd_panel_io_handle_t mipi_dbi_io = NULL;

static esp_err_t init_lcd(void)
{
#if PIN_BK_LIGHT >= 0
    ESP_LOGI(TAG, "Backlight GPIO init");
    gpio_config_t bk_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_BK_LIGHT,
        .pull_up_en = false,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bk_cfg);
    gpio_set_level(PIN_BK_LIGHT, 1);
#endif

    ESP_LOGI(TAG, "MIPI DSI PHY power on (LDO)");
    esp_ldo_channel_config_t ldo_cfg = {
        .chan_id = MIPI_DSI_PHY_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_LDO_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_cfg, &ldo_mipi_phy));

    ESP_LOGI(TAG, "Init MIPI DSI bus");
    esp_lcd_dsi_bus_config_t bus_config = JD9365_PANEL_BUS_DSI_2CH_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&bus_config, &mipi_dsi_bus));

    ESP_LOGI(TAG, "Install Panel IO (DBI)");
    esp_lcd_dbi_io_config_t dbi_config = JD9365_PANEL_IO_DBI_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(mipi_dsi_bus, &dbi_config, &mipi_dbi_io));

    ESP_LOGI(TAG, "Install JD9365 driver");
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    esp_lcd_dpi_panel_config_t dpi_config = JD9365_800_1280_PANEL_60HZ_DPI_CONFIG_CF(MIPI_DPI_PX_FORMAT);
#else
    esp_lcd_dpi_panel_config_t dpi_config = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = PANEL_MIPI_DSI_DPI_CLK_MHZ,
        .pixel_format = MIPI_DPI_PX_FORMAT,
        .video_timing =
            {
                .h_size = PANEL_MIPI_DSI_LCD_H_RES,
                .v_size = PANEL_MIPI_DSI_LCD_V_RES,
                .hsync_back_porch = PANEL_MIPI_DSI_LCD_HBP,
                .hsync_pulse_width = PANEL_MIPI_DSI_LCD_HSYNC,
                .hsync_front_porch = PANEL_MIPI_DSI_LCD_HFP,
                .vsync_back_porch = PANEL_MIPI_DSI_LCD_VBP,
                .vsync_pulse_width = PANEL_MIPI_DSI_LCD_VSYNC,
                .vsync_front_porch = PANEL_MIPI_DSI_LCD_VFP,
            },
        .flags.use_dma2d = true,
        .num_fbs = 1,
    };
#endif
    jd9365_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds_ch700wx09a,
        .init_cmds_size = sizeof(lcd_init_cmds_ch700wx09a) / sizeof(jd9365_lcd_init_cmd_t),
        .mipi_config = {
            .dsi_bus = mipi_dsi_bus,
            .dpi_config = &dpi_config,
            .lane_num = MIPI_DSI_LANE_NUM,
        },
    };
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(mipi_dbi_io, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    return ESP_OK;
}

static esp_err_t app_touch_init(void)
{
    // Initialize I2C
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = 1,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = 0,
    };
    esp_err_t ret = i2c_new_master_bus(&i2c_config, &i2c_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize GT911 touch
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = PANEL_MIPI_DSI_LCD_H_RES,
        .y_max = PANEL_MIPI_DSI_LCD_V_RES,
        .rst_gpio_num = TOUCH_I2C_RST,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 1,
            .mirror_y = 0,
        },
    };

    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.scl_speed_hz = TOUCH_I2C_CLK_HZ;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_handle, &tp_io_config, &tp_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, &touch_handle));

    return ESP_OK;
}

static lv_disp_t *bsp_display_lcd_init(void)
{
    app_touch_init();
    esp_err_t ret = init_lcd();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LCD panel: %s", esp_err_to_name(ret));
        return NULL;
    }

    esp_lcd_panel_disp_on_off(panel_handle, true);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = mipi_dbi_io,
        .panel_handle = panel_handle,
        .control_handle = NULL,
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .hres = PANEL_MIPI_DSI_LCD_H_RES,
        .vres = PANEL_MIPI_DSI_LCD_V_RES,
        .monochrome = false,
        .rotation =
            {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
        .flags = {
            .buff_dma = 0,
            .sw_rotate = 1,
            .buff_spiram = 1,
        }};

    const lvgl_port_display_dsi_cfg_t dsi_cfg = {
        .flags = {
            .avoid_tearing = false,
        }
    };

    lv_disp_t *disp = lvgl_port_add_disp_dsi(&disp_cfg, &dsi_cfg);

    // Add touch input to LVGL
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return disp;
}

lv_disp_t *bsp_display_start(void)
{
    lv_disp_t *disp = NULL;
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 10,
        .task_stack = 14336,
        .task_affinity = -1,
        .task_max_sleep_ms = 30,
        .timer_period_ms = 5,
    };
    lvgl_port_init(&lvgl_cfg);
    disp = bsp_display_lcd_init();

    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    return disp;
}

// ============================================================================
// EEZ Flow tick task
// ============================================================================
// EEZ Flow requires periodic ui_tick() calls to process flow engine updates,
// variable bindings, and screen transitions.
// ============================================================================
static void eez_ui_tick_task(void *arg)
{
    while (1) {
        // Lock LVGL before calling EEZ tick (thread safety)
        if (lvgl_port_lock(100)) {
            ui_tick();
            lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-P4 + EEZ Studio LVGL Flow Demo");
    ESP_LOGI(TAG, "Display: JD9365 800x1280, 60Hz, MIPI DSI 2-lane");

    bsp_display_start();

    // Initialize EEZ Studio generated UI
    // Lock LVGL mutex before calling ui_init (creates LVGL objects)
    if (lvgl_port_lock(1000)) {
        ui_init();
        lvgl_port_unlock();
    }

    // Start EEZ Flow tick task (processes flow engine, variable updates)
    xTaskCreate(eez_ui_tick_task, "eez_tick", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "UI initialized, EEZ Flow engine running");
}
