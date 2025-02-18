/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <optional>
#include <sstream>
#include <iomanip>
#include <string_view>
#include <string>
#include <vector>

#include "rocsift/pm4.h"

#include "../sift/logging.h"
#include "../sift/statuserror.h"
#include "pm4.h"
#include "rls_parser.h"

using namespace sift::pm4;

RlsParser::NextState RlsParser::ParseNode(const std::string &line)
{
  auto result = parseNodeLine(line);
  switch (result.status) {
    case Status::ERROR: {
      return {State::ERROR, 0};
    }
    case Status::NO_MATCH: {
      return {State::NODE, 1};
    }
    case Status::SUCCESS: {
      // Found the node, start parsing data
      auto &found_node = result.value.value();
      if (find_all_nodes_ || (static_cast<int>(found_node.node_id) == node_ &&
                              static_cast<int>(found_node.gpu_id) == gpu_)) {
        node_ = found_node.node_id;
        gpu_ = found_node.gpu_id;
        return {State::DATA, 1};
      } else {
        return {State::NODE, 1};
      }
    }
    default: {
      sift::Logger()->error("CODE_BUG: Failed to parse RLS file contents: {}", line);
      throw StatusError(SIFT_STATUS_CODE_BUG);
    }
  };
}

std::optional<std::vector<sift_pm4_runlist_entry_t>> RlsParser::BuildEntries()
{
  if (dwords_buffer_.empty()) {
    return std::nullopt;
  }
  size_t total_dwords = 0;
  for (auto &v : dwords_buffer_) {
    total_dwords += v.size();
  }
  std::vector<uint32_t> dwords(total_dwords);
  int i = 0;
  for (const auto &dword_buffer : dwords_buffer_) {
    for (const auto &dword : dword_buffer) {
      dwords[i] = dword;
      i++;
    }
  }
  dwords_buffer_.clear();
  auto result = parseRunlistEntries({dwords.data(), dwords.size()});
  if (result.status != Status::SUCCESS) {
    sift::Logger()->error("Failed to get entries for Node {}, GPU {:#x}", node_, gpu_);
    std::stringstream ss;
    for (auto &dword : dwords) {
      ss << "0x" << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << dword;
      ss << " ";
    }
    sift::Logger()->error("Runlist data: {}", ss.str());
    return std::nullopt;
  }
  return result.value;
}
RlsParser::NextState RlsParser::ParseData(const std::string &line)
{
  auto result = getDataStartPosition(line);
  switch (result.status) {
    case Status::ERROR: {
      sift::Logger()->error("Failed to parse RLS file contents: {}", line);
      return {State::ERROR, 0};
    }
    case Status::NO_MATCH: {
      if (dwords_buffer_.size() == 0) {
        // no valid address line was found for a node
        return {State::END, 0};
      } else {
        // Reached end of current data section
        return {State::END, 0};
      }
    }
    case Status::SUCCESS: {
      size_t data_start_pos = result.value.value();
      auto data_section = line.substr(data_start_pos);
      auto data = parseDataSection(data_section);
      if (Status::SUCCESS != data.status) {
        sift::Logger()->error("Failed to parse RLS file contents: {}", line);
        return {State::ERROR, 0};
      }
      dwords_buffer_.emplace_back(data.value.value());
      return {State::DATA, 1};
    }
    default: {
      sift::Logger()->error("CODE_BUG: Failed to parse RLS file contents: {}", line);
      throw StatusError(SIFT_STATUS_CODE_BUG);
    }
  };
}

std::optional<std::vector<sift_pm4_runlist_entry_t>> RlsParser::Parse()
{
  std::string_view rls_view(rls_text_);
  size_t start = end_;
  size_t last_data_match = start;
  end_ = rls_view.find("\n", start);
  bool parsed_node = false;
  state_ = State::NODE;

  while (end_ != rls_view.npos && !parsed_node) {
    // Make a copy of the substring. Need a null terminated substring.
    const std::string line{rls_view.substr(start, end_ - start)};
    NextState next;
    // handle current state
    switch (state_) {
      case State::NODE:
        next = ParseNode(line);
        state_ = next.state;
        start = end_ + next.increment;
        end_ = rls_view.find('\n', start);
        break;
      case State::DATA:
        next = ParseData(line);
        if (next.state != State::DATA) {
          last_data_match = start - 1;
        }
        state_ = next.state;
        start = end_ + next.increment;
        end_ = rls_view.find('\n', start);
        break;
      case State::END:
        parsed_node = true;
        // rewind end on successful match to right after last match position
        end_ = last_data_match;
        break;
      case State::ERROR:
        sift::Logger()
            ->error("CODE_BUG: Failed to parse RLS file contents, runlist format is invalid: {}",
                    rls_text_);
        throw StatusError(SIFT_STATUS_CODE_BUG);
    };
  }
  // Parsing done, use the data to compose the entries
  return BuildEntries();
}
