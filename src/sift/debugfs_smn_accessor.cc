/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "debugfs_smn_accessor.h"

namespace sift
{

DebugFSSMNAccessor::DebugFSSMNAccessor()
    : mmio_(nullptr),
      rsmu_index_hi_offset_(0xbadc0de),
      rsmu_index_offset_(0xbadc0de),
      rsmu_data_offset_(0xbadc0de)
{
}

DebugFSSMNAccessor::DebugFSSMNAccessor(DebugFSMMIOAccessor &mmio, uint64_t rsmu_index_hi_offset,
                                       uint64_t rsmu_index_offset, uint64_t rsmu_data_offset)
    : mmio_(&mmio),
      rsmu_index_hi_offset_(rsmu_index_hi_offset),
      rsmu_index_offset_(rsmu_index_offset),
      rsmu_data_offset_(rsmu_data_offset)
{
}

uint32_t DebugFSSMNAccessor::ReadReg32(uint64_t reg)
{
  uint32_t reg_lo = reg & 0xFFFFFFFFUL;
  uint32_t reg_hi = (reg >> 32) & 0xFF;
  mmio_->WriteReg32(rsmu_index_hi_offset_, reg_hi);
  mmio_->WriteReg32(rsmu_index_offset_, reg_lo);
  return mmio_->ReadReg32(rsmu_data_offset_);
}

void DebugFSSMNAccessor::WriteReg32(uint64_t reg, uint32_t value)
{
  uint32_t reg_lo = reg & 0xFFFFFFFFUL;
  uint32_t reg_hi = (reg >> 32) & 0xFF;
  mmio_->WriteReg32(rsmu_index_hi_offset_, reg_hi);
  mmio_->WriteReg32(rsmu_index_offset_, reg_lo);
  mmio_->WriteReg32(rsmu_data_offset_, value);
}

}  // namespace sift
