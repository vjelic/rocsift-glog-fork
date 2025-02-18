#include "rocsift/kfd.h"
#include "rocsift/sift.h"
#include "CLI/CLI.hpp"

#include <vector>

#define SIFTTHROW(cmd)                                                    \
  do {                                                                    \
    sift_status_t err = cmd;                                              \
    if (err != SIFT_STATUS_SUCCESS) {                                     \
      throw std::runtime_error(std::string("Call to " #cmd " failed. ") + \
                               std::to_string(__LINE__));                 \
    }                                                                     \
  } while (false)

uint64_t GFXHUB_OFFSET(int xcc_id)
{
  // TODO: Replace with a lookup table. This is specific to MI300.
  return 0x40000 * xcc_id;
}

int main(int argc, char** argv)
{
  CLI::App app{"List rocsift partitions and devices"};
  CLI11_PARSE(app, argc, argv);

  bool sift_initialized = false;
  try {
    int num_partitions;
    SIFTTHROW(sift_init());
    sift_initialized = true;
    SIFTTHROW(sift_get_partition_count(&num_partitions));

    auto grbm_status_offset = 0x8010;  // TODO: create lookup table

    for (int i = 0; i < num_partitions; i++) {
      sift_partition_t partition;
      SIFTTHROW(sift_get_partition(i, &partition));
      int num_xcc = 1;
      SIFTTHROW(sift_partition_get_xcc_count(partition, &num_xcc));
      std::vector<int> xcc_ids(num_xcc);
      SIFTTHROW(sift_partition_get_xcc_die_ids(partition, xcc_ids.data(), num_xcc));

      std::string status = "IDLE";
      for (int j = 0; j < num_xcc; j++) {
        uint32_t grbm_status = 0;
        SIFTTHROW(sift_partition_read_reg32(partition, &grbm_status, SIFT_APER_MMIO,
                                            GFXHUB_OFFSET(xcc_ids[j]) + grbm_status_offset));
        bool busy = (grbm_status >> 31 /*GUI_ACTIVE*/) & 0x1;
        if (busy) {
          status = "BUSY";
          break;
        }
      }
      std::cout << "partition " << i << " status:" << status << std::endl;
    }

    return 0;
  } catch (std::exception& e) {
    std::cerr << e.what() << ", Are you sure you have proper permissions? (try sudo)" << std::endl;
    if (sift_initialized) {
      SIFTTHROW(sift_destroy());
    }
    return -1;
  }
}
