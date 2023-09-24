#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

typedef struct pin_config {
    int miso;
    int mosi;
    int clk;
    int cs;
} pin_config;

typedef struct {
    pin_config pin_mode;
    int max_req_khz;
    char mount_point[10];
} sdcard_config;

struct SdCard {
    void *data;
    sdmmc_card_t *card;
    sdmmc_host_t host;
    sdcard_config *config;
    bool err;
};

typedef struct SdCard SdCard;

SdCard mount_sdcard(sdcard_config sdcard_config);

void unmount_sdcard(SdCard *card);

void sdcard_rename_file(SdCard *card, const char* file_path, const char *file_name, const char* new_file_name);