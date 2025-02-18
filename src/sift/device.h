/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_DEVICE_H_
#define _SIFT_DEVICE_H_

#include <cstdint>
#include <memory>
#include <vector>
#include <utility>

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include "partition.h"
#include "amdgpu_debugfs.h"
#include "sram_accessor.h"
#include "ip_discovery.h"
#include "../kfd/kfdnode.h"

namespace sift
{

class Engine;  // forward declaration
class Device
{
 private:
  Engine *engine_;
  uint32_t instance_;
  int domain_;
  uint16_t bdf_;
  std::unique_ptr<ipdiscovery::Root> ipdiscovery_table_;
  std::vector<std::unique_ptr<Partition>> partitions_;

  // privileged features
  std::unique_ptr<AMDGPUDebugFS> debugfs_;
  std::unique_ptr<SRAMAccessor> sram_;

  std::vector<std::unique_ptr<Partition>> PartitionsFromNodes(
      const std::vector<std::pair<KFDNode *, int>> &nodes);

 public:
  Device(Engine *engine, uint32_t instance, int domain, uint16_t bdf,
         const std::vector<std::pair<KFDNode *, int>> &nodes);
  ~Device();
  Device(Device const &other) = delete;
  Device(Device &&other) noexcept = default;

  uint32_t Instance();

  std::vector<std::unique_ptr<Partition>> &Partitions();

  AMDGPUDebugFS &DebugFS();
  ssize_t ReadVRAM(uint64_t address, void *buffer, size_t count);
  ssize_t WriteVRAM(uint64_t address, const void *buffer, size_t count);
  ssize_t ReadSystemRAM(uint64_t address, void *buffer, size_t count);
  ssize_t WriteSystemRAM(uint64_t address, const void *buffer, size_t count);
  uint32_t SMNReadReg32(uint64_t reg);
  void SMNWriteReg32(uint64_t reg, uint32_t value);
  uint32_t MMIOReadReg32(uint64_t reg);
  void MMIOWriteReg32(uint64_t reg, uint32_t value);
};

class DeviceBuilder
{
 public:
  int instance;
  int domain;
  uint64_t bdf;
  std::vector<std::pair<KFDNode *, int>> nodes;
  DeviceBuilder(int instance, int domain, uint16_t bdf);
  void AddPartition(KFDNode &node, int global_partition_id);
};

}  // namespace sift

#endif  // _SIFT_DEVICE_H
