/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "rocsift/kfd.h"
#include "rocsift/sift.h"
#include "rocsift/pm4.h"
#include "rocsift/chipid.h"

#include "rocsift/xlator.h"

#define SIFT_CHECK(command)                              \
  {                                                      \
    sift_status_t stat = (command);                      \
    if (stat != SIFT_STATUS_SUCCESS) {                   \
      printf("sift error: %s:%d\n", __FILE__, __LINE__); \
      exit(-1);                                          \
    }                                                    \
  }

#define PDE_SIZE_BYTES 8
#define PTE_SIZE_BYTES 8

static const uint64_t log2num_bytes_4kib_page = 12;
static const uint64_t log2num_intermediate_pdbs = 9;
static const uint64_t log2num_mib2 = 21;

bool fragments_overlap(const fragment_t lhs, const fragment_t rhs)
{
  if (!lhs.flags.valid) {
    return false;
  }
  if (!rhs.flags.valid) {
    return false;
  }

  if ((lhs.va == rhs.va) && (lhs.pa == rhs.pa) && (lhs.size == rhs.size)) {
    fprintf(stderr,
            "DUPLICATE: lhs == rhs! rhs.va: 0x%016lx rhs.pa: 0x%016lx rhs.size:0x%016lx lhs.va: "
            "0x%016lx lhs.pa: 0x%016lx lhs.size:0x%016lx\n",
            rhs.va, rhs.pa, rhs.size, lhs.va, lhs.pa, lhs.size);
    return true;
  }
  if ((rhs.va >= lhs.va) && (rhs.va < lhs.va + lhs.size)) {
    fprintf(stderr,
            "VA OVERLAP DETECTED: (rhs.va >= lhs.va) && (rhs.va < lhs.va + lhs.size) rhs.va: "
            "0x%016lx "
            "rhs.pa: 0x%016lx "
            "rhs.size:0x%016lx lhs.va: 0x%016lx lhs.pa: 0x%016lx lhs.size:0x%016lx\n",
            rhs.va, rhs.pa, rhs.size, lhs.va, lhs.pa, lhs.size);
    return true;
  }
  if ((rhs.pa >= lhs.pa) && (rhs.pa < lhs.pa + lhs.size)) {
    fprintf(stderr,
            "PA OVERLAP DETECTED: (rhs.pa >= lhs.pa) && (rhs.pa < lhs.pa + lhs.size) rhs.va: "
            "0x%016lx rhs.pa: 0x%016lx  rhs.size: 0x%016lx "
            "lhs.va: 0x%016lx lhs.pa: 0x%016lx "
            "lhs.size:0x%016lx\n",
            rhs.va, rhs.pa, rhs.size, lhs.va, lhs.pa, lhs.size);
    return true;
  }
  if ((lhs.pa >= rhs.pa) && (lhs.pa < rhs.pa + rhs.size)) {
    fprintf(stderr,
            "PA OVERLAP DETECTED: (lhs.pa >= rhs.pa) && (lhs.pa < rhs.pa + rhs.size) lhs.va: "
            "0x%016lx lhs.pa: 0x%016lx lhs.size: 0x%016lx "
            "rhs.va: 0x%016lx "
            "rhs.pa: 0x%016lx "
            "rhs.size:0x%016lx\n",
            lhs.va, lhs.pa, lhs.size, rhs.va, rhs.pa, rhs.size);
    return true;
  }
  return false;
}

bool device_supported(uint32_t device_id) { return is_vega20(device_id) || is_mi3xx(device_id); }

uint32_t VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_LO32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A32C;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A3AC;

  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_HI32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A330;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A3B0;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t VM_CONTEXT0_PAGE_TABLE_START_ADDR_LO32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A3AC;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A42C;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t VM_CONTEXT0_PAGE_TABLE_START_ADDR_HI32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A3B0;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A430;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t VM_CONTEXT0_PAGE_TABLE_END_ADDR_LO32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A42C;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A4AC;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t VM_CONTEXT0_PAGE_TABLE_END_ADDR_HI32(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A430;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A4B0;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}
uint32_t VM_CONTEXT0_CNTL(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A180;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A200;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t MC_VM_FB_OFFSET_BASE(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A51C;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A5AC;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint32_t MC_VM_FB_LOCATION_BASE(uint32_t device_id)
{
  if (is_mi3xx(device_id)) return 0x0000A570;
  if (is_vega20(device_id) || is_mi200(device_id)) return 0x0000A600;
  fprintf(stderr, "DeviceID 0x%08x not currently supported\n", device_id);
  abort();
}

uint64_t get_bits(uint64_t val, int msb, int lsb)
{
  uint64_t mask = ((1ul << msb) - 1u) * 2 + 1;
  return (val & mask) >> lsb;
}

uint64_t GFXHUB_OFFSET(int xcc_id) { return 0x40000 * xcc_id; }

uint64_t MMHUB_OFFSET(int xcc_id) { return 0x80000 * xcc_id; }

typedef struct pde_s {
  uint8_t valid;
  uint8_t system;
  uint8_t coherent;
  uint64_t base_address;
  uint8_t pte;
  uint8_t block_fragment_size;  // log2 number of 4KiB pages represented by any given PTE.
                                // default/legacy value of 0 means that the PTEs pointed to by a PDE
                                // each map to 4KiB pages
} pde_t;

#define PT_MAX_DEPTH 3

typedef struct walk_level_info_s {
  size_t pde_size_shift;
  size_t pde_size;
  size_t num_pdes;
  size_t va_range;
} walk_level_info_t;

typedef struct walk_helper_s {
  uint32_t pt_depth;
  uint32_t pt_block_size;
  uint32_t current_depth;
  uint64_t pt_start_addr;
  uint64_t pt_base_addr;
  uint64_t pt_end_addr;
  uint64_t fb_offset;
  uintptr_t pde_addrs[PT_MAX_DEPTH + 1];
  uint64_t raw_pdes[PT_MAX_DEPTH + 1];
  pde_t pdes[PT_MAX_DEPTH + 1];
  walk_level_info_t levelinfo[PT_MAX_DEPTH + 1];
  pte_t pte;
} walk_helper_t;

typedef struct walk_results_s {
  pte_t pte;
  size_t size;
  size_t mask;
} walk_results_t;

void print_pde(pde_t fields, int level, bool base)
{
  if (base) {
    printf("PDE:BASE v:%d s:%d c:%d pte:%d block_frag_sz:%d base:0x%012lx\n", fields.valid,
           fields.system, fields.coherent, fields.pte, fields.block_fragment_size,
           fields.base_address);
  } else {
    printf("PDE:%d v:%d s:%d c:%d pte:%d block_frag_sz:%d base:0x%012lx\n", level, fields.valid,
           fields.system, fields.coherent, fields.pte, fields.block_fragment_size,
           fields.base_address);
  }
}
void print_pte(pte_t fields)
{
  printf(
      "v:%d s:%d c:%d tmz:%d x:%d r:%d w:%d f:%d t:%d sw:%d pte:%d log:%d tf:%d mtype:%d "
      "base:0x%012lx\n",
      fields.valid, fields.system, fields.coherent, fields.tmz, fields.execute, fields.read,
      fields.write, fields.fragment, fields.tiled, fields.sw, fields.pte, fields.log,
      fields.translate_further, fields.mtype, fields.base_address);
}

pde_t get_pde_fields(uint64_t pde)
{
  pde_t p;
  p.valid = get_bits(pde, 0, 0);
  p.system = get_bits(pde, 1, 1);
  p.coherent = get_bits(pde, 2, 2);
  p.base_address = get_bits(pde, 47, 6) << 6ull;
  p.pte = get_bits(pde, 54, 54);
  p.block_fragment_size = get_bits(pde, 63, 59);
  return p;
}

pte_t get_pte_fields(uint64_t pte)
{
  pte_t p;
  p.valid = get_bits(pte, 0, 0);
  p.system = get_bits(pte, 1, 1);
  p.coherent = get_bits(pte, 2, 2);
  p.tmz = get_bits(pte, 3, 3);
  p.execute = get_bits(pte, 4, 4);
  p.read = get_bits(pte, 5, 5);
  p.write = get_bits(pte, 6, 6);
  p.fragment = get_bits(pte, 11, 7);
  p.base_address = get_bits(pte, 47, 12) << 12ull;
  p.tiled = get_bits(pte, 51, 51);
  p.sw = get_bits(pte, 53, 52);
  p.pte = get_bits(pte, 54, 54);
  p.log = get_bits(pte, 55, 55);
  p.translate_further = get_bits(pte, 56, 56);
  p.mtype = get_bits(pte, 58, 57);
  return p;
}

uint64_t read_ent(sift_partition_t part, uint64_t addr, bool system)
{
  size_t n;
  uint64_t val;
  if (system) {
    n = sift_partition_read(part, &val, SIFT_LINEAR_SYSTEM_MEM, addr, sizeof(uint64_t));
  } else {
    n = sift_partition_read(part, &val, SIFT_LINEAR_VIDEO_MEM, addr, sizeof(uint64_t));
  }
  if (n == -1) {
    printf("An error occured: %s\n", strerror(errno));
    exit(-1);
  }
  if (n < sizeof(uint64_t)) {
    printf("Failed to read 8B at once, what to do\n");
    exit(-1);
  }
  return val;
};

static int walk_pt(sift_partition_t part, uint64_t va, walk_helper_t *helper,
                   walk_results_t *results)
{
  if (helper->pt_depth == 0) {
    fprintf(stderr, "Flat page tables are not supported\n");
    return -1;
  }
  uint64_t base = helper->pt_base_addr;
  va -= helper->pt_start_addr;
  pde_t pde;
  bool pde_is_pte = false;
  for (helper->current_depth = helper->pt_depth; helper->current_depth != -1;
       helper->current_depth--) {
    walk_level_info_t *levelinfo = &helper->levelinfo[helper->current_depth];
    pde = get_pde_fields(base);
    helper->pdes[helper->current_depth] = pde;
    if (pde.pte) {
      helper->current_depth--;
      pde_is_pte = true;
      break;
    }
    if (!pde.system) {
      pde.base_address -= helper->fb_offset;
    }

    size_t pde_idx = (va >> levelinfo->pde_size_shift) & (levelinfo->num_pdes - 1);
    uintptr_t pde_addr = pde_idx * PDE_SIZE_BYTES + pde.base_address;
    if (pde_addr == helper->pde_addrs[helper->current_depth]) {
      base = helper->raw_pdes[helper->current_depth];
    } else {
      base = read_ent(part, pde_addr, pde.system);
      helper->pde_addrs[helper->current_depth] = pde_addr;
      helper->raw_pdes[helper->current_depth] = base;
    }
  }

  pte_t pte = get_pte_fields(base);
  helper->pte = pte;

  // at this point, helper->current_depth is equal to actual current_depth - 1
  // due to the structure of the loop above.
  pde_t last_pde = helper->pdes[helper->current_depth + 1];

  if (pde_is_pte) {
    if (helper->current_depth + 1 == 0) {
      results->size = 1ull << (helper->pt_block_size + log2num_mib2);
    } else {
      results->size = (1ull << log2num_intermediate_pdbs) * PDE_SIZE_BYTES;
    }

  } else {
    results->size = 1ull << (last_pde.block_fragment_size + log2num_bytes_4kib_page);
  }

  if (pte.valid) {
    // translate-further does not apply to PDE.P PTEs
    if (!pde.pte && pte.translate_further) {
      if (results->size != 1ull << 21) {
        fprintf(stderr, "2MiB pages required for translate-further");
        return -1;
      }
      size_t num_ptes = 1ull << log2num_intermediate_pdbs;
      size_t pte_idx = (va >> log2num_bytes_4kib_page) & (num_ptes - 1);
      results->size = results->size >> log2num_intermediate_pdbs;

      /* The Translate Further (F) bit applies only to what is normally the leaf pte.
       * It is expected to be used in cases where the native page size is greater than 4kb.
       * In that case, when the F bit is asserted, then the pte is treated instead as a pointer to a
       * block of 4kb equivalents; in fact, a pte with the F bit set has the same format as a PDE.
       * In what is expected to be the most typical case, when the native page size of a pte is 64kb
       * and the F bit is asserted, then the pte is treated as a pointer to a block of 16 4kb ptes.
       * For now, the Translate Further (F) option cannot be used in concert with the PDE as PTE (P)
       * option. At this time, our verification plan is targeted only at supporting 64KB native size
       * ptes using TF that point to 4KB native size ptes. While other combinations are possible, we
       * do not plan to verify any but this particular situation.
       */
      pde = get_pde_fields(base);
      uintptr_t pte_addr = pde.base_address + pte_idx * PTE_SIZE_BYTES;
      base = read_ent(part, pte_addr, pde.system);
      pte = get_pte_fields(base);
      helper->pte = pte;
    }
  }
  results->mask = results->size - 1;
  results->pte = pte;
  helper->current_depth = helper->pt_depth;
  return 0;
}

int sift_xlator_create(xlator_t *xlator, sift_partition_t part, sift_kfd_node_t node, int vmid)
{
  sift_kfd_node_properties_t props;
  sift_status_t rc = sift_kfd_node_get_properties(node, &props);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  if (!device_supported(props.device_id)) {
    return -1;
  }

  int xcc_id;

  rc = sift_partition_get_xcc_die_ids(part, &xcc_id, 1);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  xlator->part = part;
  xlator->vmid = vmid;
  xlator->pt_base_addr_lo_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                 VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_LO32(props.device_id);
  xlator->pt_base_addr_hi_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                 VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_HI32(props.device_id);
  xlator->pt_start_addr_lo_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                  VM_CONTEXT0_PAGE_TABLE_START_ADDR_LO32(props.device_id);
  xlator->pt_start_addr_hi_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                  VM_CONTEXT0_PAGE_TABLE_START_ADDR_HI32(props.device_id);
  xlator->pt_end_addr_lo_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                VM_CONTEXT0_PAGE_TABLE_END_ADDR_LO32(props.device_id);
  xlator->pt_end_addr_hi_addr = GFXHUB_OFFSET(xcc_id) + 8 * vmid +
                                VM_CONTEXT0_PAGE_TABLE_END_ADDR_HI32(props.device_id);
  xlator->context_cntl_addr = GFXHUB_OFFSET(xcc_id) + 4 * vmid + VM_CONTEXT0_CNTL(props.device_id);
  xlator->mc_vm_fb_offset_addr = GFXHUB_OFFSET(xcc_id) + MC_VM_FB_OFFSET_BASE(props.device_id);

  return 0;
}

int sift_xlator_pt_base_from_pasid(uint64_t *pt_base, sift_kfd_node_t node, int pasid)
{
  int gpuid;
  sift_status_t rc = sift_kfd_node_get_gpuid(node, &gpuid);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  sift_pm4_runlist_series_t *runlists;
  rc = sift_kfd_get_runlists(&runlists);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }
  sift_pm4_runlist_series_t *current = runlists;
  sift_pm4_runlist_t *rls = NULL;
  while (current) {
    rls = current->runlist;
    if (rls->node.gpu_id == gpuid) {
      break;
    }
    current = current->next;
  }

  if (!rls) {
    return -1;
  }

  sift_pm4_runlist_entry_t *ent = NULL;
  for (size_t i = 0; i < rls->num_entries; i++) {
    if (rls->entries[i].header.type != eSIFT_PM4_TYPE3) {
      continue;
    }
    if (rls->entries[i].header.opcode != eSIFT_PM4_OP_MAP_PROCESS) {
      continue;
    }

    if (rls->entries[i].body->map_process.pasid != pasid) {
      continue;
    }
    ent = &rls->entries[i];
  }

  if (!ent) {
    sift_pm4_destroy_runlists(runlists);
    return -1;
  }

  uint64_t base = ((uint64_t)ent->body->map_process.vm_context_page_table_base_addr_hi32) << 32ul;
  base |= ent->body->map_process.vm_context_page_table_base_addr_lo32;

  *pt_base = base;
  return sift_pm4_destroy_runlists(runlists);
}

int sift_xlator_vmid_from_pt_base(int *vmid, sift_partition_t part, sift_kfd_node_t node,
                                  uint64_t pt_base)
{
  sift_kfd_node_properties_t props;
  sift_status_t rc = sift_kfd_node_get_properties(node, &props);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }
  int xcc_id;

  rc = sift_partition_get_xcc_die_ids(part, &xcc_id, 1);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  for (int v = 0; v < 16; v++) {
    uint64_t pt_base_addr_lo_addr = GFXHUB_OFFSET(xcc_id) + 8 * v +
                                    VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_LO32(props.device_id);
    uint64_t pt_base_addr_hi_addr = GFXHUB_OFFSET(xcc_id) + 8 * v +
                                    VM_CONTEXT0_PAGE_TABLE_BASE_ADDR_HI32(props.device_id);

    uint32_t pt_base_lo;
    uint32_t pt_base_hi;

    sift_status_t rc = sift_partition_read_reg32(part, &pt_base_lo, SIFT_APER_MMIO,
                                                 pt_base_addr_lo_addr);
    if (rc != SIFT_STATUS_SUCCESS) {
      return -1;
    }
    rc = sift_partition_read_reg32(part, &pt_base_hi, SIFT_APER_MMIO, pt_base_addr_hi_addr);
    if (rc != SIFT_STATUS_SUCCESS) {
      return -1;
    }
    uint64_t base = (uint64_t)pt_base_hi << 32ul | pt_base_lo;
    if (base == pt_base) {
      *vmid = v;
      return 0;
    }
  }
  return -1;
}

int sift_xlator_destroy(xlator_t *xlator) { return 0; }

int sift_xlator_create_for_process(xlator_t *xlator, int pid, sift_partition_t part)
{
  sift_kfd_proc_list_t list;
  sift_status_t rc = sift_kfd_get_proc_list(&list);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }
  bool found = false;
  int pasid;
  for (size_t i = 0; i < list.n; i++) {
    if (list.procs[i].pid == pid) {
      pasid = list.procs[i].pasid;
      found = true;
      break;
    }
  }

  rc = sift_kfd_proc_list_destroy(&list);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  if (!found) {
    return -1;
  }

  sift_kfd_node_t node;
  rc = sift_partition_get_kfd_node(part, &node);
  if (rc != SIFT_STATUS_SUCCESS) {
    return -1;
  }

  uint64_t pt_base;
  int err = sift_xlator_pt_base_from_pasid(&pt_base, node, pasid);
  if (err) {
    return err;
  }
  int vmid;
  err = sift_xlator_vmid_from_pt_base(&vmid, part, node, pt_base);
  if (err) {
    return err;
  }
  return sift_xlator_create(xlator, part, node, vmid);
}

int sift_xlator_create_for_current_process(xlator_t *xlator, sift_partition_t part)
{
  return sift_xlator_create_for_process(xlator, getpid(), part);
}

static int sift_xlator_populate_walk_helper(xlator_t *xlate, walk_helper_t *helper)
{
  uint32_t page_table_start_addr_lo;
  uint32_t page_table_start_addr_hi;
  uint32_t page_table_base_addr_lo;
  uint32_t page_table_base_addr_hi;
  uint32_t page_table_end_addr_lo;
  uint32_t page_table_end_addr_hi;
  uint32_t context_cntl;
  uint32_t fb_offset;

  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_start_addr_lo, SIFT_APER_MMIO,
                                       xlate->pt_start_addr_lo_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_start_addr_hi, SIFT_APER_MMIO,
                                       xlate->pt_start_addr_hi_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_end_addr_lo, SIFT_APER_MMIO,
                                       xlate->pt_end_addr_lo_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_end_addr_hi, SIFT_APER_MMIO,
                                       xlate->pt_end_addr_hi_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_base_addr_lo, SIFT_APER_MMIO,
                                       xlate->pt_base_addr_lo_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &page_table_base_addr_hi, SIFT_APER_MMIO,
                                       xlate->pt_base_addr_hi_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &context_cntl, SIFT_APER_MMIO,
                                       xlate->context_cntl_addr));
  SIFT_CHECK(sift_partition_read_reg32(xlate->part, &fb_offset, SIFT_APER_MMIO,
                                       xlate->mc_vm_fb_offset_addr));

  fb_offset <<= 24;

  uint64_t pt_start_addr = (((uint64_t)page_table_start_addr_hi << 32ul) | page_table_start_addr_lo)
                           << 12ul;
  uint64_t pt_base_addr = (uint64_t)page_table_base_addr_hi << 32ul | page_table_base_addr_lo
                                                                          << 0ul;

  uint64_t pt_end_addr = (((uint64_t)page_table_end_addr_hi << 32ul) | page_table_end_addr_lo)
                         << 12ul;

  uint32_t pt_depth = get_bits(context_cntl, 2, 1);
  uint32_t pt_block_size = get_bits(context_cntl, 6, 3);

  helper->pt_depth = pt_depth;
  helper->pt_block_size = pt_block_size;
  helper->fb_offset = fb_offset;
  helper->pt_start_addr = pt_start_addr;
  helper->pt_base_addr = pt_base_addr;
  helper->pt_end_addr = pt_end_addr;
  helper->current_depth = 0;

  memset(helper->pde_addrs, 0x0, sizeof(uintptr_t) * (PT_MAX_DEPTH + 1));
  memset(helper->raw_pdes, 0x0, sizeof(uint64_t) * (PT_MAX_DEPTH + 1));

  for (size_t i = helper->pt_depth; i != -1; i--) {
    helper->levelinfo[i].pde_size_shift = (helper->pt_block_size + log2num_mib2) +
                                          ((i - 1) * log2num_intermediate_pdbs);

    helper->levelinfo[i].pde_size = 1ull << helper->levelinfo[i].pde_size_shift;
    helper->levelinfo[i].va_range = helper->pt_end_addr - helper->pt_start_addr +
                                    (1ull << log2num_bytes_4kib_page);
    helper->levelinfo[i].num_pdes = (i == helper->pt_depth) ? helper->levelinfo[i].va_range /
                                                                  helper->levelinfo[i].pde_size
                                                            : 1ull << log2num_intermediate_pdbs;
  }
  return 0;
}

int sift_xlator_translate(xlator_t *xlate, translation_t *trans, uintptr_t va)
{
  fragment_list_t *list;
  int rc = sift_xlator_translate_range(xlate, &list, va, 0x1000 - (va & 0xfff),
                                       false /*combine contiquous pages*/);
  if (rc) {
    return rc;
  }
  if (!list) {
    return -1;
  }
  trans->pte = list->fragment.flags;
  trans->ok = list->fragment.flags.valid;
  trans->size = list->fragment.size;

  uintptr_t aligned_va = va & ~(list->fragment.size - 1);
  uintptr_t offset = va - aligned_va;
  trans->pa = list->fragment.flags.base_address + offset;

  return sift_xlator_fragment_list_destroy(list);
}

bool is_contiguous(fragment_t *prev, fragment_t *curr)
{
  if (prev->va + prev->size > curr->va) {
    fprintf(stderr, "CODE BUG\n");
    exit(-1);
  }
  if (prev->va + prev->size != curr->va) return false;

  // if neither translation is valid, but VAs are contiguous, is contig
  if ((!prev->flags.valid) && (!curr->flags.valid)) return true;
  if (prev->pa + prev->size != curr->pa) return false;
  if (prev->flags.valid != curr->flags.valid) return false;
  if (prev->flags.system != curr->flags.system) return false;
  if (prev->flags.coherent != curr->flags.coherent) return false;
  if (prev->flags.tmz != curr->flags.tmz) return false;
  if (prev->flags.execute != curr->flags.execute) return false;
  if (prev->flags.read != curr->flags.read) return false;
  if (prev->flags.write != curr->flags.write) return false;
  if (prev->flags.fragment != curr->flags.fragment) return false;
  if (prev->flags.tiled != curr->flags.tiled) return false;
  if (prev->flags.sw != curr->flags.sw) return false;
  if (prev->flags.pte != curr->flags.pte) return false;
  if (prev->flags.log != curr->flags.log) return false;
  if (prev->flags.translate_further != curr->flags.translate_further) return false;
  if (prev->flags.mtype != curr->flags.mtype) return false;
  return true;
}

int sift_xlator_translate_range(xlator_t *xlate, fragment_list_t **list, uintptr_t va, size_t size,
                                bool combine_contiguous_pages)
{
  walk_helper_t helper;
  int rc = sift_xlator_populate_walk_helper(xlate, &helper);
  if (rc) {
    return rc;
  }

  fragment_list_t *head = NULL;
  fragment_list_t *node = NULL;

  uintptr_t addr = va;
  size_t count = 0;
  fragment_t prev;

  memset(&prev, 0x0, sizeof(fragment_t));
  while (addr < va + size) {
    walk_results_t results;
    rc = walk_pt(xlate->part, addr, &helper, &results);
    if (rc) {
      return rc;
    }
    uintptr_t aligned_va = addr & ~results.mask;
    //    uintptr_t offset = addr - aligned_va;
    // uintptr_t pa = results.pte.base_address + offset;
    uintptr_t pa = results.pte.base_address;

    if (addr != aligned_va) {
      // if the start of the buffer is not aligned to the page size,
      //  then we need to make sure to add in the bytes we loped off
      size += (addr - aligned_va);
    }
    fragment_t curr;
    curr.va = aligned_va;
    curr.pa = pa;
    curr.size = results.size;
    curr.flags = results.pte;

    if (count == 0) {
      prev = curr;
    } else {
      // if fragment is identical, increment address and continue?
      if (fragments_overlap(prev, curr)) {
        fprintf(stderr, "ROCSIFT: fragment overlap detected\n");
        sift_xlator_fragment_list_destroy(head);
        return -1;
      }
      if (combine_contiguous_pages && is_contiguous(&prev, &curr)) {
        prev.size += curr.size;
      } else {
        if (!head) {
          head = malloc(sizeof(fragment_list_t));
          if (!head) {
            fprintf(stderr, "ROCSIFT: Failed to allocate memory for fragment_list_t\n");
            return -1;
          }
          node = head;
        } else {
          fragment_list_t *next = malloc(sizeof(fragment_list_t));
          if (!next) {
            free(head);
            fprintf(stderr, "ROCSIFT: Failed to allocate memory for fragment_list_t\n");
            return -1;
          }
          node->next = next;
          node = next;
        }
        node->fragment.va = prev.va;
        node->fragment.pa = prev.pa;
        node->fragment.size = prev.size;
        node->fragment.flags = prev.flags;
        node->next = NULL;
        prev = curr;
      }
    }
    addr = curr.va + curr.size;
    count++;
  }
  if (!head) {
    head = malloc(sizeof(fragment_list_t));
    if (!head) {
      fprintf(stderr, "ROCSIFT: Failed to allocate memory for fragment_list_t\n");
      return -1;
    }
    node = head;
  } else {
    fragment_list_t *next = malloc(sizeof(fragment_list_t));
    if (!next) {
      free(head);
      fprintf(stderr, "ROCSIFT: Failed to allocate memory for fragment_list_t\n");
      return -1;
    }
    node->next = next;
    node = next;
  }
  node->fragment.va = prev.va;
  node->fragment.pa = prev.pa;
  node->fragment.size = prev.size;
  node->fragment.flags = prev.flags;
  node->next = NULL;

  *list = head;
  return 0;
}

int sift_xlator_fragment_list_destroy(fragment_list_t *list)
{
  while (list) {
    fragment_list_t *next = list->next;
    free(list);
    list = next;
  }
  return 0;
}
