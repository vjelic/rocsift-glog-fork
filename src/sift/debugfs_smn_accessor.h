/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_DEBUGFS_SMN_ACCESSOR_H_
#define _SIFT_DEBUGFS_SMN_ACCESSOR_H_

#include "debugfs_mmio_accessor.h"

namespace sift
{

class DebugFSSMNAccessor
{
  DebugFSMMIOAccessor *mmio_;
  const uint64_t rsmu_index_hi_offset_;
  const uint64_t rsmu_index_offset_;
  const uint64_t rsmu_data_offset_;

 public:
  DebugFSSMNAccessor();
  DebugFSSMNAccessor(DebugFSMMIOAccessor &mmio, uint64_t rsmu_index_hi_offset,
                     uint64_t rsmu_index_offset, uint64_t rsmu_data_offset);
  uint32_t ReadReg32(uint64_t reg);
  void WriteReg32(uint64_t reg, uint32_t value);
};

}  // namespace sift

#endif  //  _SIFT_DEBUGFS_SMN_ACCESSOR_H_
