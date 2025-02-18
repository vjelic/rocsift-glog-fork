/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_DEBUGFS_MMIO_ACCESSOR_H_
#define _SIFT_DEBUGFS_MMIO_ACCESSOR_H_

#include <cstdint>

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include "fdowner.h"

namespace sift
{

struct AMDGPUDebugFSRegs2IocData {
  uint32_t use_srbm;
  uint32_t use_grbm;
  uint32_t pg_lock;
  struct GRBM {
    uint32_t se;
    uint32_t sh;
    uint32_t instance;
  } grbm;
  struct SRBM {
    uint32_t me;
    uint32_t pipe;
    uint32_t queue;
    uint32_t vmid;
  } srbm;
};

struct AMDGPUDebugFSRegs2IocDataV2 {
  uint32_t use_srbm;
  uint32_t use_grbm;
  uint32_t pg_lock;
  struct GRBM {
    uint32_t se;
    uint32_t sh;
    uint32_t instance;
  } grbm;
  struct SRBM {
    uint32_t me;
    uint32_t pipe;
    uint32_t queue;
    uint32_t vmid;
  } srbm;
  uint32_t xcc_id;
};

enum class AMDGPUDebugFSRegs2CMDS : int {
  SET_STATE = 0,
  SET_STATE_V2,
};

class DebugFSMMIOAccessor
{
 private:
  FDOwner fd_;

 public:
  DebugFSMMIOAccessor(std_fs::path regs2);

  void ApplyGRBMBank(uint32_t se, uint32_t sh, uint32_t instance, bool pg_lock = false,
                     int xcc_id = -1);
  // void ApplySRBMBank(uint32_t me, uint32_t pipe, uint32_t queue, uint32_t vmid, bool
  // pg_lock=false, int xcc_id=-1);

  uint32_t ReadReg32(uint64_t reg);
  void WriteReg32(uint64_t reg, uint32_t value);
};

}  // namespace sift

#endif  // _SIFT_DEBUGFS_MMIO_ACCESSOR_H_
