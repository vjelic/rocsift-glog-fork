/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_RLS_PARSER_H_

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>

typedef struct sift_pm4_runlist_entry_s sift_pm4_runlist_entry_t;
typedef struct sift_pm4_node_s sift_pm4_node_t;

namespace sift
{
namespace pm4
{
/// @brief drives pm4 runlist parsing with internal state machine
class RlsParser
{
 public:
  /// @brief state machine parser state
  ///
  /// Enum values:
  /// - NODE: match a node line
  /// - DATA: match a address and data line
  /// - END: finished matching a data line, a valid runlist was parsed
  /// - ERROR: internal error or code bug
  enum class State { NODE = 0, DATA = 1, ERROR = 2, END = 3 };

 private:
  /// @brief contains next parser state and an increment to the line number
  struct NextState {
    State state;
    int increment;
  };

 public:
  /// @brief constructs parser for an input text string, searching for a specific node
  ///
  /// @param rls_text null-terminated input text
  /// @param node node in runlist to search for
  /// @param gpu gpu in runlist to search for
  inline RlsParser(const char *rls_text, int node, int gpu)
      : rls_text_(rls_text),
        node_(node),
        gpu_(gpu),
        find_all_nodes_(false),
        state_(State::NODE),
        dwords_buffer_(),
        end_(0)
  {
  }

  /// @brief constructs parser for an input text string
  ///
  /// @param rls_text null-terminated input text
  /// @note will match any node in the runlist
  inline RlsParser(const char *rls_text) : RlsParser(rls_text, 0, 0) { find_all_nodes_ = true; }

  virtual ~RlsParser() = default;

  /// @brief last parsed position in input string
  ///
  /// If Parse() matched a runlist, end will point to the end of the
  /// runlist in the input string, allowing the next call to Parse()
  /// to start from this point.
  /// Otherwise, it will point to the end of the input string.
  ///
  /// @returns last parsed position in input string
  inline size_t End() const noexcept { return end_; }

  /// @brief matched runlist gpu id, if Parse() succeeded.
  /// @see Parse()
  /// @returns gpu id
  inline int Gpu() const noexcept { return gpu_; }

  /// @brief matched runlist node id, if Parse() succeeded.
  /// @see Parse()
  /// @returns node id
  inline int Node() const noexcept { return node_; }

  /// @brief extracts a runlist from the input text
  ///
  /// Runs a search for the gpu and node id (if set).
  /// Each line is matched according to the current state.
  ///
  /// If a full runlist is matched, true is returned and entries() can
  /// be accessed to get the parsed runlist.
  /// Additionally, End() is set to the end of the runlist in the input text.
  /// Call Parse() again to parse additionall runlists, if they exist.
  ///
  /// If no runlist is matched, then Parse() returns false.
  ///
  /// @returns true if a runlist is matched, false if not
  std::optional<std::vector<sift_pm4_runlist_entry_t>> Parse();

 private:
  /// @brief handles parsing in the NODE state
  /// @returns state transition
  NextState ParseNode(const std::string &line);

  /// @brief handles parsing in the DATA state
  /// @returns state transition
  NextState ParseData(const std::string &line);

  /// @brief Clears pushes the dwords_buffer_ and builds entry vector
  /// @returns optional entry vector if successful
  std::optional<std::vector<sift_pm4_runlist_entry_t>> BuildEntries();

  /// @brief internal pointer to null-terminated input text
  const char *rls_text_;

  /// @brief target or found node id
  /// @see Node()
  int node_;

  /// @brief target or found gpu id
  /// @see Gpu()
  int gpu_;

  /// @brief flag for searching for all nodes or only the set node_
  ///
  /// If gpu_ and node_ are set by the constructor, this flag will be false.
  /// Otherwise, this flags the parser to count all node and gpu ids as
  /// valid matches.
  bool find_all_nodes_;

  /// @brief internal state machine state
  State state_;

  /// @brief buffer to store dwords parsed in data state
  ///
  /// Each address line has a data section of dwords.
  /// Since number of data sections is unknown at start,
  /// store a buffer of them and combine at the end.
  ///
  /// @see BuildEntries()
  std::vector<std::vector<uint32_t>> dwords_buffer_;

  /// @brief internal pointer to last parsed position in input string
  /// @see End()
  ///
  size_t end_;
};
};  // namespace pm4
};  // namespace sift

#endif
