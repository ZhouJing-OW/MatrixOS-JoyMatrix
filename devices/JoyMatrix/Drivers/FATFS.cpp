#include "MatrixOS.h"
#include "Device.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"

#define MOUNT_PATH "/storage"

namespace Device::FATFS {

    void Init()
    {
        static wl_handle_t wl_handle;
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 4,
            .allocation_unit_size = 1024,
            .disk_status_check_enable = false
        };

        esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, "storage", &mount_config, &wl_handle);

        if (err != ESP_OK){
            MLOGI("FatFs", "Failed to mount FatFs (%s)", esp_err_to_name(err));
            return;
        } else {
            MLOGI("FatFs", "Succeeded to mount FatFs (%s)", esp_err_to_name(err));
            return;
        }
    }

    void Format()
    {   
        const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, "storage");
        esp_partition_erase_range(partition, 0, partition->size);
    }

}