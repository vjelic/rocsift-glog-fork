/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "kfddebugfs.h"

#include "../sift/logging.h"
#include "../sift/statuserror.h"
#include "../pm4/pm4.h"

namespace sift
{

KFDDebugFS::KFDDebugFS(std_fs::path root) : root_(root)
{
  if (!std_fs::exists(root_)) {
    Logger()->error("KFDDebugFS is not accessible or does not exist, root: {}", root_.string());
    throw StatusError(SIFT_STATUS_ERROR);
  }
}

const ::std::string KFDDebugFS::GetRunlists()
{
  std_fs::path rlspath(root_ / "rls");
  std::ifstream f(rlspath);
  std::stringstream buffer;
  buffer << f.rdbuf();
  return buffer.str();
}

const std::string KFDDebugFS::GetMQDs()
{
  std_fs::path rlspath(root_ / "mqds");
  std::ifstream f(rlspath);
  std::stringstream buffer;
  buffer << f.rdbuf();
  return buffer.str();
}

const std::string KFDDebugFS::GetHQDs()
{
  std_fs::path rlspath(root_ / "hdqs");
  std::ifstream f(rlspath);
  std::stringstream buffer;
  buffer << f.rdbuf();
  return buffer.str();
}

}  // namespace sift
