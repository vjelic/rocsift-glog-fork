#include "stdio.h"
#include "stdlib.h"

#include "rocsift/sift.h"
#include "rocsift/xlator.h"
#include "rocsift/mtype.h"

#include "CLI/CLI.hpp"
#include <map>
#include <string>

enum class IDType : int {
  PID,
  VMID,
};

static std::map<std::string, IDType> map{{"pid", IDType::PID}, {"vmid", IDType::VMID}};

int main(int argc, char **argv)
{
  constexpr int default_size = 4096;

  CLI::App app;

  int id;
  int partition;
  uint64_t addr;
  uint64_t size;
  const char *mtype;
  bool combine_fragments = true;
  IDType id_type = IDType::PID;

  app.add_option("id", id, "ID to target based on id-type")->required()->check(CLI::PositiveNumber);
  app.add_option("va", addr, "Virtual Address to translate")->required()->check(CLI::Number);
  app.add_option("-i,--id-type", id_type, "Select ID Type from {pid, vmid}")
      ->transform(CLI::CheckedTransformer(map))
      ->option_text("[pid]");
  app.add_option("-p,--partition", partition, "GPU partition to target")
      ->check(CLI::Number)
      ->default_val(0);
  app.add_option("-s,--size", size, "Address range size in bytes to translate")
      ->check(CLI::PositiveNumber)
      ->default_val(default_size);
  app.add_flag("!-n,!--no-combine", combine_fragments,
               "Don't combine fragments -- only return a single translation. Note: only works with "
               "size=4096");
  CLI11_PARSE(app, argc, argv);

  sift_status_t rc = sift_init();
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to initialize sift\n");
    return -1;
  }
  int num_parts;
  rc = sift_get_partition_count(&num_parts);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition count\n");
    return -1;
  }
  sift_partition_t part;
  rc = sift_get_partition(partition, &part);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get partition %d\n", partition);
    return -1;
  }

  sift_kfd_node_t node;
  rc = sift_partition_get_kfd_node(part, &node);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get node from partition id\n");
    return -1;
  }

  sift_kfd_node_properties_t props;
  rc = sift_kfd_node_get_properties(node, &props);
  if (rc != SIFT_STATUS_SUCCESS) {
    fprintf(stderr, "failed to get properties from node id\n");
    return -1;
  }

  sift_chipid_t chip_id = (sift_chipid_t)props.device_id;

  xlator_t xlate;

  switch (id_type) {
    case IDType::PID:
      if (sift_xlator_create_for_process(&xlate, id, part)) {
        fprintf(stderr, "failed to create xlator, make sure you are running as root\n");
        return -1;
      }
      break;
    case IDType::VMID:
      if (sift_xlator_create(&xlate, part, node, id)) {
        fprintf(stderr, "failed to create xlator, make sure you are running as root\n");
        return -1;
      }
      break;
  }

  fragment_list_t *list = NULL;
  if (sift_xlator_translate_range(&xlate, &list, addr, size, combine_fragments)) {
    fprintf(stderr, "failed to translate range\n");
    sift_xlator_destroy(&xlate);
    return -1;
  }

  fragment_list_t *current = list;
  while (current) {
    rc = decode_mtype(current->fragment.flags.mtype, &mtype, chip_id);
    if (rc == SIFT_STATUS_SUCCESS) {
      printf(
          "V:%d S:%d C:%d T:%d X:%d R:%d W:%d MType:%s(%d) VA 0x%016lx -> PA 0x%016lx Size:%ld\n",
          current->fragment.flags.valid, current->fragment.flags.system,
          current->fragment.flags.coherent, current->fragment.flags.tmz,
          current->fragment.flags.execute, current->fragment.flags.read,
          current->fragment.flags.write, mtype, current->fragment.flags.mtype, current->fragment.va,
          current->fragment.pa, current->fragment.size);
    } else {
      printf("V:%d S:%d C:%d T:%d X:%d R:%d W:%d MType:%d VA 0x%016lx -> PA 0x%016lx Size:%ld\n",
             current->fragment.flags.valid, current->fragment.flags.system,
             current->fragment.flags.coherent, current->fragment.flags.tmz,
             current->fragment.flags.execute, current->fragment.flags.read,
             current->fragment.flags.write, current->fragment.flags.mtype, current->fragment.va,
             current->fragment.pa, current->fragment.size);
    }
    current = current->next;
  }

  int r = 0;
  if (sift_xlator_fragment_list_destroy(list)) {
    fprintf(stderr, "Failed to destroy fragment list\n");
    r = -1;
  }

  if (sift_xlator_destroy(&xlate)) {
    fprintf(stderr, "failed to destroy xlator\n");
    r = -1;
  }
  return r;
}
