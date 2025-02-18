/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_PARTITION_H_
#define _SIFT_PARTITION_H_

#include <memory>
#include <vector>

#include "../kfd/kfdnode.h"
#include "../drm/drm.h"
#include "amdgpu_debugfs.h"
#include "sram_accessor.h"

namespace sift
{

class Engine;  // forward delcartion
class Device;  // forward declaration
class Partition
{
 private:
  Engine *engine_;
  Device *dev_;
  KFDNode *kfdnode_;
  DRMNode *drmnode_;
  int local_id_;
  int global_id_;
  std::vector<int> xcc_die_ids_;

  // privileged features
  std::unique_ptr<AMDGPUDebugFS> amdgpu_debugfs_;
  std::unique_ptr<SRAMAccessor> sram_;

  std::unique_ptr<AMDGPUDebugFS> ResolveDebugFS();
  std::unique_ptr<SRAMAccessor> ResolveSramAccessor();

 public:
  Partition(Engine *engine, Device *dev, KFDNode *node, int local_id, int global_id,
            DRMNode *drmnode, const std::vector<int> &xcc_die_ids);
  Partition(const Partition &other) = delete;
  Partition(Partition &&other) = delete;
  Partition &operator=(Partition &&) = delete;
  KFDNode &KFD();
  Device &GetDevice();
  AMDGPUDebugFS &DebugFS();
  DRMNode &DRM();
  const std::vector<int> &XCCDieIDs() const noexcept;

  ssize_t ReadVRAM(uint64_t address, void *buffer, size_t count);
  ssize_t WriteVRAM(uint64_t address, const void *buffer, size_t count);
  ssize_t ReadSystemRAM(uint64_t address, void *buffer, size_t count);
  ssize_t WriteSystemRAM(uint64_t address, const void *buffer, size_t count);
  uint32_t SMNReadReg32(uint64_t reg);
  void SMNWriteReg32(uint64_t reg, uint32_t value);
  uint32_t MMIOReadReg32(uint64_t reg);
  void MMIOWriteReg32(uint64_t reg, uint32_t value);

  int GlobalID() const noexcept;
};

}  // namespace sift
#endif  // _SIFT_PARTITION_H_
