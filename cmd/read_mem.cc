#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "rocsift/sift.h"
#include "rocsift/xlator.h"

#include "CLI/CLI.hpp"

static void write_bytes_to_stdout(char *data, size_t size)
{
  while (size > 0) {
    const size_t num_written = fwrite(data, 1, size, stdout);
    if (ferror(stdout)) {
      perror("fwrite");
      exit(-1);
    }
    size -= num_written;
    data += num_written;
  }
}

/// @brief Reads `size` bytes from the physical address `addr` and writes them to stdout
static int read_phys_addr(const sift_partition_t part, const bool system, uint64_t addr,
                          uint64_t size)
{
  constexpr size_t BUF_SIZE = 2 << 20;
  std::unique_ptr<char[]> buf = std::make_unique<char[]>(BUF_SIZE);
  const sift_mem_region_t region = system ? SIFT_LINEAR_SYSTEM_MEM : SIFT_LINEAR_VIDEO_MEM;
  while (size > 0) {
    const size_t num_to_read = std::min(size, BUF_SIZE);
    const ssize_t num_read = sift_partition_read(part, buf.get(), region, addr, num_to_read);
    if (num_read < 0) {
      fprintf(stderr, "%s: error: %s\n", __func__, strerror(errno));
      return -1;
    }
    write_bytes_to_stdout(buf.get(), (size_t)num_read);
    size -= (size_t)num_read;
    addr += (size_t)num_read;
  }
  return 0;
}

/// @brief Reads `size` bytes from the virtual address `addr` and writes them to stdout
static int read_virt_addr(const int pid, const sift_partition_t part, uint64_t virt_addr,
                          uint64_t size)
{
  xlator_t xlate;
  if (sift_xlator_create_for_process(&xlate, pid, part)) {
    fprintf(stderr, "failed to create xlator, make sure you are running as root\n");
    return -1;
  }
  int ret = 0;
  fragment_list_t *list = NULL;
  fragment_list_t *current = NULL;
  constexpr bool combine_fragments = true;
  if (sift_xlator_translate_range(&xlate, &list, virt_addr, size, combine_fragments)) {
    fprintf(stderr, "failed to translate range\n");
    ret = -1;
    goto out;
  }
  current = list;
  while (current && size) {
    if (!current->fragment.flags.valid) {
      fprintf(stderr, "Page for VA 0x%lx is not valid\n", virt_addr);
      ret = -1;
      goto out;
    }
    const bool system = current->fragment.flags.system;
    // current->fragment.pa is the address of the start of the page
    const size_t virt_addr_lower_bits = virt_addr & (current->fragment.size - 1);
    const uint64_t phys_addr = current->fragment.pa | virt_addr_lower_bits;
    const size_t current_size = std::min(current->fragment.size - virt_addr_lower_bits, size);
    if (read_phys_addr(part, system, phys_addr, current_size) < 0) {
      ret = -1;
      goto out;
    }
    size -= current_size;
    virt_addr += current_size;
    current = current->next;
  }
out:
  if (list) {
    sift_xlator_fragment_list_destroy(list);
  }
  sift_xlator_destroy(&xlate);
  return ret;
}

int main(int argc, char **argv)
{
  CLI::App app;

  int pid = 0;
  int partition = 0;
  bool physical = false;
  bool system = false;
  uint64_t addr = 0;
  uint64_t size = 0;

  app.add_option("--pid", pid, "Process ID to target")->check(CLI::PositiveNumber);
  app.add_option("-p,--partition", partition, "GPU partition to target")->check(CLI::Number);
  app.add_flag("--physical", physical, "Treat the address as physical instead of virtual");
  app.add_flag("--system", system,
               "Treat the address as a system address instead of VRAM. "
               "May only be used with --physical");
  app.add_option("addr", addr, "Address to read from")->required()->check(CLI::Number);
  app.add_option("size", size, "Size in bytes to read")->required()->check(CLI::PositiveNumber);

  CLI11_PARSE(app, argc, argv);

  if (system && !physical) {
    fprintf(stderr, "Usage error: --system may only be used with --physical\n");
    return -1;
  }
  if ((pid && physical) || (!pid && !physical)) {
    fprintf(stderr, "Usage error: exactly one of --pid and --physical must be specified\n");
    return -1;
  }

  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to initialize sift\n");
    return -1;
  }
  sift_partition_t part;
  rc = sift_get_partition(partition, &part);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition %d\n", partition);
    return -1;
  }
  if (physical) {
    return read_phys_addr(part, system, addr, size);
  } else {
    return read_virt_addr(pid, part, addr, size);
  }
}
