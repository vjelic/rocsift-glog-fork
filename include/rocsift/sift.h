/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_H_
#define _SIFT_H_

#include "sys/types.h"
#include "stddef.h"
#include "stdint.h"
#include "rocsift/status.h"
#include "rocsift/visibility.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct sift_device_s {
  uint64_t handle;
} sift_device_t;

typedef struct sift_partition_s {
  uint64_t handle;
} sift_partition_t;

/// @brief device attributes
typedef enum {
  SIFT_DEVICE_INFO_NAME_SIZE = 0,
  SIFT_DEVICE_INFO_NAME = 1,
  SIFT_DEVICE_INFO_FWINFO_SIZE = 2,
  SIFT_DEVICE_INFO_FWINFO = 3,
} sift_device_info_t;

/// @brief accessor aperture
typedef enum {
  SIFT_APER_SMN = 0,
  SIFT_APER_MMIO = 1,
} sift_aperture_t;

typedef enum {
  SIFT_LINEAR_VIDEO_MEM = 0,
  SIFT_LINEAR_SYSTEM_MEM = 1,
} sift_mem_region_t;

///! @cond
typedef struct sift_kfd_node_s sift_kfd_node_t;
///! @endcond

/// @brief initializes the sift runtime
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_init();

/// @brief shuts down the sift runtime
/// @retval status
ROCSIFT_EXPORT sift_status_t sift_destroy();

/// @brief fetches the number of devices discovered
///
/// @param n Pointer to integer to fill with number of devices
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_get_device_count(int *n);

/// @brief gets device handle from index
///
/// @param n index of the device to retrieve
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_get_device(int n, sift_device_t *dev);

/// @brief fetches the number of devices discovered
///
/// @param n Pointer to integer to fill with number of partitions
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_device_get_partition_count(sift_device_t dev, int *n);

/// @brief gets device handle from index
///
/// @param n index of the partition to retrieve
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_device_get_partition(sift_device_t dev, sift_partition_t *part,
                                                       int n);

/// @brief gets a KFD node handle from a partition
///
/// @param part partition handle
/// @param node pointer to kfd node handle to fill in
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_get_kfd_node(sift_partition_t part,
                                                         sift_kfd_node_t *node);

/// @brief reads a 32-bit MMIO register
///
/// @param dev device handle
/// @param ap aperture
/// @param reg address of register to read
/// @param value pointer to memory to populate with the value read
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_device_read_reg32(sift_device_t dev, uint32_t *value,
                                                    sift_aperture_t ap, uint64_t reg);

/// @brief writes a 32-bit MMIO register
///
/// @param dev device handle
/// @param ap aperture
/// @param reg address of register to read
/// @param value to write to the register
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_device_write_reg32(sift_device_t dev, uint32_t value,
                                                     sift_aperture_t ap, uint64_t reg);

/// @brief Reads up to size bytes from addr
///
/// @param dev device handle
/// @param region memory region
/// @param addr address to target
/// @param data buffer to place read data
/// @param size number of bytes to read
/// @returns up to size on success, -1 on error
ROCSIFT_EXPORT ssize_t sift_device_read(sift_device_t dev, void *data, sift_mem_region_t region,
                                        uint64_t addr, size_t size);

/// @brief Writes up to size bytes to addr
///
/// @param dev device handle
/// @param region memory region
/// @param addr address to target
/// @param data buffer of data to write
/// @param size number of bytes to write
/// @returns up to size on success, -1 on error
ROCSIFT_EXPORT ssize_t sift_device_write(sift_device_t dev, const void *data,
                                         sift_mem_region_t region, uint64_t addr, size_t size);

/// @brief fetches the number of device partitions globally discovered
///
/// @param n Pointer to integer to fill with number of partitions
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_get_partition_count(int *n);

/// @brief gets partition handle from index
///
/// @param n index of the partition to retrieve
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_get_partition(int n, sift_partition_t *part);

/// @brief gets device handle from partition
///
/// @param dev pointer to device handle to populate
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_get_device(sift_partition_t part, sift_device_t *dev);

/// @brief fetches the number of xccs in a partition
///
/// @param part partition handle
/// @param num_xcc pointer to integer to fill with number of xccs
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_get_xcc_count(sift_partition_t part, int *num_xcc);

/// @#brief fetches the xcc ids in a partition
///
/// @param part partition handle
/// @param xcc_ids pointer to integer array to fill with xcc ids
/// @param num_xcc_ids number of xcc ids to fetch
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_get_xcc_die_ids(sift_partition_t part, int *xcc_ids,
                                                            int num_xcc_ids);

/// @brief reads a 32-bit MMIO register
///
/// @param part partition handle
/// @param ap aperture
/// @param reg address of register to read
/// @param value pointer to memory to populate with the value read
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_read_reg32(sift_partition_t part, uint32_t *value,
                                                       sift_aperture_t ap, uint64_t reg);

/// @brief writes a 32-bit MMIO register
///
/// @param part partition handle
/// @param ap aperture
/// @param reg address of register to read
/// @param value to write to the register
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_write_reg32(sift_partition_t part, uint32_t value,
                                                        sift_aperture_t ap, uint64_t reg);

/// @brief Reads up to size bytes from addr
///
/// @param part partition handle
/// @param region memory region
/// @param addr address to target
/// @param data buffer to place read data
/// @param size number of bytes to read
/// @returns up to size on success, -1 on error
ROCSIFT_EXPORT ssize_t sift_partition_read(sift_partition_t part, void *data,
                                           sift_mem_region_t region, uint64_t addr, size_t size);

/// @brief Writes up to size bytes to addr
///
/// @param part partition handle
/// @param region memory region
/// @param addr address to target
/// @param data buffer of data to write
/// @param size number of bytes to write
/// @returns up to size on success, -1 on error
ROCSIFT_EXPORT ssize_t sift_partition_write(sift_partition_t part, const void *data,
                                            sift_mem_region_t region, uint64_t addr, size_t size);

/// @brief fetches KFD node for a given partition
///
/// @param part partition to fetch node for
/// @param node pointer to node to fill
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_partition_get_kfd_node(sift_partition_t part,
                                                         sift_kfd_node_t *node);

/// @brief looks up message for a sift_status_t
/// @param status status to lookup mesage for
/// @retval const char* to message
ROCSIFT_EXPORT const char *sift_status_get_message(sift_status_t status);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _SIFT_H_
