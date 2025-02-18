/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <cstdlib>

#include "pci_mmio_accessor.h"
#include "logging.h"
#include "statuserror.h"

namespace sift
{

PCIMMIOAccessor::PCIMMIOAccessor(struct pci_device *pci, int region)
    : mux_(std::mutex()), pci_(pci), region_(region), mmio_bar_(nullptr)
{
  auto bar = pci_->regions[region_];
  auto r = pci_device_map_range(pci_, bar.base_addr, bar.size, PCI_DEV_MAP_FLAG_WRITABLE,
                                &mmio_bar_);
  if (r) {
    Logger()->error("failed to map MMIO bar: {}", r);
    throw StatusError(SIFT_STATUS_ERROR);
  }
}

std::mutex &PCIMMIOAccessor::Mux() { return mux_; }

PCIMMIOAccessor::~PCIMMIOAccessor()
{
  if (mmio_bar_) {
    auto bar = pci_->regions[region_];
    auto r = pci_device_unmap_range(pci_, bar.memory, bar.size);
    if (r) {
      Logger()->error("Failed to unmap range");
      abort();
    }
  }
}

uint32_t PCIMMIOAccessor::ReadMMIOReg32(uint64_t reg)
{
  return *reinterpret_cast<uint32_t *>(&static_cast<uint8_t *>(mmio_bar_)[reg]);
}

void PCIMMIOAccessor::WriteMMIOReg32(uint64_t reg, uint32_t value)
{
  uint32_t *u32 = reinterpret_cast<uint32_t *>(&static_cast<uint8_t *>(mmio_bar_)[reg]);
  *u32 = value;
}

}  // namespace sift
