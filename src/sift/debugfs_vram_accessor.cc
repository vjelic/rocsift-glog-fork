/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "debugfs_vram_accessor.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "logging.h"
#include "statuserror.h"

namespace sift
{

DebugFSVRAMAccessor::DebugFSVRAMAccessor(std_fs::path vram)
    : fd_(FDOwner([&]() {
        int f = open(vram.string().c_str(), O_RDWR);
        if (f == -1) {
          Logger()->error("failed to open {}, {}", vram.string(), strerror(errno));
          throw StatusError(SIFT_STATUS_ERROR);
        }
        return f;
      }()))
{
}

ssize_t DebugFSVRAMAccessor::Read(uint64_t address, void *buffer, size_t count)
{
  if (lseek(fd_.FD(), address, SEEK_SET) < 0) {
    Logger()->error("DebugFSVramAccessor: failed to seek address {:#016x}, {}", address,
                    strerror(errno));
    return -1;
  }
  return read(fd_.FD(), buffer, count);
}

ssize_t DebugFSVRAMAccessor::Write(uint64_t address, const void *buffer, size_t count)
{
  if (lseek(fd_.FD(), address, SEEK_SET) < 0) {
    Logger()->error("DebugFSVramAccessor::Write: failed to seek address {:#016x}, {}", address,
                    strerror(errno));
    return -1;
  }
  return write(fd_.FD(), buffer, count);
  return count;
}

}  // namespace sift
