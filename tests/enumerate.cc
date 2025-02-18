/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "rocsift/kfd.h"
#include "rocsift/sift.h"

#include <set>

TEST_CASE("Enumerate Partitions")
{
  bool sift_initialized = false;
  int num_devices;
  REQUIRE(sift_init() == SIFT_STATUS_SUCCESS);
  sift_initialized = true;
  REQUIRE(sift_get_device_count(&num_devices) == SIFT_STATUS_SUCCESS);

  std::set<int> node_ids;
  int total_num_partitions = 0;
  for (int i = 0; i < num_devices; i++) {
    int num_partitions = 0;
    sift_device_t dev;
    REQUIRE(sift_get_device(i, &dev) == SIFT_STATUS_SUCCESS);
    REQUIRE(sift_device_get_partition_count(dev, &num_partitions) == SIFT_STATUS_SUCCESS);

    for (int p = 0; p < num_partitions; p++) {
      sift_partition_t part;
      REQUIRE(sift_device_get_partition(dev, &part, p) == SIFT_STATUS_SUCCESS);

      sift_kfd_node_t node;
      REQUIRE(sift_partition_get_kfd_node(part, &node) == SIFT_STATUS_SUCCESS);

      int node_id;
      REQUIRE(sift_kfd_node_get_id(node, &node_id) == SIFT_STATUS_SUCCESS);
      REQUIRE(node_ids.find(node_id) == node_ids.end());
      node_ids.insert(node_id);

      // TODO: determine if these are guaranteed to be unique. They should be for rls lookup..
      int gpu_id;
      REQUIRE(sift_kfd_node_get_gpuid(node, &gpu_id) == SIFT_STATUS_SUCCESS);

      sift_kfd_node_properties_t props;
      REQUIRE(sift_kfd_node_get_properties(node, &props) == SIFT_STATUS_SUCCESS);
      std::cout << total_num_partitions << ":  Device " << i << "." << p << " Node " << node_id
                << " GPU_ID " << std::hex << std::setw(8) << std::setfill('0') << gpu_id << " "
                << props.vendor_id << ":" << props.device_id << std::endl;
      total_num_partitions++;
    }
  }
  REQUIRE(sift_destroy() == SIFT_STATUS_SUCCESS);
}
