#include <iostream>
#include <fstream>

#include "rocsift/sift.h"
#include "CLI/CLI.hpp"

uint64_t GFXHUB_OFFSET(int xcc_id) { return 0x40000 * xcc_id; }

const size_t DATA_WIDTH = 4;

int main(int argc, char** argv)
{
  CLI::App app;

  int xcc_id;
  size_t size;
  bool verbose = false;

  app.add_option("-x,--xcc_die_id", xcc_id, "XCC Die ID to target")
      ->check(CLI::Number)
      ->default_val(0);
  app.add_option("-s, --size", size, "Number of bytes to read, must be multiple of 4")
      ->check(CLI::Number)
      ->default_val(1024 * DATA_WIDTH);
  app.add_flag("-v, --verbose", verbose,
               "Verbose output: note will corrupt binary output but useful for debug")
      ->default_val(false);
  CLI11_PARSE(app, argc, argv);

  // iterate through partitions until you find
  // one with the xcc_id requested.

  uint64_t CPC_SCRATCH_INDEX = GFXHUB_OFFSET(xcc_id) + 0x8240;
  uint64_t CPC_SCRATCH_DATA = GFXHUB_OFFSET(xcc_id) + 0x8244;

  if (size % DATA_WIDTH != 0) {
    fprintf(stderr, "--size must be a multiple of 4");
    return -1;
  }

  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "Failed to initialize sift");
    sift_destroy();
    return -1;
  }

  int num_parts;
  rc = sift_get_partition_count(&num_parts);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition count\n");
    sift_destroy();
    return -1;
  }

  bool found = false;
  int logical_xcc_id;
  sift_partition_t part;
  int part_id;
  for (int p = 0; p < num_parts; p++) {
    rc = sift_get_partition(p, &part);
    if (rc != SIFT_STATUS_SUCCESS) {
      fprintf(stderr, "failed to get partition %d\n", p);
      sift_destroy();
      return -1;
    }

    int num_xcc;
    rc = sift_partition_get_xcc_count(part, &num_xcc);
    if (rc != SIFT_STATUS_SUCCESS) {
      fprintf(stderr, "failed to get partition %d xcc count\n", p);
      sift_destroy();
      return -1;
    }
    int* xcc_ids = (int*)malloc(sizeof(int) * num_xcc);
    if (!xcc_ids) {
      fprintf(stderr, "failed to allocate %ld bytes for xcc_ids\n", sizeof(int) * num_xcc);
      sift_destroy();
      return -1;
    }
    rc = sift_partition_get_xcc_die_ids(part, xcc_ids, num_xcc);
    if (rc != SIFT_STATUS_SUCCESS) {
      fprintf(stderr, "failed to get partition %d xcc count\n", p);
      free(xcc_ids);
      sift_destroy();
      return -1;
    }
    for (int i = 0; i < num_xcc; i++) {
      if (xcc_ids[i] == xcc_id) {
        part_id = p;
        logical_xcc_id = i;
        found = true;
        break;
      }
    }
    free(xcc_ids);
    if (found) {
      break;
    }
  }

  if (!found) {
    fprintf(stderr, "failed to find partition with XCC_DIE_ID %d\n", xcc_id);
    sift_destroy();
    return -1;
  }

  if (verbose) {
    fprintf(stdout, "Using partition %d logical XCC_ID %d to target XCC_DIE_ID %d\n", part_id,
            logical_xcc_id, xcc_id);
  }

  size_t num_loops = size / DATA_WIDTH;
  size_t data_size = num_loops * DATA_WIDTH;

  uint32_t* data = (uint32_t*)malloc(data_size);
  if (!data) {
    fprintf(stderr, "Failed to allocate %ld bytes for data buffer\n", data_size);
    sift_destroy();
    return -1;
  }

  rc = sift_partition_write_reg32(part, 0x0, SIFT_APER_MMIO, CPC_SCRATCH_INDEX);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to write CPC_SCRATCH_INDEX\n");
    free(data);
    sift_destroy();
    return -1;
  }

  for (size_t i = 0; i < num_loops; i++) {
    rc = sift_partition_read_reg32(part, &data[i], SIFT_APER_MMIO, CPC_SCRATCH_DATA);
    if (rc != SIFT_STATUS_SUCCESS) {
      fprintf(stderr, "failed to read CPC_SCRATCH_DATA\n");
      free(data);
      sift_destroy();
      return -1;
    }
  }

  std::ofstream output_stream;
  output_stream.open("/dev/stdout", std::ios::binary);
  output_stream.write(reinterpret_cast<const char*>(data), data_size);
  output_stream.close();

  free(data);
  rc = sift_destroy();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "Error destroying sift");
    return -1;
  }

  return 0;
}
