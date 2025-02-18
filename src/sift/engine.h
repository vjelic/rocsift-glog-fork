/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_ENGINE_H_
#define _SIFT_ENGINE_H_

#include <vector>
#include <memory>

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include "../kfd/kfdtop.h"
#include "device.h"
#include "partition.h"

namespace sift
{
class Engine
{
 private:
  class LoggerManager
  {
   public:
    LoggerManager();
  };
  LoggerManager logman_;
  KFDHandle kfd_;
  std::vector<std::unique_ptr<Device> > devices_;
  std::vector<Partition *> partitions_;

  std::vector<std::unique_ptr<Device> > EnumerateDevices();
  std::vector<Partition *> EnumeratePartitions();

 public:
  Engine();
  std::vector<std::unique_ptr<Device> > &Devices();
  std::vector<Partition *> &Partitions();
  KFDHandle &KFD();
};

}  // namespace sift

#endif  // _SIFT_ENGINE_H
