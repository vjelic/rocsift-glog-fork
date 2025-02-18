/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_KFD_EXT_H_
#define _SIFT_KFD_EXT_H_

#include <stdint.h>
#include <stddef.h>

#include "rocsift/status.h"
#include "rocsift/visibility.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct sift_kfd_node_s {
  uint64_t handle;
} sift_kfd_node_t;

typedef struct sift_kfd_node_properties_s {
  uint64_t cpu_cores_count;
  uint64_t simd_count;
  uint64_t mem_banks_count;
  uint64_t caches_count;
  uint64_t io_links_count;
  uint64_t p2p_links_count;
  uint64_t cpu_core_id_base;
  uint64_t simd_id_base;
  uint64_t max_waves_per_simd;
  uint64_t lds_size_in_kb;
  uint64_t gds_size_in_kb;
  uint64_t num_gws;
  uint64_t wave_front_size;
  uint64_t array_count;
  uint64_t simd_arrays_per_engine;
  uint64_t cu_per_simd_array;
  uint64_t simd_per_cu;
  uint64_t max_slots_scratch_cu;
  uint64_t gfx_target_version;
  uint64_t vendor_id;
  uint64_t device_id;
  uint64_t location_id;
  uint64_t domain_id;
  uint64_t drm_render_minor;
  uint64_t hive_id;
  uint64_t num_sdma_engines;
  uint64_t num_sdma_xgmi_engines;
  uint64_t num_sdma_queues_per_engine;
  uint64_t num_cp_queues;
  uint64_t max_engine_clk_fcompute;
  uint64_t local_mem_size;
  uint64_t fw_version;
  uint64_t capability;
  uint64_t debug_prop;
  uint64_t sdma_fw_version;
  uint64_t unique_id;
  uint64_t num_xcc;
  uint64_t max_engine_clk_ccompute;
} sift_kfd_node_properties_t;

typedef struct sift_kfd_proc_s {
  int pid;
  int pasid;
} sift_kfd_proc_t;

typedef struct sift_kfd_proc_list_s {
  size_t n;
  sift_kfd_proc_t *procs;
} sift_kfd_proc_list_t;

///! @cond
typedef struct sift_pm4_runlist_series_s sift_pm4_runlist_series_t;
///! @endcond

/// @brief fetches the number of KFD nodes discovered
///
/// @param n Pointer to integer to fill with number of KFD nodes
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_get_node_count(int *n);

/// @brief gets KFD node from index
///
/// @param n index of the KFD node to retrieve
/// @param node  pointer to node to fill
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_get_node(int n, sift_kfd_node_t *node);

/// @brief gets KFD properties for given node
///
/// @param node KFD node
/// @param props pointer to kfd properties to fill
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_node_get_properties(sift_kfd_node_t node,
                                                          sift_kfd_node_properties_t *props);

/// @brief gets a KFD process list
///
/// @param list pointer to KFD process list to fill in
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_get_proc_list(sift_kfd_proc_list_t *list);

/// @brief destroys a KFD process list
///
/// @param list pointer to KFD process list to destroy
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_proc_list_destroy(sift_kfd_proc_list_t *list);

/// @brief gets KFD node ID
///
/// @param node KFD node
/// @param instance pointer to int to fill with KFD node instance ID
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_node_get_id(sift_kfd_node_t node, int *instance);

/// @brief gets KFD node GPU_ID
///
/// @param node KFD node
/// @param instance pointer to int to fill with KFD node GPU ID
/// @returns SIFT_STATUS_SUCCESS on success
ROCSIFT_EXPORT sift_status_t sift_kfd_node_get_gpuid(sift_kfd_node_t node, int *gpu_id);

/// @brief Dumps runlist and extracts entries for all nodes
///
/// Populates a runlist linked list handle with the node(s) of the list.
/// The function handles allocation of runlist series nodes.
/// Destroy the series with sift_pm4_destroy_runlists (from pm4.h)
///
/// @param series handle to runlist series to populate
/// @returns SIFT_STATUS_SUCCESS on success
/// @see sift_pm4_destroy_runlists
ROCSIFT_EXPORT sift_status_t sift_kfd_get_runlists(sift_pm4_runlist_series_t **series);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _SIFT_KFD_EXT_H_
