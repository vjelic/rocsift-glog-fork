/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <algorithm>
#include <sstream>
#include <fstream>

#include "kfdtop.h"
#include "../sift/logging.h"
#include "../sift/statuserror.h"

namespace sift

{
KFDProc::KFDProc(KFDHandle *kfd, int pid)
    : root_(kfd->Root() / "kfd" / "proc" / std::to_string(pid)), pid_(pid), pasid_([&]() {
        std::ifstream f(root_ / "pasid");
        std::stringstream buffer;
        buffer << f.rdbuf();
        return std::stoi(buffer.str());
      }())
{
}

int KFDProc::PID() { return pid_; };

int KFDProc::PASID() { return pasid_; };

KFDHandle::KFDHandle(std_fs::path root)
    : root_(root),
      nodes_([&]() {
        std::vector<KFDNode> nodes;

        auto topo = root_ / "kfd" / "topology" / "nodes";
        // build up partitions
        for (auto const &dir_entry : std_fs::directory_iterator{topo}) {
          // each directory here is a KFD node.
          nodes.push_back(KFDNode(dir_entry.path()));
        }
        // sort the nodes by instance number
        std::sort(nodes.begin(), nodes.end(), [](const KFDNode &n0, const KFDNode &n1) {
          return n0.Instance() < n1.Instance();
        });

        return nodes;
      }()),
      debugfs_([]() {
        try {
          return std::make_unique<KFDDebugFS>();
        } catch (std::exception &e) {
          Logger()->error(e.what());
          return std::unique_ptr<KFDDebugFS>();
        }
      }())
{
}

std::vector<KFDNode> &KFDHandle::Nodes() { return nodes_; }

std::vector<KFDProc> KFDHandle::Processes()
{
  std::vector<KFDProc> processes;
  for (auto const &dir_entry : std_fs::directory_iterator{root_ / "kfd" / "proc"}) {
    if (std_fs::is_directory(dir_entry)) {
      processes.push_back(KFDProc(this, std::stoi(dir_entry.path().filename())));
    }
  }
  return processes;
}

std_fs::path KFDHandle::Root() const noexcept { return root_; }

KFDDebugFS &KFDHandle::DebugFS()
{
  if (!debugfs_) {
    Logger()->warn("Failed to get debugfs interface -- not privileged");
    throw StatusError(SIFT_STATUS_NOT_PRIVILEGED);
  }
  return *debugfs_.get();
}

}  // namespace sift
