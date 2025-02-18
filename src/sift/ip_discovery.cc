/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <algorithm>
#include <fstream>

#include "logging.h"
#include "ip_discovery.h"
#include "statuserror.h"

namespace sift
{

namespace ipdiscovery
{

static std::string ReadFile(std_fs::path path)
{
  std::ifstream p(path);
  std::stringstream buffer;
  buffer << p.rdbuf();
  return buffer.str();
}

IPInstance::IPInstance()
    : num_instance_(0), base_addrs_({}), harvest_(0), hwid_(0), major_(0), minor_(0), revision_(0)
{
}

IPInstance::IPInstance(uint8_t num_instance, const std::vector<uint64_t>& base_addrs,
                       uint8_t harvest, uint16_t hwid, uint8_t major, uint8_t minor,
                       uint8_t revision)
    : num_instance_(num_instance),
      base_addrs_(base_addrs),
      harvest_(harvest),
      hwid_(hwid),
      major_(major),
      minor_(minor),
      revision_(revision)
{
}

IPInstance::IPInstance(std_fs::path ip_instance_root)
    : num_instance_([&]() {
        Logger()->debug("\t\t\tIPInst: " + ip_instance_root.string());
        return std::stoul(ReadFile(ip_instance_root / "num_instance"), nullptr, 10);
      }()),
      base_addrs_([&](std::string str, size_t num_addrs) {
        std::istringstream ss(str);
        std::string line;
        std::vector<uint64_t> base;
        while (std::getline(ss, line)) {
          base.push_back(std::stoull(line, nullptr, 0));
        }

        if (base.size() != num_addrs) {
          Logger()->error("IPInstance num_base_addresses mismatch: {}", ip_instance_root.string());
          throw StatusError(SIFT_STATUS_OUT_OF_RANGE);
        }

        return base;
      }(ReadFile(ip_instance_root / "base_addr"),
                  std::stoul(ReadFile(ip_instance_root / "num_base_addresses"), nullptr, 0))),
      harvest_(std::stoul(ReadFile(ip_instance_root / "harvest"), nullptr, 0)),
      hwid_(std::stoul(ReadFile(ip_instance_root / "hw_id"), nullptr, 0)),
      major_(std::stoul(ReadFile(ip_instance_root / "major"), nullptr, 0)),
      minor_(std::stoul(ReadFile(ip_instance_root / "minor"), nullptr, 0)),
      revision_(std::stoul(ReadFile(ip_instance_root / "revision"), nullptr, 0))
{
}

uint8_t IPInstance::NumInstance() const noexcept { return num_instance_; }
const std::vector<uint64_t>& IPInstance::BaseAddrs() const noexcept { return base_addrs_; }
uint8_t IPInstance::Harvest() const noexcept { return harvest_; }
uint16_t IPInstance::HWID() const noexcept { return hwid_; }
uint8_t IPInstance::Major() const noexcept { return major_; }
uint8_t IPInstance::Minor() const noexcept { return minor_; }
uint8_t IPInstance::Revision() const noexcept { return revision_; }

IP::IP() : name_(""), instances_({}) {}

IP::IP(const std::string& name, const std::vector<IPInstance>& instances) : instances_(instances) {}

IP::IP(std_fs::path ip_root)
    : name_([&]() {
        Logger()->debug("\t\tIP: " + ip_root.string());
        return ip_root.filename().string();
      }()),
      instances_([&]() {
        std::vector<IPInstance> inst;
        for (auto const& dir_entry : std_fs::directory_iterator{ip_root}) {
          if (!std_fs::is_directory(dir_entry)) {
            continue;
          }
          inst.push_back(IPInstance(dir_entry.path()));
        }
        std::sort(inst.begin(), inst.end(), [](const IPInstance& a, const IPInstance& b) {
          return a.NumInstance() < b.NumInstance();
        });
        return inst;
      }())
{
}
const std::string& IP::Name() const noexcept { return name_; }

const std::vector<IPInstance>& IP::Instances() const noexcept { return instances_; }

Die::Die() : id_(0), num_ips_(0), ips_({}) {}

Die::Die(std_fs::path die_root)
    : id_([&]() {
        Logger()->debug("\tDIE: " + die_root.string());
        return std::stoul(die_root.filename().string(), nullptr, 10);
      }()),
      num_ips_(std::stoul(ReadFile(die_root / "num_ips"), nullptr, 10)),
      ips_([&]() {
        std::map<std::string, IP> mutable_ips;
        std::vector<std_fs::path> sorted_paths;
        for (auto const& dir_entry : std_fs::directory_iterator{die_root}) {
          if (!std_fs::is_directory(dir_entry)) {
            continue;
          }
          sorted_paths.push_back(dir_entry.path());
        }

        std::sort(sorted_paths.begin(), sorted_paths.end(),
                  [](const std_fs::path& a, const std_fs::path& b) {
                    return a.filename().string() < b.filename().string();
                  });

        for (auto& path : sorted_paths) {
          auto ip = IP(path);
          mutable_ips[ip.Name()] = ip;
        }
        return mutable_ips;
      }())
{
}

int Die::ID() const noexcept { return id_; }
const IP& Die::LookupIP(const std::string& name) const { return ips_.at(name); };

Root::Root() : dies_({}) {}

Root::Root(std_fs::path root)
    : dies_([&]() {
        Logger()->debug("ROOT: " + root.string());
        std::vector<Die> mutable_dies;
        for (auto const& dir_entry : std_fs::directory_iterator{root / "die"}) {
          if (!std_fs::is_directory(dir_entry)) {
            continue;
          }
          mutable_dies.push_back(Die(dir_entry.path()));
        }
        std::sort(mutable_dies.begin(), mutable_dies.end(),
                  [](const Die& a, const Die& b) { return a.ID() < b.ID(); });
        return mutable_dies;
      }())
{
}
const std::vector<Die>& Root::Dies() const noexcept { return dies_; }

}  // namespace ipdiscovery

}  // namespace sift
