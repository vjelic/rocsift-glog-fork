/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_IP_DISCOVERY_H_
#define _SIFT_IP_DISCOVERY_H_

#if __has_include(<filesystem>)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include <map>
#include <string>
#include <vector>

namespace sift
{

namespace ipdiscovery
{

class IPInstance
{
 private:
  uint8_t num_instance_;
  std::vector<uint64_t> base_addrs_;
  uint8_t harvest_;
  uint16_t hwid_;
  uint8_t major_;
  uint8_t minor_;
  uint8_t revision_;

 public:
  IPInstance();
  IPInstance(std_fs::path ip_instance_root);
  IPInstance(uint8_t num_instance, const std::vector<uint64_t>& base_addrs, uint8_t harvest,
             uint16_t hwid, uint8_t major, uint8_t minor, uint8_t revision);
  uint8_t NumInstance() const noexcept;
  const std::vector<uint64_t>& BaseAddrs() const noexcept;
  uint8_t Harvest() const noexcept;
  uint16_t HWID() const noexcept;
  uint8_t Major() const noexcept;
  uint8_t Minor() const noexcept;
  uint8_t Revision() const noexcept;
};

class IP
{
 private:
  std::string name_;
  std::vector<IPInstance> instances_;

 public:
  IP();
  IP(std_fs::path ip_root);
  IP(const std::string& name, const std::vector<IPInstance>& instances);
  const std::string& Name() const noexcept;
  const std::vector<IPInstance>& Instances() const noexcept;
};

class Die
{
 private:
  int id_;
  int num_ips_;
  std::map<std::string, IP> ips_;

 public:
  Die();
  Die(std_fs::path die_root);
  const IP& LookupIP(const std::string& name) const;
  int ID() const noexcept;
};

class Root
{
 private:
  std::vector<Die> dies_;

 public:
  Root();
  Root(std_fs::path root);
  const std::vector<Die>& Dies() const noexcept;
};

}  // namespace ipdiscovery
}  // namespace sift

#endif  // _SIFT_IP_DISCOVERY_H_
