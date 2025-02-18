/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_AMDGPU_DEBUGFS_H_
#define _SIFT_AMDGPU_DEBUGFS_H_

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include "debugfs_vram_accessor.h"
#include "debugfs_mmio_accessor.h"
#include "debugfs_smn_accessor.h"
#include "debugfs_iomem_accessor.h"

namespace sift
{

class AMDGPUDebugFS
{
 private:
  std_fs::path root_;
  DebugFSMMIOAccessor mmio_;
  DebugFSSMNAccessor smn_;
  DebugFSVRAMAccessor vram_;
  DebugFSIOMEMAccessor iomem_;

 public:
  AMDGPUDebugFS(std_fs::path root);
  AMDGPUDebugFS(const AMDGPUDebugFS &other) = delete;
  AMDGPUDebugFS(AMDGPUDebugFS &&other) = delete;
  AMDGPUDebugFS &operator=(AMDGPUDebugFS &&) = delete;
  DebugFSMMIOAccessor &MMIO();
  DebugFSSMNAccessor &SMN();
  DebugFSVRAMAccessor &VRAM();
  DebugFSIOMEMAccessor &IOMEM();
  std_fs::path Root() const noexcept;
};

}  // namespace sift
#endif  // _SIFT_AMDGPU_DEBUGFS_H_
