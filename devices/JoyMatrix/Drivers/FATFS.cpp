#include "MatrixOS.h"
#include "Device.h"
#include "esp_vfs_fat.h"
#include "esp_partition.h"
#include <fstream>

#define MOUNT_PATH "/storage"
#define PART_LABLE "storage"

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

    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, PART_LABLE, &mount_config, &wl_handle);

    if (err != ESP_OK){
      MLOGE("FatFs", "Failed to mount FatFs (%s)", esp_err_to_name(err));
      return;
    } else {
      MLOGI("FatFs", "Succeeded to mount FatFs (%s)", esp_err_to_name(err));
      GetFreeSpace();
      return;
    }
  }

  uint64_t GetFreeSpace()
  {
    uint64_t total = 0, free = 0;
    esp_err_t err = esp_vfs_fat_info(MOUNT_PATH, &total, &free);
    if (err != ESP_OK) {
      MLOGE("FatFs", "Failed to get vfs FAT partition information (%s)", esp_err_to_name(err));
      return 0;
    }
    MLOGD("FatFs", "Total space: %llu, Free space: %llu", total, free);
    return free;
  }

  void Format()
  {   
    const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, PART_LABLE);
    esp_partition_erase_range(partition, 0, partition->size);
  }

}