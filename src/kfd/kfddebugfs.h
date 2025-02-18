/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_KFDDEBUGFS_H_
#define _SIFT_KFDDEBUGFS_H_

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

namespace sift
{

class KFDDebugFS
{
 private:
  std_fs::path root_;

 public:
  KFDDebugFS(std_fs::path root = "/sys/kernel/debug/kfd");
  const std::string GetRunlists();
  const std::string GetMQDs();
  const std::string GetHQDs();
  // TODO: get system mem and ttm mem usage and limits
};

}  // namespace sift
#endif  // _SIFT_KFDDEBUGFS_H_
