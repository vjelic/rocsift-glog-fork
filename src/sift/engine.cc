/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <algorithm>
#include <cstdlib>
#include <set>
#include <map>
#include <tuple>

#include "engine.h"
#include "logging.h"
#include "statuserror.h"

namespace sift
{

Engine::LoggerManager::LoggerManager()
{
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto level = spdlog::level::off;
  if (const char *cloglevel = std::getenv("ROCSIFT_LOG_LEVEL")) {
    auto console_log_level = std::string(cloglevel);
    level = spdlog::level::from_str(console_log_level);
    if ((level == spdlog::level::off) && (console_log_level != "off")) {
      Logger()->error("invalid ROCSIFT_LOG_LEVEL: {}", console_log_level);
      throw StatusError(SIFT_STATUS_ERROR);
    }
  }
  console_sink->set_level(level);
  auto log = std::make_shared<spdlog::logger>("sift", console_sink);
  spdlog::register_logger(log);
  spdlog::set_level(spdlog::level::trace);
}

Engine::Engine()
    : logman_(LoggerManager()),
      kfd_(KFDHandle()),
      devices_(EnumerateDevices()),
      partitions_(EnumeratePartitions())
{
}

std::vector<Partition *> Engine::EnumeratePartitions()
{
  std::vector<Partition *> partitions;
  for (auto &dev : devices_) {
    for (auto &part : dev->Partitions()) {
      partitions.push_back(part.get());
    }
  }
  return partitions;
}

std::vector<std::unique_ptr<Device> > Engine::EnumerateDevices()
{
  std::map<uint64_t, std::unique_ptr<DeviceBuilder> > builders;

  struct {
    std_fs::path path;
    uint64_t dbdf;
    int local_instance;
  } partinfo_t;

  std::vector<KFDNode> &nodes = kfd_.Nodes();

  int global_partition_id = 0;
  int num_builders = 0;
  for (auto &n : nodes) {
    auto props = n.Properties();
    if (!props.simd_count) {
      continue;
    }
    uint64_t bdf = props.domain_id << 16 | (props.location_id & 0xffffff8);
    auto search = builders.find(bdf);
    if (search == builders.end()) {
      builders[bdf] = std::make_unique<DeviceBuilder>(num_builders++, props.domain_id,
                                                      props.location_id);
      search = builders.find(bdf);
    }
    Logger()->debug("adding Partition {} to {:#016x}", n.Instance(), bdf);
    search->second->AddPartition(n, global_partition_id);
    global_partition_id++;
  }

  std::vector<std::unique_ptr<Device> > devices;
  for (auto &[k, builder] : builders) {
    devices.push_back(std::make_unique<Device>(this, builder->instance, builder->domain,
                                               builder->bdf, builder->nodes));
  }

  return devices;
}

std::vector<std::unique_ptr<Device> > &Engine::Devices() { return devices_; }

std::vector<Partition *> &Engine::Partitions() { return partitions_; }

KFDHandle &Engine::KFD() { return kfd_; }

}  // namespace sift
