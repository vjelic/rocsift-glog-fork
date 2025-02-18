/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_PCI_MMIO_ACCESSSOR_H_
#define _SIFT_PCI_MMIO_ACCESSSOR_H_

#include <cstdint>
#include <mutex>
#include <pciaccess.h>

namespace sift
{

class PCIMMIOAccessor
{
 private:
  std::mutex mux_;
  struct pci_device *pci_;
  int region_;
  void *mmio_bar_;

 public:
  PCIMMIOAccessor(struct pci_device *pci, int region);
  ~PCIMMIOAccessor();
  uint32_t ReadMMIOReg32(uint64_t reg);
  void WriteMMIOReg32(uint64_t reg, uint32_t value);
  std::mutex &Mux();
};

}  // namespace sift

#endif  // _SIFT_PCI_MMIO_ACCESSSOR_H_
