/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_PM4_PM4_H_
#define _SIFT_PM4_PM4_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "rocsift/pm4.h"

#include "span.h"

namespace sift
{
namespace pm4
{

/// @brief Return code for parsing actions
enum class Status { SUCCESS = 0, ERROR = 1, NO_MATCH = 2 };

/// @brief pair of Status and optional return value
/// @see Status
template <typename T>
struct ResultStatus {
  Status status;
  std::optional<T> value;
};

/// @brief extracts node id and gpu id from Node definition string
///
/// @param node_line node string
/// @returns Populated node struct
ResultStatus<sift_pm4_node_t> parseNodeLine(const std::string &node_line);

/// @brief finds start of data section in an address line
///
/// @param address_line address and data string
/// @returns Position of data section start
ResultStatus<size_t> getDataStartPosition(const std::string &address_line);

/// @brief extracts dwords from data section string
///
/// @param data_section data section string
/// @returns Vector of dwords
ResultStatus<std::vector<uint32_t>> parseDataSection(const std::string &data_section);

/// @brief extracts an entry from a dword array
///
/// @param data array of dwords containing packet data
/// @returns runlist entry with packet data populated
/// @note users must destroy the packet body with parseRunlistEntry
/// @see destroyRunlistEntry
ResultStatus<sift_pm4_runlist_entry_t> parseRunlistEntry(span<const uint32_t> data);

/// @brief frees the packet body in a runlist entry
///
/// @param entry runlist entry struct
/// @returns Status::SUCCESS on success
/// @note entry is invalid for use after call
Status destroyRunlistEntry(sift_pm4_runlist_entry_t &entry) noexcept;

/// @brief extracts all entries from a dword array
///
/// @param data array of dwords containing packet data
/// @returns Vector of populated runlist entries
/// @note users must destroy the packet bodies with parseRunlistEntry
/// @see destroyRunlistEntry
ResultStatus<std::vector<sift_pm4_runlist_entry_t>> parseRunlistEntries(span<const uint32_t> data);

/// @brief destroys entry array created when parsing entries
///
/// @param entries raw array of entries to destroy
/// @param n size of the entries array in elements
/// @returns Status::SUCCESS on success
/// @note this method frees the array of entries, not for use on a stack object
Status destroyRunlistEntries(sift_pm4_runlist_entry_t *entries, size_t n) noexcept;

/// @brief extracts MAP_PROCESS body from dwords array
///
/// @param data array of dwords starting at beginning of the body
/// @returns packet body struct
sift_pm4_body_map_process_t parseBodyMapProcess(span<const uint32_t> body_data);

/// @brief extracts MAP_QUEUES body from dwords array
///
/// @param data array of dwords starting at beginning of the body
/// @returns packet body struct
sift_pm4_body_map_queues_t parseBodyMapQueues(span<const uint32_t> body_data);
};  // namespace pm4
};  // namespace sift

#endif
