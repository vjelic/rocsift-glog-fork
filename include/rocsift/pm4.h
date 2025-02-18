/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_PM4_H_
#define _SIFT_PM4_H_

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "rocsift/status.h"
#include "rocsift/visibility.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  eSIFT_PM4_TYPE1 = 0,
  eSIFT_PM4_TYPE2 = 1,
  eSIFT_PM4_TYPE3 = 2
} sift_pm4_packet_type_t;

typedef enum {
  eSIFT_PM4_OP_NOP = 0x10,
  eSIFT_PM4_OP_SET_BASE = 0x11,
  eSIFT_PM4_OP_CLEAR_STATE = 0x12,
  eSIFT_PM4_OP_INDEX_BUFFER_SIZE = 0x13,
  eSIFT_PM4_OP_DISPATCH_DIRECT = 0x15,
  eSIFT_PM4_OP_DISPATCH_INDIRECT = 0x16,
  eSIFT_PM4_OP_ATOMIC_GDS = 0x1D,
  eSIFT_PM4_OP_OCCLUSION_QUERY = 0x1F,
  eSIFT_PM4_OP_SET_PREDICATION = 0x20,
  eSIFT_PM4_OP_REG_RMW = 0x21,
  eSIFT_PM4_OP_COND_EXEC = 0x22,
  eSIFT_PM4_OP_PRED_EXEC = 0x23,
  eSIFT_PM4_OP_DRAW_INDIRECT = 0x24,
  eSIFT_PM4_OP_DRAW_INDEX_INDIRECT = 0x25,
  eSIFT_PM4_OP_INDEX_BASE = 0x26,
  eSIFT_PM4_OP_DRAW_INDEX_2 = 0x27,
  eSIFT_PM4_OP_CONTEXT_CONTROL = 0x28,
  eSIFT_PM4_OP_INDEX_TYPE = 0x2A,
  eSIFT_PM4_OP_DRAW_INDIRECT_MULTI = 0x2C,
  eSIFT_PM4_OP_DRAW_INDEX_AUTO = 0x2D,
  eSIFT_PM4_OP_NUM_INSTANCES = 0x2F,
  eSIFT_PM4_OP_DRAW_INDEX_MULTI_AUTO = 0x30,
  eSIFT_PM4_OP_INDIRECT_BUFFER_CNST = 0x33,
  eSIFT_PM4_OP_STRMOUT_BUFFER_UPDATE = 0x34,
  eSIFT_PM4_OP_DRAW_INDEX_OFFSET_2 = 0x35,
  eSIFT_PM4_OP_DRAW_PREAMBLE = 0x36,
  eSIFT_PM4_OP_WRITE_DATA = 0x37,
  eSIFT_PM4_OP_DRAW_INDEX_INDIRECT_MULTI = 0x38,
  eSIFT_PM4_OP_MEM_SEMAPHORE = 0x39,
  eSIFT_PM4_OP_COPY_DW = 0x3B,
  eSIFT_PM4_OP_WAIT_REG_MEM = 0x3C,
  eSIFT_PM4_OP_INDIRECT_BUFFER = 0x3F,
  eSIFT_PM4_OP_COPY_DATA = 0x40,
  eSIFT_PM4_OP_PFP_SYNC_ME = 0x42,
  eSIFT_PM4_OP_SURFACE_SYNC = 0x43,
  eSIFT_PM4_OP_COND_WRITE = 0x45,
  eSIFT_PM4_OP_EVENT_WRITE = 0x46,
  eSIFT_PM4_OP_EVENT_WRITE_EOP = 0x47,
  eSIFT_PM4_OP_EVENT_WRITE_EOS = 0x48,
  eSIFT_PM4_OP_RELEASE_MEM = 0x49,
  eSIFT_PM4_OP_PREAMBLE_CNTL = 0x4A,
  eSIFT_PM4_OP_DMA_DATA = 0x50,
  eSIFT_PM4_OP_ACQUIRE_MEM = 0x58,
  eSIFT_PM4_OP_REWIND = 0x59,
  eSIFT_PM4_OP_LOAD_UCONFIG_REG = 0x5E,
  eSIFT_PM4_OP_LOAD_SH_REG = 0x5F,
  eSIFT_PM4_OP_LOAD_CONFIG_REG = 0x60,
  eSIFT_PM4_OP_LOAD_CONTEXT_REG = 0x61,
  eSIFT_PM4_OP_SET_CONFIG_REG = 0x68,
  eSIFT_PM4_OP_SET_CONTEXT_REG = 0x69,
  eSIFT_PM4_OP_SET_CONTEXT_REG_INDIRECT = 0x73,
  eSIFT_PM4_OP_SET_SH_REG = 0x76,
  eSIFT_PM4_OP_SET_SH_REG_OFFSET = 0x77,
  eSIFT_PM4_OP_SET_QUEUE_REG = 0x78,
  eSIFT_PM4_OP_SET_UCONFIG_REG = 0x79,
  eSIFT_PM4_OP_SCRATCH_RAM_WRITE = 0x7D,
  eSIFT_PM4_OP_SCRATCH_RAM_READ = 0x7E,
  eSIFT_PM4_OP_LOAD_CONST_RAM = 0x80,
  eSIFT_PM4_OP_WRITE_CONST_RAM = 0x81,
  eSIFT_PM4_OP_DUMP_CONST_RAM = 0x83,
  eSIFT_PM4_OP_INCREMENT_CE_COUNTER = 0x84,
  eSIFT_PM4_OP_INCREMENT_DE_COUNTER = 0x85,
  eSIFT_PM4_OP_WAIT_ON_CE_COUNTER = 0x86,
  eSIFT_PM4_OP_WAIT_ON_DE_COUNTER_DIFF = 0x88,
  eSIFT_PM4_OP_SWITCH_BUFFER = 0x8B,
  eSIFT_PM4_OP_SET_RESOURCES = 0xA0,
  eSIFT_PM4_OP_MAP_PROCESS = 0xA1,
  eSIFT_PM4_OP_MAP_QUEUES = 0xA2,
  eSIFT_PM4_OP_UNMAP_QUEUES = 0xA3,
  eSIFT_PM4_OP_QUERY_STATUS = 0xA4,
  eSIFT_PM4_OP_RUN_LIST = 0xA5,
} sift_pm4_pm4_opcode_t;

typedef struct sift_pm4_body_map_process_s {
  bool single_memop;
  bool debug_flag;
  bool tmz;
  bool diq_enable;
  bool sdma_enable;
  uint32_t pasid;
  uint16_t debug_vmid;
  uint8_t process_quantum;
  uint32_t vm_context_page_table_base_addr_lo32;
  uint32_t vm_context_page_table_base_addr_hi32;
  uint32_t sh_mem_bases;
  uint32_t sh_mem_config;
  uint32_t sq_shader_tba_lo;
  uint32_t sq_shader_tba_hi;
  uint32_t sq_shader_tma_lo;
  uint32_t sq_shader_tma_hi;
  uint32_t gds_addr_lo;
  uint32_t gds_addr_hi;
  uint8_t num_gws;
  uint8_t num_oac;
  uint8_t gds_size_hi;
  uint8_t gds_size_lo;
  uint16_t num_queues;
  uint32_t spi_gdbg_per_vmid_cntl;
  uint32_t tcp_watch0_cntl;
  uint32_t tcp_watch1_cntl;
  uint32_t tcp_watch2_cntl;
  uint32_t tcp_watch3_cntl;
  uint32_t completion_signal_lo32;
  uint32_t completion_signal_hi32;
} sift_pm4_body_map_process_t;

typedef struct sift_pm4_body_map_queues_s {
  uint8_t extended_engine_sel;
  uint8_t queue_sel;
  uint8_t vmid;
  bool gws_enabled;
  uint8_t queue;
  uint8_t queue_type;
  uint8_t static_queue_group;
  uint8_t engine_sel;
  uint8_t num_queues;
  bool check_disable;
  uint32_t doorbell_offset;
  uint32_t mqd_addr_lo;
  uint32_t mqd_addr_hi;
  uint32_t wptr_addr_lo;
  uint32_t wptr_addr_hi;
} sift_pm4_body_map_queues_t;

typedef struct sift_pm4_header_s {
  uint8_t opcode;
  uint16_t count;
  sift_pm4_packet_type_t type;
} sift_pm4_header_t;

typedef union sift_pm4_body_u {
  sift_pm4_body_map_process_t map_process;
  sift_pm4_body_map_queues_t map_queues;
} sift_pm4_body_t;

typedef struct sift_pm4_runlist_entry_s {
  sift_pm4_header_t header;
  sift_pm4_body_t *body;
} sift_pm4_runlist_entry_t;

typedef struct sift_pm4_node_s {
  uint32_t node_id;
  uint32_t gpu_id;
} sift_pm4_node_t;

typedef struct sift_pm4_runlist_s {
  sift_pm4_node_t node;
  size_t num_entries;
  sift_pm4_runlist_entry_t *entries;
} sift_pm4_runlist_t;

typedef struct sift_pm4_runlist_series_s {
  sift_pm4_runlist_t *runlist;
  struct sift_pm4_runlist_series_s *next;
} sift_pm4_runlist_series_t;

/// @brief extracts entries from a runlist string for a specific node
///
/// @param node node id to search for
/// @param gpu gpu id to search for
/// @param runlist_file_contents null-terminated string containing runlist text
/// @param runlist runlist struct to populate
/// @returns eSIFT_PM4_STATUS_SUCCESS on success
/// @see sift_pm4_destroy_runlist
ROCSIFT_EXPORT sift_status_t sift_pm4_parse_runlist(int node, int gpu,
                                                    const char *runlist_file_contents,
                                                    sift_pm4_runlist_t *runlist);

//// @brief extracts entries from a runlist string for all nodes
///
/// Populates a runlist linked list handle with the node(s) of the list.
/// The function handles allocation of runlist series nodes.
/// Destroy the series with sift_pm4_destroy_runlists
///
/// @param runlist_file_contents runlist file contents as a null-terminated string
/// @param runlists handle to runlist series to populate
/// @returns eSIFT_PM4_STATUS_SUCCESS on success
/// @see sift_pm4_destroy_runlists
ROCSIFT_EXPORT sift_status_t sift_pm4_parse_runlists(const char *runlist_file_contents,
                                                     sift_pm4_runlist_series_t **runlists);

/// @brief extracts entries from the runlist for a specific node
///
/// @param node node id to search for
/// @param gpu gpu id to search for
/// @param runlist runlist struct to populate
/// @param runlist_file_path runlist file to read
/// @returns eSIFT_PM4_STATUS_SUCCESS on success
/// @see sift_pm4_destroy_runlist
ROCSIFT_EXPORT sift_status_t sift_pm4_get_runlist(int node, int gpu, sift_pm4_runlist_t *runlist,
                                                  const char *runlist_file_path);

/// @brief extracts entries from a runlist file for all nodes
///
/// Populates a runlist linked list handle with the node(s) of the list.
/// The function handles allocation of runlist series nodes.
/// Destroy the series with sift_pm4_destroy_runlists
///
/// @param runlist_file_path runlist file to read
/// @param runlists handle to runlist series to populate
/// @returns eSIFT_PM4_STATUS_SUCCESS on success
/// @see sift_pm4_destroy_runlists
ROCSIFT_EXPORT sift_status_t sift_pm4_get_runlists(const char *runlist_file_path,
                                                   sift_pm4_runlist_series_t **runlists);

/// @brief destroys runlist created when parsing runlist
///
/// @param packets pointer to runlist to destroy
/// @returns eSIFT_PM4_STATUS_SUCCESS
ROCSIFT_EXPORT sift_status_t sift_pm4_destroy_runlist(sift_pm4_runlist_t *runlist);

/// @brief destroys runlist linked list created when parsing runlists
///
/// @param packets pointer to runlist series head
/// @returns eSIFT_PM4_STATUS_SUCCESS
ROCSIFT_EXPORT sift_status_t sift_pm4_destroy_runlists(sift_pm4_runlist_series_t *runlists);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif
