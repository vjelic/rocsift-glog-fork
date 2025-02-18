/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <string>
#include <mutex>
#include <cstdint>

#include "sys/ioctl.h"
#include "asm/ioctl.h"
#include "sys/fcntl.h"

#include "device.h"
#include "engine.h"

#include "logging.h"
#include "statuserror.h"

namespace sift
{

std::vector<std::unique_ptr<Partition>> Device::PartitionsFromNodes(
    const std::vector<std::pair<KFDNode *, int>> &nodes)
{
  std::vector<std::unique_ptr<Partition>> partitions;
  if (!nodes.size()) {
    return partitions;
  }

  std::vector<DRMNode *> drm_nodes;

  std::vector<std::vector<int>> xcc_die_ids(nodes.size());

  size_t current_node = 0;
  if (ipdiscovery_table_) {
    for (auto &die : ipdiscovery_table_->Dies()) {
      auto &gc = die.LookupIP("GC");
      for (auto &inst : gc.Instances()) {
        if (inst.Harvest()) {
          continue;
        }
        for (auto &id = current_node; id < nodes.size(); id++) {
          if (xcc_die_ids.at(id).size() < nodes.at(id).first->Properties().num_xcc) {
            xcc_die_ids.at(id).push_back(inst.NumInstance());
            current_node = id;
            break;
          }
        }
      }
    }
  } else {
    Logger()->debug("Failed to find IP Discovery table, taking best guess at GC IDs");
    for (auto &id = current_node; id < nodes.size(); id++) {
      auto num_xcc = nodes.at(id).first->Properties().num_xcc;
      if (num_xcc > 1) {
        Logger()->error("Failed to find IPDiscoveryTable for KFDNode with multiple XCCs(" +
                        std::to_string(num_xcc) + "), bailing out");
        return std::vector<std::unique_ptr<Partition>>();
      }
      xcc_die_ids.at(id).push_back(0);
    }
  }

  for (auto i = 0ul; i < nodes.size(); i++) {
    std::string drm_name = "renderD" +
                           std::to_string(nodes.at(i).first->Properties().drm_render_minor);
    auto drm_node = DRM::GetInstance().NodeByName(drm_name);
    if (!drm_node) {
      Logger()->error("Failed to find DRM node with name {}!", drm_name);
      throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
    }
    partitions.push_back(std::make_unique<Partition>(engine_, this, nodes.at(i).first, i,
                                                     nodes.at(i).second, drm_node,
                                                     xcc_die_ids.at(i)));
  }
  return partitions;
}

Device::Device(Engine *engine, uint32_t instance, int domain, uint16_t bdf,
               const std::vector<std::pair<KFDNode *, int>> &nodes)
    : engine_(engine),
      instance_(instance),
      domain_(domain),
      bdf_(bdf),
      ipdiscovery_table_([&]() {
        auto func = bdf_ & 0x7;
        auto device = (bdf_ >> 3) & 0x1f;
        auto bus = (bdf_ >> 8) & 0xff;
        std::stringstream ss;
        ss << "/sys/bus/pci/devices/";
        ss << std::hex << std::setw(4) << std::setfill('0') << domain_ << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << bus << ":";
        ss << std::hex << std::setw(2) << std::setfill('0') << device << ".";
        ss << std::hex << std::setw(1) << std::setfill('0') << func;

        auto pci_path = std_fs::path(ss.str());
        auto ip_disc = pci_path / "ip_discovery";
        if (std_fs::exists(ip_disc) && std_fs::is_directory(ip_disc)) {
          return std::make_unique<ipdiscovery::Root>(ip_disc);
        }
        return std::unique_ptr<ipdiscovery::Root>();
      }()),
      partitions_(PartitionsFromNodes(nodes)),
      debugfs_([this]() {
        try {
          return std::make_unique<AMDGPUDebugFS>(partitions_.at(0)->DebugFS().Root());
        } catch (std::exception &e) {
          Logger()->error(e.what());
          return std::unique_ptr<AMDGPUDebugFS>();
        }
      }()),
      sram_([this]() {
        try {
          if (!debugfs_) {
            return std::unique_ptr<SRAMAccessor>();
          }
          return std::make_unique<SRAMAccessor>(debugfs_->IOMEM());
        } catch (std::exception &e) {
          Logger()->error(e.what());
          return std::unique_ptr<SRAMAccessor>();
        }
      }())
{
  // now, we can use the pci bdf to try to lookup
  // the same device in:
  //    PCI land: /sys/bus/pci/devices/0000:31:00.0 (some special files here)
  //    KFD land: /sys/devices/virtual/kfd/kfd/topology/nodes/1 (KFD properties!)
  //    debugfs land: /sys/kernel/debug/dri/1/name
  //    					name file contains: /sys/amdgpu dev=0000:31:00.0
  //    unique=0000:31:00.0
}

Device::~Device() { Logger()->trace("Device {} is going out of scope", instance_); }

uint32_t Device::Instance() { return instance_; }

AMDGPUDebugFS &Device::DebugFS()
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }

  return *debugfs_.get();
}

std::vector<std::unique_ptr<Partition>> &Device::Partitions() { return partitions_; }

ssize_t Device::ReadVRAM(uint64_t address, void *buffer, size_t count)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->VRAM().Read(address, buffer, count);
}

ssize_t Device::WriteVRAM(uint64_t address, const void *buffer, size_t count)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->VRAM().Write(address, buffer, count);
}

ssize_t Device::ReadSystemRAM(uint64_t address, void *buffer, size_t count)
{
  if (!sram_) {
    Logger()->warn("Failed to get system ram interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return sram_->Read(address, buffer, count);
}

ssize_t Device::WriteSystemRAM(uint64_t address, const void *buffer, size_t count)
{
  if (!sram_) {
    Logger()->warn("Failed to get system ram interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return sram_->Write(address, buffer, count);
}

uint32_t Device::SMNReadReg32(uint64_t reg)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->SMN().ReadReg32(reg);
}

void Device::SMNWriteReg32(uint64_t reg, uint32_t value)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->SMN().WriteReg32(reg, value);
}

uint32_t Device::MMIOReadReg32(uint64_t reg)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->MMIO().ReadReg32(reg);
}

void Device::MMIOWriteReg32(uint64_t reg, uint32_t value)
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return debugfs_->MMIO().WriteReg32(reg, value);
}

DeviceBuilder::DeviceBuilder(int instance, int domain, uint16_t bdf)
    : instance(instance), domain(domain), bdf(bdf), nodes(std::vector<std::pair<KFDNode *, int>>())
{
}

void DeviceBuilder::AddPartition(KFDNode &node, int global_partition_id)
{
  nodes.push_back(std::make_pair(&node, global_partition_id));
}

}  // namespace sift
