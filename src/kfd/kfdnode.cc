/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#include "kfdnode.h"

#include "../sift/logging.h"
#include "../sift/statuserror.h"
#include "rocsift/status.h"

namespace sift
{

KFDNode::KFDNode()
    : node_(-1), kfd_dir_(std_fs::path()), kfd_properties_(ParseKFDProperties("")), gpu_id_(-1)
{
}

KFDNode::KFDNode(std_fs::path kfd_dir)
    : node_(std::stoi(kfd_dir.filename())),
      kfd_dir_(kfd_dir),
      kfd_properties_(ParseKFDProperties(kfd_dir / "properties")),
      gpu_id_([&]() {
        std::ifstream f(kfd_dir / "gpu_id");
        std::stringstream buffer;
        buffer << f.rdbuf();
        return std::stoi(buffer.str());
      }())
{
  // first, we have to determine the PCIe device to know
  // what KFD nodes are part of the same device.
  // Then, we can look through the debug/dri directories, matching pci device
  // with the "name" file, and if amdgpu* files exist, that is the proper dri filepath.
}

std_fs::path KFDNode::Path() const noexcept { return kfd_dir_; }

std_fs::path KFDNode::DetermineDebugDRIDir() { return std_fs::path(); }

sift_kfd_node_properties_t KFDNode::ParseKFDProperties(std_fs::path props_file)
{
  sift_kfd_node_properties_t props = {0};
  if (props_file.empty()) {
    return props;
  }
  // clang-format off
	const std::regex cpu_cores_count_re(R"(cpu_cores_count\s+(\w+))");
	const std::regex simd_count_re(R"(simd_count\s+(\w+))");
	const std::regex mem_banks_count_re(R"(mem_banks_count\s+(\w+))");
	const std::regex caches_count_re(R"(caches_count\s+(\w+))");
	const std::regex io_links_count_re(R"(io_links_count\s+(\w+))");
	const std::regex p2p_links_count_re(R"(p2p_links_count\s+(\w+))");
	const std::regex cpu_core_id_base_re(R"(cpu_core_id_base\s+(\w+))");
	const std::regex simd_id_base_re(R"(simd_id_base\s+(\w+))");
	const std::regex max_waves_per_simd_re(R"(max_waves_per_simd\s+(\w+))");
	const std::regex lds_size_in_kb_re(R"(lds_size_in_kb\s+(\w+))");
	const std::regex gds_size_in_kb_re(R"(gds_size_in_kb\s+(\w+))");
	const std::regex num_gws_re(R"(num_gws \s+(\w+))");
	const std::regex wave_front_size_re(R"(wave_front_size\s+(\w+))");
	const std::regex array_count_re(R"(array_count\s+(\w+))");
	const std::regex simd_arrays_per_engine_re( R"(simd_arrays_per_engine\s+(\w+))");
	const std::regex cu_per_simd_array_re(R"(cu_per_simd_array\s+(\w+))");
	const std::regex simd_per_cu_re(R"(simd_per_cu\s+(\w+))");
	const std::regex max_slots_scratch_cu_re( R"(max_slots_scratch_cu\s+(\w+))");
	const std::regex gfx_target_version_re(R"(gfx_target_version\s+(\w+))");
	const std::regex vendor_id_re(R"(vendor_id\s+(\w+))");
	const std::regex device_id_re(R"(device_id\s+(\w+))");
	const std::regex location_id_re(R"(location_id\s+(\w+))");
	const std::regex domain_id_re(R"(domain\s+(\w+))");
	const std::regex drm_render_minor_re(R"(drm_render_minor\s+(\w+))");
	const std::regex hive_id_re(R"(hive_id\s+(\w+))");
	const std::regex num_sdma_engines_re(R"(num_sdma_engines\s+(\w+))");
	const std::regex num_sdma_xgmi_engines_re( R"(num_sdma_xgmi_engines\s+(\w+))");
	const std::regex num_sdma_queues_per_engine_re( R"(num_sdma_queues_per_engine\s+(\w+))");
	const std::regex num_cp_queues_re(R"(num_cp_queues\s+(\w+))");
	const std::regex max_engine_clk_fcompute_re( R"(max_engine_clk_fcompute\s+(\w+))");
	const std::regex local_mem_size_re(R"(local_mem_size\s+(\w+))");
	const std::regex fw_version_re(R"(fw_version\s+(\w+))");
	const std::regex capability_re(R"(capability\s+(\w+))");
	const std::regex debug_prop_re(R"(debug_prop\s+(\w+))");
	const std::regex sdma_fw_version_re(R"(sdma_fw_version\s+(\w+))");
	const std::regex unique_id_re(R"(unique_id\s+(\w+))");
	const std::regex num_xcc_re(R"(num_xcc\s+(\w+))");
	const std::regex max_engine_clk_ccompute_re( R"(max_engine_clk_ccompute\s+(\w+))");
  // clang-format on

  std::string line;
  std::ifstream f(props_file.string());
  while (std::getline(f, line)) {
    std::smatch match;
    if (std::regex_search(line, match, cpu_cores_count_re)) {
      props.cpu_cores_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, simd_count_re)) {
      props.simd_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, mem_banks_count_re)) {
      props.mem_banks_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, caches_count_re)) {
      props.caches_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, io_links_count_re)) {
      props.io_links_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, p2p_links_count_re)) {
      props.p2p_links_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, cpu_core_id_base_re)) {
      props.cpu_core_id_base = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, simd_id_base_re)) {
      props.simd_id_base = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, max_waves_per_simd_re)) {
      props.max_waves_per_simd = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, lds_size_in_kb_re)) {
      props.lds_size_in_kb = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, gds_size_in_kb_re)) {
      props.gds_size_in_kb = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_gws_re)) {
      props.num_gws = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, wave_front_size_re)) {
      props.wave_front_size = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, array_count_re)) {
      props.array_count = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, simd_arrays_per_engine_re)) {
      props.simd_arrays_per_engine = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, cu_per_simd_array_re)) {
      props.cu_per_simd_array = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, simd_per_cu_re)) {
      props.simd_per_cu = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, max_slots_scratch_cu_re)) {
      props.max_slots_scratch_cu = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, gfx_target_version_re)) {
      props.gfx_target_version = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, vendor_id_re)) {
      props.vendor_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, device_id_re)) {
      props.device_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, location_id_re)) {
      props.location_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, domain_id_re)) {
      props.domain_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, drm_render_minor_re)) {
      props.drm_render_minor = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, hive_id_re)) {
      props.hive_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_sdma_engines_re)) {
      props.num_sdma_engines = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_sdma_xgmi_engines_re)) {
      props.num_sdma_xgmi_engines = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_sdma_queues_per_engine_re)) {
      props.num_sdma_queues_per_engine = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_cp_queues_re)) {
      props.num_cp_queues = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, max_engine_clk_fcompute_re)) {
      props.max_engine_clk_fcompute = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, local_mem_size_re)) {
      props.local_mem_size = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, fw_version_re)) {
      props.fw_version = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, capability_re)) {
      props.capability = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, debug_prop_re)) {
      props.debug_prop = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, sdma_fw_version_re)) {
      props.sdma_fw_version = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, unique_id_re)) {
      props.unique_id = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, num_xcc_re)) {
      props.num_xcc = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
    if (std::regex_search(line, match, max_engine_clk_ccompute_re)) {
      props.max_engine_clk_ccompute = std::strtoul(match[1].str().c_str(), nullptr, 10);
      continue;
    }
  }

  char* char_override = std::getenv("ROCSIFT_DEVID_OVERRIDE");

  if (char_override == nullptr) {
    return props;
  }

  std::string overrides_string(char_override);

  const int BUS_ID_SHIFT = 0x8;
  const int BUS_ID_MASK = 0xff;
  const int DEVICE_ID_SHIFT = 0x3;
  const int DEVICE_ID_MASK = 0x1f;
  const int FUNCTION_ID_MASK = 0x7;

  uint32_t bus_id = (props.location_id >> BUS_ID_SHIFT) & BUS_ID_MASK;
  uint32_t device_id = (props.location_id >> DEVICE_ID_SHIFT) & DEVICE_ID_MASK;
  uint32_t function_id = props.location_id & FUNCTION_ID_MASK;

  // EXAMPLE OVERRIDE: 0:83:00.0->0x753,0.03:00.0->0x74a1,0.04:00.0->0x234
  std::regex regex_pattern(
      R"(([0-9a-fA-F]+)[.:]([0-9a-fA-F]+):([0-9a-fA-F]+)\.([0-9a-fA-F]+)->(0[xX])?([0-9a-fA-F]+))");
  std::stringstream ss(overrides_string);
  std::vector<std::string> override_strings;
  std::string item;

  while (std::getline(ss, item, ',')) {
    override_strings.push_back(item);
  }

  std::vector<std::smatch> overrides;
  overrides.reserve(override_strings.size());

  for (const auto& entry : override_strings) {
    std::smatch match;
    if (!std::regex_match(entry, match, regex_pattern)) {
      Logger()
          ->error("Invalid DEVID OVERRIDE provided: \"{}\" Please see example: 0.83:00.0->0x753",
                  entry);
      throw StatusError(SIFT_STATUS_ERROR);
    } else {
      overrides.push_back(match);
    }
  }

  for (const auto& match : overrides) {
    try {
      uint32_t override_domain = std::stoul(match[1].str(), nullptr, 16);
      uint32_t override_bus = std::stoul(match[2].str(), nullptr, 16);
      uint32_t override_device = std::stoul(match[3].str(), nullptr, 16);
      uint32_t override_function = std::stoul(match[4].str(), nullptr, 16);
      uint32_t override_did = std::stoul(match[6].str(), nullptr, 16);

      if (override_domain == props.domain_id && override_bus == bus_id &&
          override_device == device_id && override_function == function_id) {
        Logger()->info("DEVID Override applied! {:04x}:{:02x}:{:02x}.{:01x} {:08x} --> {:08x}",
                       override_domain, override_bus, override_device, override_function,
                       props.device_id, override_did);
        props.device_id = override_did;
      }
    } catch (const std::invalid_argument& e) {
      Logger()
          ->error("Invalid DEVID OVERRIDE provided: \"{}\" Please see example: 0.83:00.0->0x753",
                  match[0].str());
      throw;
    } catch (const std::out_of_range& e) {
      Logger()
          ->error("Invalid DEVID OVERRIDE provided: \"{}\" Please see example: 0.83:00.0->0x753",
                  match[0].str());
      throw;
    }
  }

  return props;
}

const sift_kfd_node_properties_t& KFDNode::Properties() const noexcept { return kfd_properties_; }

int KFDNode::Instance() const noexcept { return node_; }

int KFDNode::GPUID() const noexcept { return gpu_id_; }

}  // namespace sift
