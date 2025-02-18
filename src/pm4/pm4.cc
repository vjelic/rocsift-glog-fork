/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <cstdlib>
#include <exception>
#include <string>
#include <regex>
#include <vector>

#include "rocsift/pm4.h"
#include "rocsift/status.h"

#include "../sift/logging.h"
#include "pm4.h"
#include "span.h"
#include "rls_parser.h"

using namespace sift::pm4;

namespace
{
const std::regex node_format{R"(\s*Node (\d+), gpu_id ([a-fA-F0-9]+):\s*)"};
const std::regex data_format{R"([a-fA-F0-9]{8})"};
const std::regex data_start_format{R"((^\s*[a-fA-F0-9]{8}:).*)"};

constexpr uint64_t slice(uint64_t val, int msb, int lsb)
{
  uint64_t mask = ((1ul << msb) - 1u) * 2 + 1;
  return (val & mask) >> lsb;
}
};  // namespace

ResultStatus<sift_pm4_node_t> sift::pm4::parseNodeLine(const std::string &node_line)
{
  {
    try {
      std::smatch m;
      if (std::regex_match(node_line, m, node_format)) {
        return {Status::SUCCESS,
                sift_pm4_node_t{.node_id = static_cast<uint32_t>(std::stoul(m[1])),
                                .gpu_id = static_cast<uint32_t>(std::stoul(m[2], nullptr, 16))}};
      } else {
        return {Status::NO_MATCH, std::nullopt};
      }
    } catch (std::exception &e) {
      Logger()->error("Error parsing node line: {}", node_line);
      Logger()->error("Exception: {}", e.what());
      return {Status::ERROR, std::nullopt};
    };
  }
}

ResultStatus<size_t> sift::pm4::getDataStartPosition(const std::string &address_line)
{
  try {
    std::cmatch m;
    if (std::regex_match(address_line.data(), m, data_start_format)) {
      return {Status::SUCCESS, m[1].length()};
    } else {
      return {Status::NO_MATCH, std::nullopt};
    }
  } catch (std::exception &e) {
    Logger()->error("Error parsing address line: {}", address_line);
    Logger()->error("Exception: {}", e.what());
    return {Status::ERROR, std::nullopt};
  };
}

ResultStatus<std::vector<uint32_t>> sift::pm4::parseDataSection(const std::string &data_section)
{
  try {
    auto begin = std::sregex_iterator(data_section.cbegin(), data_section.cend(), data_format);
    auto end = std::sregex_iterator();
    size_t n = std::distance(begin, end);
    if (n == 0) {
      return {Status::SUCCESS, std::vector<uint32_t>{}};
    }

    std::vector<uint32_t> data(n);
    int i = 0;
    for (std::sregex_iterator iter = begin; iter != end; iter++) {
      std::smatch match = *iter;
      data[i] = static_cast<uint32_t>(std::stoul(match.str(), nullptr, 16));
      i++;
    }
    return {Status::SUCCESS, data};
  } catch (std::exception &e) {
    Logger()->error("Error parsing data section: {}", data_section);
    Logger()->error("Exception: {}", e.what());
    return {Status::ERROR, std::nullopt};
  };
}

ResultStatus<sift_pm4_runlist_entry_t> sift::pm4::parseRunlistEntry(span<const uint32_t> data)
{
  sift_pm4_runlist_entry_t entry;
  try {
    if (data.empty()) {
      Logger()->error("Parsing runlist entry on empty data");
      return {Status::ERROR, std::nullopt};
    }
    uint32_t header = data[0];
    uint32_t reserved = slice(header, 7, 0);
    if (reserved != 0) {
      Logger()->error("Header parsing error in RESERVED: {:#08x} != 0", reserved);
    }
    entry.header.opcode = slice(header, 15, 8);
    entry.header.count = slice(header, 29, 16) + 1;  // count is N-1, so add 1 to get N
    uint64_t type = slice(header, 31, 30) - 1;       // Enum starts at 0, but PM4 type starts at 1
    if (type > 2) {
      Logger()->error("Invalid PM4 packet type in header: {}", type);
      return {Status::ERROR, std::nullopt};
    }
    entry.header.type = static_cast<sift_pm4_packet_type_t>(type);

    // Validate the header + body can fit in the input data
    if (data.size() - 1 < entry.header.count) {
      Logger()->error("Data is undersized for op {:#x}", entry.header.opcode);
      return {Status::ERROR, std::nullopt};
    }

    entry.body = static_cast<sift_pm4_body_t *>(std::malloc(sizeof(sift_pm4_body_t)));
    if (!entry.body) {
      Logger()->error("Failed to allocate memory for PM4 packet body");
      return {Status::ERROR, std::nullopt};
    }

    const auto body_data = data.subspan(1, data.size() - 1);
    switch (entry.header.opcode) {
      case eSIFT_PM4_OP_MAP_PROCESS: {
        entry.body->map_process = parseBodyMapProcess(body_data);
        break;
      }
      case eSIFT_PM4_OP_MAP_QUEUES: {
        entry.body->map_queues = parseBodyMapQueues(body_data);
        break;
      }
      default: {
        Logger()->warn("Skipping body of unsupported op code: {:#08x}", entry.header.opcode);
        std::free(entry.body);
        return {Status::ERROR, std::nullopt};
      }
    }
    return {Status::SUCCESS, entry};
  } catch (std::exception &e) {
    Logger()->error("Exception: {}", e.what());
    if (entry.body) {
      std::free(entry.body);
    }
    return {Status::ERROR, std::nullopt};
  };
}

Status sift::pm4::destroyRunlistEntry(sift_pm4_runlist_entry_t &entry) noexcept
{
  std::free(entry.body);
  return Status::SUCCESS;
}

ResultStatus<std::vector<sift_pm4_runlist_entry_t>> sift::pm4::parseRunlistEntries(
    span<const uint32_t> data)
{
  try {
    if (data.size() == 0) {
      Logger()->error("Tried to parse empty packet data.");
      return {Status::ERROR, std::nullopt};
    }
    // Count num entries in data
    std::vector<size_t> entry_sizes;
    // Sanity check that the entries actually fit in the provided
    // input data.
    size_t total_entries_size = 0;
    while (total_entries_size < data.size()) {
      // Bits[29:16] in the header provide number of dwords-1 in the body.
      size_t entry_body_size = slice(data[total_entries_size], 29, 16) + 1;
      size_t entry_size = entry_body_size + 1;
      total_entries_size += entry_size;
      entry_sizes.push_back(entry_size);
    }
    if (total_entries_size > data.size()) {
      Logger()->error(
          "Invalid packet data for parsing. Output packet size and input data size mismatch.");
      return {Status::ERROR, std::nullopt};
    }

    std::vector<sift_pm4_runlist_entry_t> entries(entry_sizes.size());
    size_t start = 0;
    for (size_t i = 0; i < entries.size(); i++) {
      size_t entry_size = entry_sizes[i];
      auto result = parseRunlistEntry(data.subspan(start, entry_size));
      if (result.status != Status::SUCCESS) {
        Logger()->error("Failed to parse header: {:#08x}", data[start]);
        return {Status::ERROR, std::nullopt};
      }
      entries[i] = result.value.value();
      if (entries[i].header.type != eSIFT_PM4_TYPE3) {
        Logger()->error("Unsupported PM4 Packet: TYPE{}. Only TYPE3 is supported.",
                        static_cast<uint64_t>(entries[i].header.type));
      }
      start += entry_size;  // start of next header
    }
    return {Status::SUCCESS, entries};
  } catch (std::exception &e) {
    Logger()->error("Exception: {}", e.what());
    return {Status::ERROR, std::nullopt};
  };
}
Status sift::pm4::destroyRunlistEntries(sift_pm4_runlist_entry_t *entries, size_t n) noexcept
{
  auto status = Status::SUCCESS;
  for (size_t i = 0; i < n; i++) {
    auto &entry = entries[i];
    if (Status::SUCCESS != destroyRunlistEntry(entry)) {
      status = Status::ERROR;
    }
  }
  std::free(entries);
  return status;
}

namespace
{
constexpr uint64_t getBodySlice(span<const uint32_t> body_data, uint64_t index, int msb, int lsb)
{
  // Subtract 2 from dword index to make it easier to match
  // the cp packet format table
  return slice(body_data[index - 2], msb, lsb);
}
};  // namespace

sift_pm4_body_map_process_t sift::pm4::parseBodyMapProcess(span<const uint32_t> body_data)
{
  sift_pm4_body_map_process_t b;
  b.pasid = getBodySlice(body_data, 2, 15, 0);
  b.single_memop = getBodySlice(body_data, 2, 16, 16);
  b.debug_vmid = getBodySlice(body_data, 2, 21, 18);
  b.debug_flag = getBodySlice(body_data, 2, 22, 22);
  b.tmz = getBodySlice(body_data, 2, 23, 23);
  b.diq_enable = getBodySlice(body_data, 2, 24, 24);
  b.process_quantum = getBodySlice(body_data, 2, 31, 25);
  b.vm_context_page_table_base_addr_lo32 = getBodySlice(body_data, 3, 31, 0);
  b.vm_context_page_table_base_addr_hi32 = getBodySlice(body_data, 4, 31, 0);
  b.sh_mem_bases = getBodySlice(body_data, 5, 31, 0);
  b.sh_mem_config = getBodySlice(body_data, 6, 31, 0);
  b.sq_shader_tba_lo = getBodySlice(body_data, 7, 31, 0);
  b.sq_shader_tba_hi = getBodySlice(body_data, 8, 31, 0);
  b.sq_shader_tma_lo = getBodySlice(body_data, 9, 31, 0);
  b.sq_shader_tma_hi = getBodySlice(body_data, 10, 31, 0);
  b.gds_addr_lo = getBodySlice(body_data, 12, 31, 0);
  b.gds_addr_hi = getBodySlice(body_data, 13, 31, 0);
  b.num_gws = getBodySlice(body_data, 14, 6, 0);
  b.sdma_enable = getBodySlice(body_data, 14, 7, 0);
  b.num_oac = getBodySlice(body_data, 14, 11, 8);
  b.gds_size_hi = getBodySlice(body_data, 14, 15, 12);
  b.gds_size_lo = getBodySlice(body_data, 14, 21, 16);
  b.num_queues = getBodySlice(body_data, 14, 31, 22);
  b.spi_gdbg_per_vmid_cntl = getBodySlice(body_data, 15, 31, 0);
  b.tcp_watch0_cntl = getBodySlice(body_data, 16, 31, 0);
  b.tcp_watch1_cntl = getBodySlice(body_data, 17, 31, 0);
  b.tcp_watch2_cntl = getBodySlice(body_data, 18, 31, 0);
  b.tcp_watch3_cntl = getBodySlice(body_data, 19, 31, 0);
  b.completion_signal_lo32 = getBodySlice(body_data, 20, 31, 0);
  b.completion_signal_hi32 = getBodySlice(body_data, 21, 31, 0);
  return b;
}

sift_pm4_body_map_queues_t sift::pm4::parseBodyMapQueues(span<const uint32_t> body_data)
{
  sift_pm4_body_map_queues_t b;
  b.extended_engine_sel = getBodySlice(body_data, 2, 3, 2);
  b.queue_sel = getBodySlice(body_data, 2, 5, 4);
  b.vmid = getBodySlice(body_data, 2, 11, 8);
  b.gws_enabled = getBodySlice(body_data, 2, 12, 12);
  b.queue = getBodySlice(body_data, 2, 20, 13);
  b.queue_type = getBodySlice(body_data, 2, 23, 21);
  b.static_queue_group = getBodySlice(body_data, 2, 25, 24);
  b.engine_sel = getBodySlice(body_data, 2, 28, 26);
  b.num_queues = getBodySlice(body_data, 2, 31, 29);
  b.check_disable = getBodySlice(body_data, 3, 1, 1);
  b.doorbell_offset = getBodySlice(body_data, 3, 27, 2);
  b.mqd_addr_lo = getBodySlice(body_data, 4, 31, 0);
  b.mqd_addr_hi = getBodySlice(body_data, 5, 31, 0);
  b.wptr_addr_lo = getBodySlice(body_data, 6, 31, 0);
  b.wptr_addr_hi = getBodySlice(body_data, 7, 31, 0);
  return b;
}

bool allocateEntries(sift_pm4_runlist_t *runlist,
                     const std::vector<sift_pm4_runlist_entry_t> &entries)
{
  runlist->entries = static_cast<sift_pm4_runlist_entry_t *>(
      std::malloc(entries.size() * sizeof(sift_pm4_runlist_entry_t)));
  if (runlist->entries == nullptr) {
    return false;
  }
  std::copy(entries.cbegin(), entries.cend(), runlist->entries);
  runlist->num_entries = entries.size();
  return true;
}

sift_status_t sift_pm4_parse_runlist(int node, int gpu, const char *runlist_file_contents,
                                     sift_pm4_runlist_t *runlist)
{
  try {
    RlsParser parser{runlist_file_contents, node, gpu};
    auto result = parser.Parse();
    if (!result.has_value()) {
      return SIFT_STATUS_ERROR;
    }
    auto &entries = result.value();
    runlist->node.gpu_id = gpu;
    runlist->node.node_id = node;
    if (gpu != parser.Gpu()) {
      sift::Logger()->error("Code bug: parser got wrong GPU ID {:#x} instead of {:#x}",
                            parser.Gpu(), gpu);
      return SIFT_STATUS_ERROR;
    }
    if (node != parser.Node()) {
      sift::Logger()->error("Code bug: parser got wrong Node ID: {} instead of {}", parser.Node(),
                            node);
      return SIFT_STATUS_ERROR;
    }
    if (entries.empty()) {
      runlist->entries = nullptr;
      runlist->num_entries = 0;
    } else {
      if (!allocateEntries(runlist, entries)) {
        sift::Logger()->error("Failed to allocate runlist entries for runlist:\n{}",
                              runlist_file_contents);
        return SIFT_STATUS_ERROR;
      }
    }
  } catch (std::exception &e) {
    sift::Logger()->error("Exception: {}", e.what());
    return SIFT_STATUS_ERROR;
  };
  return SIFT_STATUS_SUCCESS;
}

sift_status_t sift_pm4_parse_runlists(const char *runlist_file_contents,
                                      sift_pm4_runlist_series_t **runlists)
{
  sift_pm4_runlist_series_t *current = nullptr;
  try {
    *runlists = nullptr;
    sift_pm4_runlist_series_t *prev = nullptr;

    RlsParser parser{runlist_file_contents};
    while (auto result = parser.Parse()) {
      auto &entries = result.value();

      current = static_cast<sift_pm4_runlist_series_t *>(
          std::malloc(sizeof(sift_pm4_runlist_series_t)));
      if (current == nullptr) {
        sift::Logger()->error("Failed to allocate new runlist node in list.");
        throw std::runtime_error("Failed to allocate runlist node");
      }
      if (*runlists == nullptr) {
        // first node, head has not been set.
        // set head
        *runlists = current;
      }

      current->next = nullptr;
      current->runlist = static_cast<sift_pm4_runlist_t *>(std::malloc(sizeof(sift_pm4_runlist_t)));
      if (!current->runlist) {
        sift::Logger()->error("Failed to allocate runlist:\n{}", runlist_file_contents);
        throw std::runtime_error("Failed to allocate runlist");
      }

      current->runlist->num_entries = 0;
      current->runlist->entries = nullptr;
      if (!allocateEntries(current->runlist, entries)) {
        sift::Logger()->error("Failed to allocate runlist entries for runlist:\n{}",
                              runlist_file_contents);
        throw std::runtime_error("Failed to allocate runlist entries");
      }
      current->runlist->node.gpu_id = parser.Gpu();
      current->runlist->node.node_id = parser.Node();

      if (prev != nullptr) {
        // if not the first node, set prev nodes next ptr
        prev->next = current;
      }
      prev = current;
    }
  } catch (std::exception &e) {
    sift::Logger()->error("Exception: {}", e.what());
    if (current != nullptr) {
      sift_pm4_destroy_runlists(current);
    }
    if (*runlists != nullptr && current != *runlists) {
      // Current was not the head node, so free the rest of the list
      sift_pm4_destroy_runlists(*runlists);
    }
    return SIFT_STATUS_ERROR;
  };
  return SIFT_STATUS_SUCCESS;
}

sift_status_t sift_pm4_get_runlist(int node, int gpu, sift_pm4_runlist_t *runlist,
                                   const char *runlist_file_path)
{
  try {
    std::ifstream f(runlist_file_path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    auto str = buffer.str();

    sift::Logger()->debug("node {} gpu_id {} runlist:\n{}", node, gpu, str);

    return sift_pm4_parse_runlist(node, gpu, str.c_str(), runlist);
  } catch (std::exception &e) {
    sift::Logger()->error("Exception: {}", e.what());
    return SIFT_STATUS_ERROR;
  };
}

sift_status_t sift_pm4_get_runlists(const char *runlist_file_path,
                                    sift_pm4_runlist_series_t **runlists)
{
  try {
    std::ifstream f(runlist_file_path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    auto str = buffer.str();

    sift::Logger()->debug("runlist:\n{}", str);

    return sift_pm4_parse_runlists(str.c_str(), runlists);
  } catch (std::exception &e) {
    sift::Logger()->error("Exception: {}", e.what());
    return SIFT_STATUS_ERROR;
  };
}

sift_status_t sift_pm4_destroy_runlist(sift_pm4_runlist_t *runlist)
{
  if (!runlist) {
    sift::Logger()->error("Attempted to destroy a null runlist");
    return SIFT_STATUS_ERROR;
  }
  if (Status::SUCCESS != destroyRunlistEntries(runlist->entries, runlist->num_entries)) {
    return SIFT_STATUS_ERROR;
  }
  return SIFT_STATUS_SUCCESS;
}

sift_status_t sift_pm4_destroy_runlists(sift_pm4_runlist_series_t *runlists)
{
  try {
    if (!runlists) {
      sift::Logger()->error("Attempted to destroy a null runlist series");
      return SIFT_STATUS_ERROR;
    }

    auto *current = runlists;
    sift_pm4_runlist_series_t *next = nullptr;
    while (current != nullptr) {
      next = current->next;
      sift_status_t rc;
      rc = sift_pm4_destroy_runlist(current->runlist);
      if (rc != SIFT_STATUS_SUCCESS) {
        sift::Logger()->error("Failed to destroy a runlist in list");
        return rc;
      }
      std::free(current->runlist);
      std::free(current);
      current = next;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception &e) {
    sift::Logger()->error("Exception: {}", e.what());
    return SIFT_STATUS_ERROR;
  };
}
