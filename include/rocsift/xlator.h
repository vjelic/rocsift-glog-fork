/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_XLATOR_H_
#define _SIFT_XLATOR_H_

#include "rocsift/kfd.h"
#include "rocsift/sift.h"
#include "rocsift/visibility.h"

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef struct pte_s {
  uint8_t valid;
  uint8_t system;
  uint8_t coherent;
  uint8_t tmz;
  uint8_t execute;
  uint8_t read;
  uint8_t write;
  uint8_t fragment;
  uint64_t base_address;
  uint8_t tiled;
  uint8_t sw;
  uint8_t pte;
  uint8_t log;
  uint8_t translate_further;  // PTEs who themselves defined as pointers to further PTE lookup
  uint8_t mtype;
} pte_t;

typedef struct translation_s {
  uintptr_t pa;
  bool ok;
  pte_t pte;
  size_t size;
} translation_t;

typedef struct fragment_s {
  uintptr_t va;
  uintptr_t pa;
  size_t size;
  pte_t flags;
} fragment_t;

typedef struct fragment_list_s {
  fragment_t fragment;
  struct fragment_list_s *next;
} fragment_list_t;

typedef struct xlator_s {
  sift_partition_t part;
  int vmid;
  uint32_t pt_base_addr_lo_addr;
  uint32_t pt_base_addr_hi_addr;
  uint32_t pt_start_addr_lo_addr;
  uint32_t pt_start_addr_hi_addr;
  uint32_t pt_end_addr_lo_addr;
  uint32_t pt_end_addr_hi_addr;
  uint32_t context_cntl_addr;
  uint32_t mc_vm_fb_offset_addr;
  uint32_t mc_vm_fb_location_addr;
} xlator_t;

ROCSIFT_EXPORT int sift_xlator_create_for_current_process(xlator_t *xlator, sift_partition_t part);
ROCSIFT_EXPORT int sift_xlator_create_for_process(xlator_t *xlator, int pid, sift_partition_t part);

ROCSIFT_EXPORT int sift_xlator_pt_base_from_pasid(uint64_t *pt_base, sift_kfd_node_t node,
                                                  int pasid);
ROCSIFT_EXPORT int sift_xlator_vmid_from_pt_base(int *vmid, sift_partition_t part,
                                                 sift_kfd_node_t node, uint64_t pt_base);
ROCSIFT_EXPORT int sift_xlator_create(xlator_t *xlator, sift_partition_t part, sift_kfd_node_t node,
                                      int vmid);
ROCSIFT_EXPORT int sift_xlator_destroy(xlator_t *xlator);
ROCSIFT_EXPORT int sift_xlator_translate(xlator_t *xlate, translation_t *trans, uintptr_t va);
ROCSIFT_EXPORT int sift_xlator_translate_range(xlator_t *xlate, fragment_list_t **list,
                                               uintptr_t va, size_t size,
                                               bool combine_contiguous_pages);
ROCSIFT_EXPORT int sift_xlator_fragment_list_destroy(fragment_list_t *list);

#ifdef __cplusplus
}

#endif  // __cplusplus
#endif  // _SIFT_XLATOR_H_
