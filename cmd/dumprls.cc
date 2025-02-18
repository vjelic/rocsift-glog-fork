#include <stdio.h>

#include "rocsift/sift.h"
#include "rocsift/pm4.h"
#include "rocsift/kfd.h"
#include "CLI/CLI.hpp"

const char* pm4_op_to_str(sift_pm4_pm4_opcode_t op)
{
  switch (op) {
    case eSIFT_PM4_OP_NOP:
      return "NOP";
    case eSIFT_PM4_OP_SET_BASE:
      return "SET_BASE";
    case eSIFT_PM4_OP_CLEAR_STATE:
      return "CLEAR_STATE";
    case eSIFT_PM4_OP_INDEX_BUFFER_SIZE:
      return "INDEX_BUFFER_SIZE";
    case eSIFT_PM4_OP_DISPATCH_DIRECT:
      return "DISPATCH_DIRECT";
    case eSIFT_PM4_OP_DISPATCH_INDIRECT:
      return "DISPATCH_INDIRECT";
    case eSIFT_PM4_OP_ATOMIC_GDS:
      return "ATOMIC_GDS";
    case eSIFT_PM4_OP_OCCLUSION_QUERY:
      return "OCCLUSION_QUERY";
    case eSIFT_PM4_OP_SET_PREDICATION:
      return "SET_PREDICATION";
    case eSIFT_PM4_OP_REG_RMW:
      return "REG_RMW";
    case eSIFT_PM4_OP_COND_EXEC:
      return "COND_EXEC";
    case eSIFT_PM4_OP_PRED_EXEC:
      return "PRED_EXEC";
    case eSIFT_PM4_OP_DRAW_INDIRECT:
      return "DRAW_INDIRECT";
    case eSIFT_PM4_OP_DRAW_INDEX_INDIRECT:
      return "DRAW_INDEX_INDIRECT";
    case eSIFT_PM4_OP_INDEX_BASE:
      return "INDEX_BASE";
    case eSIFT_PM4_OP_DRAW_INDEX_2:
      return "DRAW_INDEX_2";
    case eSIFT_PM4_OP_CONTEXT_CONTROL:
      return "CONTEXT_CONTROL";
    case eSIFT_PM4_OP_INDEX_TYPE:
      return "INDEX_TYPE";
    case eSIFT_PM4_OP_DRAW_INDIRECT_MULTI:
      return "DRAW_INDIRECT_MULTI";
    case eSIFT_PM4_OP_DRAW_INDEX_AUTO:
      return "DRAW_INDEX_AUTO";
    case eSIFT_PM4_OP_NUM_INSTANCES:
      return "NUM_INSTANCES";
    case eSIFT_PM4_OP_DRAW_INDEX_MULTI_AUTO:
      return "DRAW_INDEX_MULTI_AUTO";
    case eSIFT_PM4_OP_INDIRECT_BUFFER_CNST:
      return "INDIRECT_BUFFER_CNST";
    case eSIFT_PM4_OP_STRMOUT_BUFFER_UPDATE:
      return "STRMOUT_BUFFER_UPDATE";
    case eSIFT_PM4_OP_DRAW_INDEX_OFFSET_2:
      return "DRAW_INDEX_OFFSET_2";
    case eSIFT_PM4_OP_DRAW_PREAMBLE:
      return "DRAW_PREAMBLE";
    case eSIFT_PM4_OP_WRITE_DATA:
      return "WRITE_DATA";
    case eSIFT_PM4_OP_DRAW_INDEX_INDIRECT_MULTI:
      return "DRAW_INDEX_INDIRECT_MULTI";
    case eSIFT_PM4_OP_MEM_SEMAPHORE:
      return "MEM_SEMAPHORE";
    case eSIFT_PM4_OP_COPY_DW:
      return "COPY_DW";
    case eSIFT_PM4_OP_WAIT_REG_MEM:
      return "WAIT_REG_MEM";
    case eSIFT_PM4_OP_INDIRECT_BUFFER:
      return "INDIRECT_BUFFER";
    case eSIFT_PM4_OP_COPY_DATA:
      return "COPY_DATA";
    case eSIFT_PM4_OP_PFP_SYNC_ME:
      return "PFP_SYNC_ME";
    case eSIFT_PM4_OP_SURFACE_SYNC:
      return "SURFACE_SYNC";
    case eSIFT_PM4_OP_COND_WRITE:
      return "COND_WRITE";
    case eSIFT_PM4_OP_EVENT_WRITE:
      return "EVENT_WRITE";
    case eSIFT_PM4_OP_EVENT_WRITE_EOP:
      return "EVENT_WRITE_EOP";
    case eSIFT_PM4_OP_EVENT_WRITE_EOS:
      return "EVENT_WRITE_EOS";
    case eSIFT_PM4_OP_RELEASE_MEM:
      return "RELEASE_MEM";
    case eSIFT_PM4_OP_PREAMBLE_CNTL:
      return "PREAMBLE_CNTL";
    case eSIFT_PM4_OP_DMA_DATA:
      return "DMA_DATA";
    case eSIFT_PM4_OP_ACQUIRE_MEM:
      return "ACQUIRE_MEM";
    case eSIFT_PM4_OP_REWIND:
      return "REWIND";
    case eSIFT_PM4_OP_LOAD_UCONFIG_REG:
      return "LOAD_UCONFIG_REG";
    case eSIFT_PM4_OP_LOAD_SH_REG:
      return "LOAD_SH_REG";
    case eSIFT_PM4_OP_LOAD_CONFIG_REG:
      return "LOAD_CONFIG_REG";
    case eSIFT_PM4_OP_LOAD_CONTEXT_REG:
      return "LOAD_CONTEXT_REG";
    case eSIFT_PM4_OP_SET_CONFIG_REG:
      return "SET_CONFIG_REG";
    case eSIFT_PM4_OP_SET_CONTEXT_REG:
      return "SET_CONTEXT_REG";
    case eSIFT_PM4_OP_SET_CONTEXT_REG_INDIRECT:
      return "SET_CONTEXT_REG_INDIRECT";
    case eSIFT_PM4_OP_SET_SH_REG:
      return "SET_SH_REG";
    case eSIFT_PM4_OP_SET_SH_REG_OFFSET:
      return "SET_SH_REG_OFFSET";
    case eSIFT_PM4_OP_SET_QUEUE_REG:
      return "SET_QUEUE_REG";
    case eSIFT_PM4_OP_SET_UCONFIG_REG:
      return "SET_UCONFIG_REG";
    case eSIFT_PM4_OP_SCRATCH_RAM_WRITE:
      return "SCRATCH_RAM_WRITE";
    case eSIFT_PM4_OP_SCRATCH_RAM_READ:
      return "SCRATCH_RAM_READ";
    case eSIFT_PM4_OP_LOAD_CONST_RAM:
      return "LOAD_CONST_RAM";
    case eSIFT_PM4_OP_WRITE_CONST_RAM:
      return "WRITE_CONST_RAM";
    case eSIFT_PM4_OP_DUMP_CONST_RAM:
      return "DUMP_CONST_RAM";
    case eSIFT_PM4_OP_INCREMENT_CE_COUNTER:
      return "INCREMENT_CE_COUNTER";
    case eSIFT_PM4_OP_INCREMENT_DE_COUNTER:
      return "INCREMENT_DE_COUNTER";
    case eSIFT_PM4_OP_WAIT_ON_CE_COUNTER:
      return "WAIT_ON_CE_COUNTER";
    case eSIFT_PM4_OP_WAIT_ON_DE_COUNTER_DIFF:
      return "WAIT_ON_DE_COUNTER_DIFF";
    case eSIFT_PM4_OP_SWITCH_BUFFER:
      return "SWITCH_BUFFER";
    case eSIFT_PM4_OP_SET_RESOURCES:
      return "SET_RESOURCES";
    case eSIFT_PM4_OP_MAP_PROCESS:
      return "MAP_PROCESS";
    case eSIFT_PM4_OP_MAP_QUEUES:
      return "MAP_QUEUES";
    case eSIFT_PM4_OP_UNMAP_QUEUES:
      return "UNMAP_QUEUES";
    case eSIFT_PM4_OP_QUERY_STATUS:
      return "QUERY_STATUS";
    case eSIFT_PM4_OP_RUN_LIST:
      return "RUN_LIST";
    default:
      return "UNKONWN";
  }
}

void parse_pm4_type1(sift_pm4_runlist_entry_t* e) { printf("TYPE1: SKIPPING\n"); }

void parse_pm4_type2(sift_pm4_runlist_entry_t* e) { printf("TYPE2: SKIPPING\n"); }

void parse_pm4_type3(sift_pm4_runlist_entry_t* e)
{
  switch (e->header.opcode) {
    case eSIFT_PM4_OP_MAP_PROCESS:
      printf("TYPE3: %s\n", pm4_op_to_str((sift_pm4_pm4_opcode_t)e->header.opcode));
      printf("\t single_memop %d\n", e->body->map_process.single_memop);
      printf("\t debug_flag %d\n", e->body->map_process.debug_flag);
      printf("\t tmz %d\n", e->body->map_process.tmz);
      printf("\t diq_enable %d\n", e->body->map_process.diq_enable);
      printf("\t sdma_enable %d\n", e->body->map_process.sdma_enable);
      printf("\t pasid 0x%x\n", e->body->map_process.pasid);
      printf("\t debug_vmid %d\n", e->body->map_process.debug_vmid);
      printf("\t process_quantum %d\n", e->body->map_process.process_quantum);
      printf("\t pt_base_addr_lo32 0x%x\n",
             e->body->map_process.vm_context_page_table_base_addr_lo32);
      printf("\t pt_base_addr_hi32 0x%x\n",
             e->body->map_process.vm_context_page_table_base_addr_hi32);
      break;
    default:
      printf("TYPE3: %s\n", pm4_op_to_str((sift_pm4_pm4_opcode_t)e->header.opcode));
  }
}

int main(int argc, char** argv)
{
  CLI::App app;
  CLI11_PARSE(app, argc, argv);

  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "Failed to initialize sift");
    return -1;
  }

  sift_pm4_runlist_series_t* series;
  rc = sift_kfd_get_runlists(&series);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr,
            "Failed to get runlists (are any active?), please make sure you are running as "
            "root!\n");
    return -1;
  }
  sift_pm4_runlist_series_t* current = series;
  while (current) {
    sift_pm4_runlist_t* rls = current->runlist;
    printf("Node %d GPU_ID %x %ld entries\n", rls->node.node_id, rls->node.gpu_id,
           rls->num_entries);
    for (size_t n = 0; n < rls->num_entries; n++) {
      sift_pm4_runlist_entry_t* ent = &rls->entries[n];
      switch (ent->header.type) {
        case eSIFT_PM4_TYPE1:
          parse_pm4_type1(ent);
          break;
        case eSIFT_PM4_TYPE2:
          parse_pm4_type2(ent);
          break;
        case eSIFT_PM4_TYPE3:
          parse_pm4_type3(ent);
          break;
      }
    }
    current = current->next;
  }

  rc = sift_pm4_destroy_runlists(series);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to destroy runlists");
    return -1;
  }

  rc = sift_destroy();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to close sift");
    return -1;
  }

  return 0;
}
