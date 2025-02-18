/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <fstream>
#include <regex>

#include "partition.h"
#include "device.h"
#include "engine.h"
#include "logging.h"
#include "statuserror.h"

namespace sift
{

Partition::Partition(Engine *engine, sift::Device *dev, KFDNode *kfd, int local_id, int global_id,
                     DRMNode *drmnode, const std::vector<int> &xcc_die_ids)
    : engine_(engine),
      dev_(dev),
      kfdnode_(kfd),
      drmnode_(drmnode),
      local_id_(local_id),
      global_id_(global_id),
      xcc_die_ids_(xcc_die_ids),
      amdgpu_debugfs_(ResolveDebugFS()),
      sram_([this]() {
        try {
          if (!amdgpu_debugfs_) {
            return std::unique_ptr<SRAMAccessor>();
          }
          return std::make_unique<SRAMAccessor>(amdgpu_debugfs_->IOMEM());
        } catch (std::exception &e) {
          Logger()->error(e.what());
          return std::unique_ptr<SRAMAccessor>();
        }
      }())
{
}

std::unique_ptr<AMDGPUDebugFS> Partition::ResolveDebugFS()
{
  try {
    auto root = std_fs::path("/sys/kernel/debug/dri");
    if ((!std_fs::exists(root)) || (!std_fs::is_directory(root))) {
      Logger()->error("CODE BUG, invalid debugfs root: {}", root.string());
      throw StatusError(SIFT_STATUS_CODE_BUG);
    }
    for (auto const &dir_entry : std_fs::directory_iterator{root}) {
      if ((!std_fs::exists(dir_entry)) || (!std_fs::is_directory(dir_entry))) {
        continue;
      }

      bool amdgpu_debugfs_interface_found = false;
      for (auto const &f : std_fs::directory_iterator{dir_entry}) {
        if (f.path().filename().string() == "amdgpu_regs2") {
          amdgpu_debugfs_interface_found = true;
          break;
        }
      }
      if (!amdgpu_debugfs_interface_found) {
        continue;
      }
      for (auto const &f : std_fs::directory_iterator{dir_entry}) {
        if (f.path().filename() == "name") {
          std::ifstream p(f.path());
          std::stringstream buffer;
          buffer << p.rdbuf();
          auto contents = buffer.str();
          std::regex r(
              "amdgpu\\s+dev=(\\w{4}):(\\w{2}):(\\w{2})\\.(\\w{1})\\s+unique=(\\w{4}):(\\w{2}):("
              "\\w{2})\\.(\\w{1})");
          std::smatch m;
          if (!std::regex_search(contents, m, r)) {
            Logger()->error("CODE BUG, regex search should match");
            throw StatusError(SIFT_STATUS_CODE_BUG);
          }
          uint32_t domain = std::stoul(m[1], 0, 16);
          uint32_t bus = std::stoul(m[2], 0, 16);
          uint32_t device = std::stoul(m[3], 0, 16);
          uint32_t func = std::stoul(m[4], 0, 16);

          uint32_t kfd_bus = (kfdnode_->Properties().location_id >> 8) & 0xff;
          uint32_t kfd_device = (kfdnode_->Properties().location_id >> 3) & 0x1f;

          if ((kfdnode_->Properties().domain_id == domain) && (kfd_bus == bus) &&
              (kfd_device == device)) {
            Logger()->debug("{:04x}.{:02X}.{:02X}.{:01X} -> {}", domain, bus, device, func,
                            dir_entry.path().string());
            return std::make_unique<AMDGPUDebugFS>(dir_entry.path());
          }
        }
      }
    }
  } catch (std::exception &e) {
    Logger()->error(e.what());
    return std::unique_ptr<AMDGPUDebugFS>();
  }
  // should never get here
  Logger()->error("CODE BUG, executed code path that should not be reached");
  throw StatusError(SIFT_STATUS_CODE_BUG);
}

Device &Partition::GetDevice() { return *dev_; }

DRMNode &Partition::DRM() { return *drmnode_; }

KFDNode &Partition::KFD() { return *kfdnode_; }

const std::vector<int> &Partition::XCCDieIDs() const noexcept { return xcc_die_ids_; }

AMDGPUDebugFS &Partition::DebugFS()
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return *amdgpu_debugfs_.get();
}

ssize_t Partition::ReadVRAM(uint64_t address, void *buffer, size_t count)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  // first, determine if in xgmi hive
  auto &xgmi = drmnode_->XGMI();
  if (!xgmi.hive_id) {
    return amdgpu_debugfs_->VRAM().Read(address, buffer, count);
  }
  // if in xgmi hive, maniuplate address.
  /* Our GPUs use a hive-wide address.
   * However, the amdgpu debugfs amdgpu_vram file
   * only operates on local VRAM for the dri node
   * in the path /sys/kernel/debug/dri/<node>/amdgpu_vram.
   *
   * Thus, we need to convert the global address, to a
   * device + local address.
   */
  for (size_t n = 0; n < xgmi.nodes.size(); n++) {
    auto vram_size = xgmi.nodes[n]->TotalVRAMBytes();
    if (address < vram_size) {
      if (xgmi.nodes[n]->CardName() == drmnode_->CardName()) {
        return amdgpu_debugfs_->VRAM().Read(address, buffer, count);
      } else {
        for (auto &p : engine_->Partitions()) {
          if (p->DRM().XGMI().device_id == xgmi.nodes[n]->XGMI().device_id) {
            return p->amdgpu_debugfs_->VRAM().Read(address, buffer, count);
          }
        }
        Logger()->error("Failed to find partition with expected XGMI device ID");
        throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
      }
    } else {
      // round up to next GiB
      auto gb = 1ull << 30;
      address -= gb * ((vram_size + gb - 1) / gb);
    }
  }
  Logger()->error("Requested address is out of range");
  throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
}

ssize_t Partition::WriteVRAM(uint64_t address, const void *buffer, size_t count)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }

  // first, determine if in xgmi hive
  auto &xgmi = drmnode_->XGMI();
  if (!xgmi.hive_id) {
    return amdgpu_debugfs_->VRAM().Write(address, buffer, count);
  }
  // if in xgmi hive, maniuplate address.
  for (auto &n : xgmi.nodes) {
    auto vram_size = n->TotalVRAMBytes();
    if (address < vram_size) {
      for (auto &p : engine_->Partitions()) {
        if (p->DRM().XGMI().device_id == n->XGMI().device_id) {
          return p->amdgpu_debugfs_->VRAM().Write(address, buffer, count);
        }
      }
      Logger()->error("Failed to find partition with expected XGMI device ID");
      throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
    } else {
      // round up to next GiB
      auto gb = 1ull << 30;
      address -= gb * ((vram_size + gb - 1) / gb);
    }
  }
  Logger()->error("Requested address is out of range");
  throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
}

ssize_t Partition::ReadSystemRAM(uint64_t address, void *buffer, size_t count)
{
  if (!sram_) {
    Logger()->warn("Failed to get system ram interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return sram_->Read(address, buffer, count);
}

ssize_t Partition::WriteSystemRAM(uint64_t address, const void *buffer, size_t count)
{
  if (!sram_) {
    Logger()->warn("Failed to get system ram interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return sram_->Write(address, buffer, count);
}

uint32_t Partition::SMNReadReg32(uint64_t reg)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return amdgpu_debugfs_->SMN().ReadReg32(reg);
}

void Partition::SMNWriteReg32(uint64_t reg, uint32_t value)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return amdgpu_debugfs_->SMN().WriteReg32(reg, value);
}

uint32_t Partition::MMIOReadReg32(uint64_t reg)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return amdgpu_debugfs_->MMIO().ReadReg32(reg);
}

void Partition::MMIOWriteReg32(uint64_t reg, uint32_t value)
{
  if (!amdgpu_debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return amdgpu_debugfs_->MMIO().WriteReg32(reg, value);
}

int Partition::GlobalID() const noexcept { return global_id_; }

}  // namespace sift
