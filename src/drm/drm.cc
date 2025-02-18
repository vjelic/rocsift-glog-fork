/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include "drm.h"

#include "../sift/logging.h"

namespace sift
{

DRMNode::DRMNode(DRM* drm, std_fs::path path)
    : drm_(drm), path_(path), xgmi_(XGMIInfo{.hive_id = 0})
{
  Logger()->debug("Created DRMNode at path {} with name {}", path_.string(), Name());
  auto drm_path = path_ / "device" / "drm";

  if (!std_fs::is_directory(drm_path)) {
    drm_path = path_ / "device" / "device" / "drm";
    if (!std_fs::is_directory(drm_path)) {
      Logger()->warn("Failed to find DRM subdirectory for Node {}", Name());
      return;
    }
  }

  for (auto const& entry : std_fs::directory_iterator{drm_path}) {
    if (!std_fs::is_directory(entry)) {
      continue;
    }
    std::string name = entry.path().filename();
    if (card_path_.empty()) {
      std::string card = "card";
      auto res = std::mismatch(card.begin(), card.end(), name.begin());
      if (res.first == card.end()) {
        // card is a prefix of name
        card_path_ = entry.path();
        continue;
      }
    }

    if (render_path_.empty()) {
      std::string render = "render";
      auto res = std::mismatch(render.begin(), render.end(), name.begin());
      if (res.first == render.end()) {
        // card is a prefix of name
        render_path_ = entry.path();
        continue;
      }
    }

    if (control_path_.empty()) {
      std::string control = "control";
      auto res = std::mismatch(control.begin(), control.end(), control.begin());
      if (res.first == control.end()) {
        // card is a prefix of name
        control_path_ = entry.path();
        continue;
      }
    }
  }
}

void DRMNode::InitXGMIInfo()
{
  auto xgmi_devid_path = path_ / "device" / "xgmi_device_id";
  if (!std_fs::exists(xgmi_devid_path)) {
    Logger()->debug("DRMNode {} is not part of a xgmi hive", Name());
    return;
  }

  std::ifstream devid(xgmi_devid_path.string());
  std::stringstream ss;
  ss << devid.rdbuf();
  xgmi_.device_id = std::strtoull(ss.str().c_str(), nullptr, 10);
  Logger()->debug("DRMNode {} is  xgmi_device_id = {}", Name(), xgmi_.device_id);

  auto xgmi_physical_id_path = path_ / "device" / "xgmi_physical_id";
  if (!std_fs::exists(xgmi_physical_id_path)) {
    Logger()->debug("DRMNode {} does not have a xgmi_physical_id_path", Name());
    return;
  }
  std::ifstream phyid(xgmi_physical_id_path.string());
  ss.str("");
  ss.clear();
  ss << phyid.rdbuf();
  xgmi_.physical_id = std::strtoull(ss.str().c_str(), nullptr, 10);
  Logger()->debug("DRMNode {} has  xgmi_physical_id = {}", Name(), xgmi_.physical_id);

  auto xgmi_hive_info_path = path_ / "device" / "xgmi_hive_info";
  if (!std_fs::exists(xgmi_hive_info_path)) {
    return;
  }
  Logger()->debug("DRMNode {} is in an XGMI hive", Name());
  if (!std_fs::is_directory(xgmi_hive_info_path)) {
    Logger()->warn("DRMNode {}  xgmi_hive_info_path not found", Name());
    return;
  }
  std::vector<DRMNode*> nodes;
  auto hive_id_path = xgmi_hive_info_path / "xgmi_hive_id";
  if (!std_fs::exists(hive_id_path)) {
    Logger()->warn("DRMNode {} xgmi_hive_id path not found", Name());
    return;
  }
  std::ifstream hiveid(hive_id_path.string());
  ss.str("");
  ss.clear();
  ss << hiveid.rdbuf();
  xgmi_.hive_id = std::strtoull(ss.str().c_str(), nullptr, 10);
  Logger()->debug("DRMNode {} is part of hive with ID {}", Name(), xgmi_.hive_id);

  std::set<std_fs::path> node_paths;
  for (auto const& dir_entry : std_fs::directory_iterator{xgmi_hive_info_path}) {
    if (std_fs::is_directory(dir_entry)) {
      node_paths.insert(dir_entry.path());
    }
  }
  for (auto& n : node_paths) {
    bool found = false;
    for (auto const& dir_entry : std_fs::directory_iterator{n / "drm"}) {
      if (!std_fs::is_directory(dir_entry)) {
        continue;
      }
      std::string card = "card";
      std::string name = dir_entry.path().filename();
      auto res = std::mismatch(card.begin(), card.end(), name.begin());
      if (res.first == card.end()) {
        // card is a prefix of name
        auto matching_node = drm_->NodeByName(name);
        if (matching_node) {
          Logger()->debug("DRMNode {} XGMI topo: {} -> {}", Name(), n.filename().string(),
                          matching_node->Name());
          found = true;
          xgmi_.nodes.push_back(matching_node);
          break;
        }
      }
    }
    if (!found) {
      Logger()->error("FAILED TO FIND DRM NODE {}", n.string());
      xgmi_.nodes = std::vector<DRMNode*>();
      break;
    }
  }

  // sort the nodes by physical hive id
  std::sort(xgmi_.nodes.begin(), xgmi_.nodes.end(), [](const DRMNode* lhs, const DRMNode* rhs) {
    return lhs->xgmi_.physical_id < rhs->xgmi_.physical_id;
  });
}

std::string DRMNode::Name() const { return path_.filename(); }

std::string DRMNode::CardName() const { return card_path_.filename(); }

std::string DRMNode::RenderName() const { return render_path_.filename(); }

size_t DRMNode::TotalVRAMBytes() const
{
  auto vrampath = path_ / "device" / "mem_info_vram_total";
  if (!std_fs::exists(vrampath)) {
    return 0;
  }
  std::ifstream f(vrampath.string());
  std::stringstream ss;
  ss << f.rdbuf();
  return std::strtoull(ss.str().c_str(), 0x0, 10);
}

XGMIInfo& DRMNode::XGMI() { return xgmi_; }

DRM::DRM(std_fs::path root)
    : root_(root), nodes_([this]() {
        std::set<std_fs::path> paths;
        std::vector<DRMNode> nodes;

        // order the DRM ndoes alphabetically
        for (auto const& dir_entry : std_fs::directory_iterator{root_}) {
          paths.insert(dir_entry.path());
        }
        for (auto& p : paths) {
          if (std_fs::is_directory(p)) {
            nodes.push_back(DRMNode(this, p));
          }
        }
        for (auto& n : nodes) {
          n.InitXGMIInfo();
        }
        return nodes;
      }())
{
}

DRMNode* DRM::NodeByName(const std::string& name)
{
  for (auto& n : nodes_) {
    if (n.Name() == name) {
      return &n;
    }
  }
  return nullptr;
}

const std::vector<DRMNode>& DRM::Nodes() { return nodes_; }

DRM& DRM::GetInstance()
{
  static DRM drm = DRM();
  return drm;
}

}  // namespace sift
