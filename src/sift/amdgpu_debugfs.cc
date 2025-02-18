/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "amdgpu_debugfs.h"
#include "logging.h"

namespace sift
{

const uint32_t RSMU_INDEX_OFFSET = 0x50;
const uint32_t RSMU_DATA_OFFSET = 0x54;
const uint32_t RSMU_INDEX_HI_OFFSET = 0x58;

AMDGPUDebugFS::AMDGPUDebugFS(std_fs::path root)
    : root_(root),
      mmio_(DebugFSMMIOAccessor(root_ / "amdgpu_regs2")),
      smn_(DebugFSSMNAccessor(mmio_, RSMU_INDEX_HI_OFFSET, RSMU_INDEX_OFFSET, RSMU_DATA_OFFSET)),
      vram_(DebugFSVRAMAccessor(root_ / "amdgpu_vram")),
      iomem_(DebugFSIOMEMAccessor(root_ / "amdgpu_iomem"))
{
  Logger()->debug("Created AMDGPUDebugFS with root {}", root_.string());
}

// DebugFSMMIOAccessor &AMDGPUDebugFS::MMIO()
//{
//	return mmio_;
// };

DebugFSMMIOAccessor &AMDGPUDebugFS::MMIO() { return mmio_; };

DebugFSSMNAccessor &AMDGPUDebugFS::SMN() { return smn_; };

DebugFSVRAMAccessor &AMDGPUDebugFS::VRAM() { return vram_; }

DebugFSIOMEMAccessor &AMDGPUDebugFS::IOMEM() { return iomem_; }

std_fs::path AMDGPUDebugFS::Root() const noexcept { return root_; }

}  // namespace sift
