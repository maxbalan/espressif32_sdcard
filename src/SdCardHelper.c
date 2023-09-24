#include "SdCardHelper.h"

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED
#endif

#ifdef DEBUG_ENABLED
static const char *TAG = "SdCard >>> ";
#endif

// #define PIN_NUM_MISO 19
// #define PIN_NUM_MOSI 23
// #define PIN_NUM_CLK 18
// #define PIN_NUM_CS 5
// #define SDCARD_MAX_REQ_KHZ 8000

// static esp_err_t s_example_write_file(const char *path, char *data) {
//     ESP_LOGI(TAG, "Opening file %s", path);
//     FILE *f = fopen(path, "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return ESP_FAIL;
//     }
//     fprintf(f, data);
//     fclose(f);
//     ESP_LOGI(TAG, "File written");

//     return ESP_OK;
// }

SdCard mount_sdcard(sdcard_config config) {
    SdCard sd_card;
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = config.max_req_khz;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = config.pin_mode.mosi,
        .miso_io_num = config.pin_mode.miso,
        .sclk_io_num = config.pin_mode.clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        sd_card.err = true;
        return sd_card;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = config.pin_mode.cs;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(config.mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG,
                     "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG,
                     "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.",
                     esp_err_to_name(ret));
        }

        sd_card.err = true;
        return sd_card;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    sdmmc_card_print_info(stdout, card);

    sd_card.config = &config;
    sd_card.host = host;
    sd_card.card = card;
    sd_card.err = false;

    return sd_card;
}

void unmount_sdcard(SdCard *card) {
    esp_vfs_fat_sdcard_unmount(card->config->mount_point, card->card);
    ESP_LOGI(TAG, "Card unmounted");
    spi_bus_free(card->host.slot);
}

void sdcard_rename_file(SdCard *card, const char *file_path, const char *file_name, const char *new_file_name) {
    struct stat st;
    char *oldFilePath = malloc(strlen(file_name) + strlen(card->config->mount_point) + strlen(file_path));
    strcpy(oldFilePath, card->config->mount_point);
    strcat(oldFilePath, file_path);
    strcat(oldFilePath, file_name);

    char *newFilePath = malloc(strlen(new_file_name) + strlen(card->config->mount_point) + strlen(file_path));
    strcpy(newFilePath, card->config->mount_point);
    strcat(newFilePath, file_path);
    strcat(newFilePath, new_file_name);

    ESP_LOGI(TAG, "rename file[ %s ] to [ %s ]", oldFilePath, newFilePath);

    if (stat(oldFilePath, &st) == 0) {
        // Delete it if it exists
        ESP_LOGI(TAG, "Ping file found!");
    }

    // Rename original file
    if (rename(oldFilePath, newFilePath) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }
}