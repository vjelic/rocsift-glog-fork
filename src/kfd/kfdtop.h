/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_KFD_H_
#define _SIFT_KFD_H_

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include <memory>
#include <vector>

#include "kfdnode.h"
#include "kfddebugfs.h"

namespace sift
{

class KFDHandle;  // Forward Delcaration
class KFDProc
{
 private:
  std_fs::path root_;
  int pid_;
  int pasid_;

 public:
  KFDProc(KFDHandle *kfd, int pid);
  int PID();
  int PASID();
};

class KFDHandle
{
 private:
  std_fs::path root_;
  std::vector<KFDNode> nodes_;
  std::unique_ptr<KFDDebugFS> debugfs_;

 public:
  KFDHandle(std_fs::path root = "/sys/class/kfd");
  std::vector<KFDNode> &Nodes();
  std::vector<KFDProc> Processes();
  std_fs::path Root() const noexcept;
  KFDDebugFS &DebugFS();
};

}  // namespace sift

#endif  // _SIFT_KFD_H_
