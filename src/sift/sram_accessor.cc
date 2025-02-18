/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "sram_accessor.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"
#include "statuserror.h"

namespace sift
{

SRAMAccessor::SRAMAccessor(DebugFSIOMEMAccessor &iomem)
    : iomem_(&iomem), devmem_(FDOwner([&]() {
        auto path = std_fs::path("/dev/mem");
        int f = open(path.string().c_str(), O_RDWR | O_DSYNC);
        if (f == -1) {
          Logger()->error("failed to create SRAMAccessor: failed to open {}, {}", path.string(),
                          strerror(errno));
          throw StatusError(SIFT_STATUS_ERROR);
        }
        return f;
      }()))
{
}

ssize_t SRAMAccessor::Read(uint64_t address, void *buffer, size_t count)
{
  bool use_iomem = (iomem_ != nullptr);
  while (true) {
    if (use_iomem) {
      auto n = iomem_->Read(address, buffer, count);
      if (n == -1) {
        use_iomem = false;
        continue;
      } else {
        return n;
      }
    }
    if (lseek(devmem_.FD(), address, SEEK_SET) < 0) {
      Logger()->error("SramAccessor: failed to seek address {:#016x}, {}", address,
                      strerror(errno));
      return -1;
    }
    return read(devmem_.FD(), buffer, count);
  }
}

ssize_t SRAMAccessor::Write(uint64_t address, const void *buffer, size_t count)
{
  bool use_iomem = (iomem_ != nullptr);
  while (true) {
    if (use_iomem) {
      auto n = iomem_->Write(address, buffer, count);
      if (n == -1) {
        use_iomem = false;
        continue;
      } else {
        return n;
      }
    }
    if (lseek(devmem_.FD(), address, SEEK_SET) < 0) {
      Logger()->error("SramAccessor: failed to seek address {:#016x}, {}", address,
                      strerror(errno));
      return -1;
    }
    return write(devmem_.FD(), buffer, count);
  }
}

}  // namespace sift
