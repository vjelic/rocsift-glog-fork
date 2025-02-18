/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include "debugfs_mmio_accessor.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <asm/ioctl.h>
#include <sys/ioctl.h>

#include "logging.h"
#include "statuserror.h"

namespace sift
{

DebugFSMMIOAccessor::DebugFSMMIOAccessor(std_fs::path regs2)
    : fd_(FDOwner([&]() {
        int f = open(regs2.string().c_str(), O_RDWR);
        if (f == -1) {
          Logger()->error("failed to create DebugFSMMIOAccessor: failed to open {}, {}",
                          regs2.string(), strerror(errno));
          throw StatusError(SIFT_STATUS_ERROR);
        }
        return f;
      }()))
{
}

void DebugFSMMIOAccessor::ApplyGRBMBank(uint32_t se, uint32_t sh, uint32_t instance, bool pg_lock,
                                        int xcc_id)
{
  AMDGPUDebugFSRegs2IocDataV2 data;
  memset(&data, 0x0, sizeof(AMDGPUDebugFSRegs2IocDataV2));
  data.use_grbm = 1;
  data.grbm.se = se;
  data.grbm.sh = sh;
  data.grbm.instance = instance;
  data.pg_lock = pg_lock ? 1 : 0;
  data.xcc_id = xcc_id;
  int r = ioctl(fd_.FD(),
                _IOWR(0x20,
                      static_cast<std::underlying_type_t<AMDGPUDebugFSRegs2CMDS> >(
                          AMDGPUDebugFSRegs2CMDS::SET_STATE_V2),
                      struct AMDGPUDebugFSRegs2IocDataV2),
                &data);
  if (r) {
    Logger()->error("failed to set GRBM bank: ", strerror(errno));
    throw StatusError(SIFT_STATUS_ERROR);
  }
}

uint32_t DebugFSMMIOAccessor::ReadReg32(uint64_t reg)
{
  if (lseek(fd_.FD(), reg, SEEK_SET) < 0) {
    Logger()->error("Cannot seek to MMIO address {}, {}:{}", std::to_string(reg),
                    std::to_string(errno), strerror(errno));
    throw StatusError(SIFT_STATUS_ERROR);
  }
  uint32_t value = 0xdeadbeef;
  if (read(fd_.FD(), &value, 4) != 4) {
    Logger()->error("Failed to read reg {} {}:{}", std::to_string(reg), std::to_string(errno),
                    strerror(errno));
    throw StatusError(SIFT_STATUS_ERROR);
  }

  return value;
}

void DebugFSMMIOAccessor::WriteReg32(uint64_t reg, uint32_t value)
{
  if (lseek(fd_.FD(), reg, SEEK_SET) < 0) {
    Logger()->error("Cannot seek to MMIO address {}, {}:{}", std::to_string(reg),
                    std::to_string(errno), strerror(errno));
    throw StatusError(SIFT_STATUS_ERROR);
  }
  if (write(fd_.FD(), &value, 4) != 4) {
    Logger()->error("Failed to write reg {} {}:{}", std::to_string(reg), std::to_string(errno),
                    strerror(errno));
    throw StatusError(SIFT_STATUS_ERROR);
  }
}

}  // namespace sift
