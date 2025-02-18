/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_DRM_H_
#define _SIFT_DRM_H_

#include <vector>

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

class DRM;      // forward declaration
class DRMNode;  // forward delcaration

struct XGMIInfo {
  uint64_t hive_id;
  uint64_t device_id;
  uint64_t physical_id;
  std::vector<DRMNode*> nodes;
};

class DRMNode
{
 private:
  DRM* drm_;
  std_fs::path path_;
  XGMIInfo xgmi_;
  std_fs::path card_path_;
  std_fs::path render_path_;
  std_fs::path control_path_;

  void InitXGMIInfo();
  friend class DRM;

 public:
  DRMNode(DRM* drm, std_fs::path path);
  std::string Name() const;
  std::string CardName() const;
  std::string RenderName() const;
  XGMIInfo& XGMI();
  size_t TotalVRAMBytes() const;
};

class DRM
{
 private:
  std_fs::path root_;
  std::vector<DRMNode> nodes_;
  DRM(std_fs::path root = "/sys/class/drm");

 public:
  ~DRM() = default;
  DRM(const DRM&) = delete;
  void operator=(const DRM&) = delete;
  static DRM& GetInstance();
  const std::vector<DRMNode>& Nodes();
  DRMNode* NodeByName(const std::string& name);
};

}  // namespace sift

#endif  // _SIFT_DRM_H_
