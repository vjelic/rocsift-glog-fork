/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_KFD_NODE_H_
#define _SIFT_KFD_NODE_H_

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

#include "rocsift/kfd.h"

namespace sift
{

class KFDNode
{
 private:
  int node_;
  std_fs::path kfd_dir_;
  sift_kfd_node_properties_t kfd_properties_;
  int gpu_id_;

  sift_kfd_node_properties_t ParseKFDProperties(std_fs::path props_file);
  std_fs::path DetermineDebugDRIDir();

 public:
  KFDNode(std_fs::path kfd_dir);
  KFDNode();
  std_fs::path Path() const noexcept;
  const sift_kfd_node_properties_t &Properties() const noexcept;
  int Instance() const noexcept;
  int GPUID() const noexcept;
};
}  // namespace sift
#endif  // _SIFT_KFD_NODE_H_
