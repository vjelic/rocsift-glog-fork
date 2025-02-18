#include "rocsift/kfd.h"
#include "rocsift/sift.h"
#include "CLI/CLI.hpp"

#define SIFTTHROW(cmd)                                                    \
  do {                                                                    \
    sift_status_t err = cmd;                                              \
    if (err != SIFT_STATUS_SUCCESS) {                                     \
      throw std::runtime_error(std::string("Call to " #cmd " failed. ") + \
                               std::to_string(__LINE__));                 \
    }                                                                     \
  } while (false)

int main(int argc, char** argv)
{
  CLI::App app{"List rocsift partitions and devices"};
  CLI11_PARSE(app, argc, argv);

  bool sift_initialized = false;
  try {
    int num_devices;
    SIFTTHROW(sift_init());
    sift_initialized = true;
    SIFTTHROW(sift_get_device_count(&num_devices));

    int total_num_partitions = 0;
    for (int i = 0; i < num_devices; i++) {
      int num_partitions = 0;
      sift_device_t dev;
      SIFTTHROW(sift_get_device(i, &dev));
      SIFTTHROW(sift_device_get_partition_count(dev, &num_partitions));

      for (int p = 0; p < num_partitions; p++) {
        sift_partition_t part;
        SIFTTHROW(sift_device_get_partition(dev, &part, p));

        sift_kfd_node_t node;
        SIFTTHROW(sift_partition_get_kfd_node(part, &node));

        int node_id;
        SIFTTHROW(sift_kfd_node_get_id(node, &node_id));

        int gpu_id;
        SIFTTHROW(sift_kfd_node_get_gpuid(node, &gpu_id));

        sift_kfd_node_properties_t props;
        SIFTTHROW(sift_kfd_node_get_properties(node, &props));
        std::cout << total_num_partitions << ":  Device " << i << "." << p << " Node " << node_id
                  << " GPU_ID " << std::hex << std::setw(8) << std::setfill('0') << gpu_id << " "
                  << props.vendor_id << ":" << props.device_id << std::endl;
        total_num_partitions++;
      }
    }

    return 0;
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    if (sift_initialized) {
      SIFTTHROW(sift_destroy());
    }
    return -1;
  }
}
