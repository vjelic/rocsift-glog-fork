/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#define CATCH_CONFIG_MAIN
#include <fstream>
#include <memory>
#include <ostream>
#include <vector>

#include "catch2/catch.hpp"

#include "rocsift/pm4.h"
#include "rocsift/status.h"

#include "pm4/pm4.h"

using namespace sift::pm4;

TEST_CASE("Parse Node")
{
  SECTION("well formed")
  {
    auto r = parseNodeLine("Node 2, gpu_id c9e7:");
    REQUIRE(Status::SUCCESS == r.status);
    auto &node = r.value.value();
    REQUIRE(node.gpu_id == 0xc9e7);
    REQUIRE(node.node_id == 2);
    r = parseNodeLine("Node 999999, gpu_id ffff:");
    REQUIRE(Status::SUCCESS == r.status);
  }
  SECTION("invalid format")
  {
    auto r = parseNodeLine("Node 2, gpu_id QQQQQQ:");
    REQUIRE(Status::SUCCESS != r.status);
    r = parseNodeLine("Node 2, cpu_id c9e7:");
    REQUIRE(Status::SUCCESS != r.status);
    r = parseNodeLine("Node A, cpu_id c9e7:");
    REQUIRE(Status::SUCCESS != r.status);
    r = parseNodeLine("");
    REQUIRE(Status::SUCCESS != r.status);
    r = parseNodeLine("00000020: 00000000 00000000");
    REQUIRE(Status::SUCCESS != r.status);
  }
}

TEST_CASE("Get data section start point")
{
  SECTION("well formed")
  {
    using test_case_pair = const std::pair<const char *, size_t>;
    auto test_case = GENERATE(test_case_pair{"  00000020: 00000000 00000000 00000000 00000000 "
                                             "00000000 c005a200 28000010 00004a80 ",
                                             11},
                              test_case_pair{"ffffffff:              00000000     ", 9},
                              test_case_pair{"ffffffff:00000000     ", 9},
                              test_case_pair{"00000000:", 9});
    auto r = getDataStartPosition(test_case.first);
    REQUIRE(Status::SUCCESS == r.status);
    REQUIRE(test_case.second == r.value.value());
  }
  SECTION("invalid")
  {
    size_t counted_test_case = 0;
    auto test_case = GENERATE("0000ffff1: 00000000", "0000fff:", "", "0000ffff 00000000",
                              "0000GGGG: 00000000");
    auto r = getDataStartPosition(test_case);
    REQUIRE(Status::SUCCESS != r.status);
  }
}

TEST_CASE("Get dWords from address line")
{
  using test_case_pair = const std::pair<const char *, std::vector<uint32_t> >;
  auto test_case = GENERATE(
      test_case_pair{"00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00004a80",
                     {0, 0, 0, 0, 0, 0xc005a200, 0x28000010, 0x00004a80}});
  auto r = parseDataSection(test_case.first);
  REQUIRE(Status::SUCCESS == r.status);
  REQUIRE(test_case.second == r.value.value());
}

TEST_CASE("entries")
{
  constexpr size_t num_entries = 3;
  constexpr size_t num_dwords = 35;
  constexpr std::array<uint32_t, num_dwords> data = {0xc013a100, 0x14008000, 0x41875003, 0x00000001,
                                                     0x00010002, 0x00001118, 0x00000020, 0x00000000,
                                                     0x00000030, 0x00000000, 0x00000000, 0x00000000,
                                                     0x00000000, 0x00800080, 0x00000008, 0x00000000,
                                                     0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                     0x00000000, 0xc005a200, 0x28000010, 0x00008800,
                                                     0x0171a000, 0x00000000, 0x0ce9c008, 0x00007f4c,
                                                     0xc005a200, 0x20000010, 0x00008000, 0x0173b000,
                                                     0x00000000, 0x0cf32038, 0x00007f4c};
  SECTION("Parse single entry")
  {
    auto r = parseRunlistEntry({data.data(), data.size()});
    REQUIRE(Status::SUCCESS == r.status);
    auto &entry = r.value.value();
    REQUIRE(entry.header.type == eSIFT_PM4_TYPE3);
    REQUIRE(entry.header.opcode == eSIFT_PM4_OP_MAP_PROCESS);
    REQUIRE(entry.header.count == 20);
    REQUIRE(Status::SUCCESS == destroyRunlistEntry(entry));
  }
  SECTION("Parse entries")
  {
    auto r = parseRunlistEntries({data.data(), data.size()});
    REQUIRE(Status::SUCCESS == r.status);
    auto &entries = r.value.value();
    REQUIRE(num_entries > 0);
    REQUIRE(entries[0].header.type == eSIFT_PM4_TYPE3);
    REQUIRE(entries[1].header.type == eSIFT_PM4_TYPE3);
    REQUIRE(entries[2].header.type == eSIFT_PM4_TYPE3);
    REQUIRE(entries[0].header.opcode == eSIFT_PM4_OP_MAP_PROCESS);
    REQUIRE(entries[1].header.opcode == eSIFT_PM4_OP_MAP_QUEUES);
    REQUIRE(entries[2].header.opcode == eSIFT_PM4_OP_MAP_QUEUES);
    REQUIRE(entries[0].header.count == 20);
    REQUIRE(entries[1].header.count == 6);
    REQUIRE(entries[2].header.count == 6);
    REQUIRE(Status::SUCCESS == destroyRunlistEntry(entries[0]));
    REQUIRE(Status::SUCCESS == destroyRunlistEntry(entries[1]));
    REQUIRE(Status::SUCCESS == destroyRunlistEntry(entries[2]));
  }
}

constexpr int num_texts = 3;
constexpr int num_nodes = 3;

void verify_runlist(uint32_t node, sift_pm4_runlist_t &runlist)
{
  REQUIRE(runlist.num_entries > 0);
  REQUIRE(nullptr != runlist.entries);
  switch (node) {
    case 1: {
      REQUIRE(runlist.node.gpu_id == 0x1576);
      REQUIRE(runlist.node.node_id == 1);
      {
        auto &entry = runlist.entries[0];
        REQUIRE(entry.header.type == eSIFT_PM4_TYPE3);
        REQUIRE(entry.header.opcode == eSIFT_PM4_OP_MAP_PROCESS);
        REQUIRE(entry.header.count == 20);
        auto *b = reinterpret_cast<sift_pm4_body_map_process_t *>(entry.body);
        REQUIRE(b->pasid == 0x8000);
        REQUIRE(b->single_memop == 0x0);
        REQUIRE(b->debug_vmid == 0x0);
        REQUIRE(b->debug_flag == 0x0);
        REQUIRE(b->tmz == 0x0);
        REQUIRE(b->diq_enable == 0x0);
        REQUIRE(b->process_quantum == 0xA);
        REQUIRE(b->vm_context_page_table_base_addr_lo32 == 0x41875003);
        REQUIRE(b->vm_context_page_table_base_addr_hi32 == 0x00000001);
        REQUIRE(b->sh_mem_bases == 0x00010002);
        REQUIRE(b->sh_mem_config == 0x00001118);
        REQUIRE(b->sq_shader_tba_lo == 0x00000020);
        REQUIRE(b->sq_shader_tba_hi == 0x0);
        REQUIRE(b->sq_shader_tma_lo == 0x00000030);
        REQUIRE(b->sq_shader_tma_hi == 0x0);
        REQUIRE(b->gds_addr_lo == 0x0);
        REQUIRE(b->gds_addr_hi == 0x0);
        REQUIRE(b->num_gws == 0x0);
        REQUIRE(b->sdma_enable == 0x1);
        REQUIRE(b->num_oac == 0x0);
        REQUIRE(b->gds_size_hi == 0x0);
        REQUIRE(b->gds_size_lo == 0x0);
        REQUIRE(b->num_queues == 0x2);
        REQUIRE(b->spi_gdbg_per_vmid_cntl == 0x00000008);
        REQUIRE(b->tcp_watch0_cntl == 0x0);
        REQUIRE(b->tcp_watch1_cntl == 0x0);
        REQUIRE(b->tcp_watch2_cntl == 0x0);
        REQUIRE(b->tcp_watch3_cntl == 0x0);
        REQUIRE(b->completion_signal_lo32 == 0x0);
        REQUIRE(b->completion_signal_hi32 == 0x0);
      }
      {
        auto &entry = runlist.entries[1];
        REQUIRE(entry.header.type == eSIFT_PM4_TYPE3);
        REQUIRE(entry.header.opcode == eSIFT_PM4_OP_MAP_QUEUES);
        REQUIRE(entry.header.count == 6);
        auto b = reinterpret_cast<sift_pm4_body_map_queues_t *>(entry.body);
        REQUIRE(b->extended_engine_sel == 0x0);  // dw(2, 3, 2);
        REQUIRE(b->queue_sel == 0x1);            // dw(2, 5, 4);
        REQUIRE(b->vmid == 0x0);                 // dw(2, 11, 8);
        REQUIRE(b->gws_enabled == 0x0);          // dw(2, 12, 12);
        REQUIRE(b->queue == 0x0);                // dw(2, 20, 13);
        REQUIRE(b->queue_type == 0x0);           // dw(2, 23, 21);
        REQUIRE(b->static_queue_group == 0x0);   // dw(2, 25, 24);
        REQUIRE(b->engine_sel == 0x2);           // dw(2, 28, 26);
        REQUIRE(b->num_queues == 0x1);           // dw(2, 31, 29);
        REQUIRE(b->check_disable == 0x0);        // dw(3, 1, 1);
        REQUIRE(b->doorbell_offset == 0x2200);   // dw(3, 27, 2);
        REQUIRE(b->mqd_addr_lo == 0x0171a000);   // dw(4, 31, 0);
        REQUIRE(b->mqd_addr_hi == 0x0);          // dw(5, 31, 0);
        REQUIRE(b->wptr_addr_lo == 0x0ce9c008);  // dw(6, 31, 0);
        REQUIRE(b->wptr_addr_hi == 0x00007f4c);  // dw(7, 31, 0);
      }
      break;
    }
    case 2: {
      REQUIRE(runlist.node.gpu_id == 0xc9e7);
      REQUIRE(runlist.node.node_id == 2);
      break;
    }
    case 3: {
      REQUIRE(runlist.node.gpu_id == 0x8a48);
      REQUIRE(runlist.node.node_id == 3);
      break;
    }
    default: {
      throw std::runtime_error("undefined verify_runlist cast for runlist");
    }
  };
}

void verify_runlists(const sift_pm4_runlist_series_t *runlists)
{
  auto current = runlists;
  for (int i = 0; i < num_nodes; i++) {
    verify_runlist(current->runlist->node.node_id, *(current->runlist));
    if (current->next != nullptr) {
      current = current->next;
    }
  }
}

constexpr std::array<const char *, num_texts> rls_texts{
    R"(Node 1, gpu_id 1576:
  00000000: c013a100 14008000 41875003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00008800
  00000060: 0171a000 00000000 0ce9c008 00007f4c c005a200 20000010 00008000 0173b000
  00000080: 00000000 0cf32038 00007f4c
Node 2, gpu_id c9e7:
  00000000: c013a100 14008000 3009d003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00006940
  00000060: 0171f000 00000000 0ce9a008 00007f4c c005a200 20000010 00006000 01733000
  00000080: 00000000 0cf48038 00007f4c
Node 3, gpu_id 8a48:
  00000000: c013a100 14008000 46190003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000 	
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00004a80
  00000060: 01724000 00000000 0cea6008 00007f4c c005a200 20000010 00004000 0172b000
  00000080: 00000000 0d288038 00007f4c
  )",

    R"(
Node 1, gpu_id 1576:
  00000000: c013a100 14008000 41875003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00008800
  00000060: 0171a000 00000000 0ce9c008 00007f4c c005a200 20000010 00008000 0173b000
  00000080: 00000000 0cf32038 00007f4c
Node 2, gpu_id c9e7:
  00000000: c013a100 14008000 3009d003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00006940
  00000060: 0171f000 00000000 0ce9a008 00007f4c c005a200 20000010 00006000 01733000
  00000080: 00000000 0cf48038 00007f4c




Node 3, gpu_id 8a48:
  00000000: c013a100 14008000 46190003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000 	
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00004a80
  00000060: 01724000 00000000 0cea6008 00007f4c c005a200 20000010 00004000 0172b000
  00000080: 00000000 0d288038 00007f4c

  )",
    R"(
JUNK
FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFfff
*********************************************



Node 1, gpu_id 1576:
  00000000: c013a100 14008000 41875003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00008800
  00000060: 0171a000 00000000 0ce9c008 00007f4c c005a200 20000010 00008000 0173b000
  00000080: 00000000 0cf32038 00007f4c

lorem ipsum, etc.
Node 2, gpu_id c9e7:
  00000000: c013a100 14008000 3009d003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00006940
  00000060: 0171f000 00000000 0ce9a008 00007f4c c005a200 20000010 00006000 01733000
  00000080: 00000000 0cf48038 00007f4c
Node 3, gpu_id 8a48:
  00000000: c013a100 14008000 46190003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000 	
  00000040: 00000000 00000000 00000000 00000000 00000000 c005a200 28000010 00004a80
  00000060: 01724000 00000000 0cea6008 00007f4c c005a200 20000010 00004000 0172b000
  00000080: 00000000 0d288038 00007f4c

)"};

TEST_CASE("runlists")
{
  SECTION("well formed")
  {
    int text_index = GENERATE_COPY(range(0, num_texts - 1));
    const char *rls_text = rls_texts[text_index];
    SECTION("parse runlist")
    {
      sift_pm4_runlist_t runlist;
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_parse_runlist(1, 0x1576, rls_text, &runlist));
      verify_runlist(1, runlist);
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_parse_runlist(2, 0xc9e7, rls_text, &runlist));
      verify_runlist(2, runlist);
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_parse_runlist(3, 0x8a48, rls_text, &runlist));
      verify_runlist(3, runlist);
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
    }

    SECTION("parse_runlists")
    {
      sift_pm4_runlist_series_t *runlists = nullptr;
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_parse_runlists(rls_text, &runlists));
      size_t num_runlists = 0;
      for (auto *r = runlists; r != nullptr; r = r->next) {
        num_runlists++;
      }
      REQUIRE(num_runlists == num_nodes);
      REQUIRE(runlists != nullptr);
      verify_runlists(runlists);
      REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlists(runlists));
    }
    SECTION("runlist file")
    {
      const char *rls_file = "rls.txt";
      {
        std::ofstream out(rls_file);
        out << rls_text;
      }

      SECTION("get runlist")
      {
        sift_pm4_runlist_t runlist;
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_get_runlist(1, 0x1576, &runlist, rls_file));
        verify_runlist(1, runlist);
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_get_runlist(2, 0xc9e7, &runlist, rls_file));
        verify_runlist(2, runlist);
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_get_runlist(3, 0x8a48, &runlist, rls_file));
        verify_runlist(3, runlist);
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlist(&runlist));
      }
      SECTION("get runlists")
      {
        sift_pm4_runlist_series_t *runlists = nullptr;
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_get_runlists(rls_file, &runlists));
        size_t num_runlists = 0;
        for (auto *r = runlists; r != nullptr; r = r->next) {
          num_runlists++;
        }
        REQUIRE(num_runlists == num_nodes);
        REQUIRE(runlists != nullptr);
        verify_runlists(runlists);
        REQUIRE(SIFT_STATUS_SUCCESS == sift_pm4_destroy_runlists(runlists));
      }
    }
  }
  SECTION("invalid")
  {
    const char *rls_invalid_text = R"(
Node 1, gpu_id 1576:
  00000000: c013a100 14008000 41875003 00000001 00010002 00001118 00000020 00000000
  00000020: 00000030 00000000 00000000 00000000 00000000 00800080 00000008 00000000
  000000000000000000000000000000000000000000000000000000000000000000000000000000000
  00000060: 0171a000 00000000 0ce9c008 00007f4c c005a200 20000010 00008000 0173b000
  00000080: 00000000 0cf32038 00007f4c
)";
    sift_pm4_runlist_t runlist;
    REQUIRE(SIFT_STATUS_SUCCESS != sift_pm4_parse_runlist(1, 0x1576, rls_invalid_text, &runlist));
  }
}
