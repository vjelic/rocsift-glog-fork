/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <atomic>

#include "rocsift/pm4.h"
#include "rocsift/kfd.h"

#include "kfdtop.h"
#include "../sift/capi.h"
#include "../sift/logging.h"

sift_status_t sift_kfd_get_node_count(int* n)
{
  try {
    *n = GetEngine()->KFD().Nodes().size();
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_get_node(int n, sift_kfd_node_t* node)
{
  try {
    if (n >= int(GetEngine()->KFD().Nodes().size())) {
      return SIFT_STATUS_ERROR;
    }
    node->handle = n;
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_node_get_properties(sift_kfd_node_t node, sift_kfd_node_properties_t* props)
{
  try {
    auto n = GetEngine()->KFD().Nodes()[node.handle];
    *props = n.Properties();
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_node_get_id(sift_kfd_node_t node, int* instance)
{
  try {
    auto n = GetEngine()->KFD().Nodes()[node.handle];
    *instance = n.Instance();
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_node_get_gpuid(sift_kfd_node_t node, int* gpu_id)
{
  try {
    auto n = GetEngine()->KFD().Nodes()[node.handle];
    *gpu_id = n.GPUID();
    return SIFT_STATUS_SUCCESS;
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_get_proc_list(sift_kfd_proc_list_t* list)
{
  try {
    auto procs = GetEngine()->KFD().Processes();
    sift_kfd_proc_t* proc_structs = static_cast<sift_kfd_proc_t*>(
        malloc(sizeof(sift_kfd_proc_t) * procs.size()));

    if (!proc_structs) {
      list->n = 0;
      list->procs = nullptr;
      return SIFT_STATUS_ERROR;
    }

    for (size_t i = 0; i < procs.size(); i++) {
      proc_structs[i].pid = procs.at(i).PID();
      proc_structs[i].pasid = procs.at(i).PASID();
    }
    list->n = procs.size();
    list->procs = proc_structs;
    return SIFT_STATUS_SUCCESS;

  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_kfd_proc_list_destroy(sift_kfd_proc_list_t* list)
{
  if (list->procs) {
    free(list->procs);
  }
  return SIFT_STATUS_SUCCESS;
}

sift_status_t sift_kfd_get_runlists(sift_pm4_runlist_series_t** series)
{
  try {
    auto& dbg = GetEngine()->KFD().DebugFS();
    auto rls = dbg.GetRunlists();
    return sift_pm4_parse_runlists(rls.c_str(), series);
  } catch (std::exception& e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}
