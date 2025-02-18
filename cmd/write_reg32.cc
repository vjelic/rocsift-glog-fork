#include "rocsift/sift.h"
#include "CLI/CLI.hpp"

int main(int argc, char** argv)
{
  CLI::App app;

  int partition;
  uint64_t addr;
  uint32_t val;
  std::string aperture_str;
  sift_aperture_t aperture;

  app.add_option("-p,--partition", partition, "GPU partition to target")
      ->check(CLI::Number)
      ->default_val(0);
  app.add_option("addr", addr, "Virtual Address to translate")->required()->check(CLI::Number);
  app.add_option("value", val, "32-bit value to write")->required()->check(CLI::Number);
  app.add_option("--aperture", aperture_str, "MMIO, or SMN")->default_val("MMIO");
  CLI11_PARSE(app, argc, argv);

  if (aperture_str == "MMIO") {
    aperture = SIFT_APER_MMIO;
  } else if (aperture_str == "SMN") {
    aperture = SIFT_APER_SMN;
  } else {
    fprintf(stderr, "--aperture must either be MMIO or SMN, not '%s'\n", aperture_str.c_str());
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

  sift_partition_t part;
  rc = sift_get_partition(partition, &part);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition %d\n", partition);
    sift_destroy();
    return -1;
  }
  rc = sift_partition_write_reg32(part, val, aperture, addr);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to write aperture %s address 0x%016lx with data 0x%08x\n",
            aperture_str.c_str(), addr, val);
    sift_destroy();
    return -1;
  }
  printf("0x%08x\n", val);

  if (sift_destroy() != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to destroy sift\n");
    return -1;
  };
  return 0;
}
