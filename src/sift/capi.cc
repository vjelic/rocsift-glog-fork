/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <atomic>
#include <cstring>
#include <memory>

#include "rocsift/sift.h"
#include "engine.h"
#include "logging.h"
#include "statuserror.h"

std::atomic<int> engref = 0;
sift::Engine *eng;

sift::Engine *GetEngine() { return eng; }

sift_status_t sift_init()
{
  try {
    auto val = engref++;
    if (!val) {
      if (eng) return SIFT_STATUS_CODE_BUG;
      eng = new sift::Engine();
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_destroy()
{
  try {
    int alive = engref--;
    if (!alive) {
      delete eng;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_get_device_count(int *n)
{
  try {
    *n = eng->Devices().size();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_get_device(int n, sift_device_t *dev)
{
  try {
    if (n >= int(eng->Devices().size())) {
      return SIFT_STATUS_OUT_OF_RANGE;
    }
    dev->handle = n;
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_device_get_partition_count(sift_device_t dev, int *n)
{
  try {
    auto &h = eng->Devices()[dev.handle];
    *n = h->Partitions().size();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_device_get_partition(sift_device_t dev, sift_partition_t *part, int n)
{
  try {
    auto &h = eng->Devices()[dev.handle];
    if (n >= int(h->Partitions().size())) {
      return SIFT_STATUS_ERROR;
    }
    part->handle = h->Partitions()[n]->GlobalID();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_get_partition_count(int *n)
{
  try {
    *n = eng->Partitions().size();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
};

sift_status_t sift_get_partition(int n, sift_partition_t *part)
{
  try {
    if (n >= int(eng->Partitions().size())) {
      return SIFT_STATUS_OUT_OF_RANGE;
    }
    part->handle = n;
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_partition_get_device(sift_partition_t part, sift_device_t *dev)
{
  try {
    auto p = eng->Partitions()[part.handle];
    dev->handle = p->GetDevice().Instance();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_partition_get_kfd_node(sift_partition_t part, sift_kfd_node_t *node)
{
  try {
    auto p = eng->Partitions()[part.handle];
    auto &nodes = eng->KFD().Nodes();

    // TODO: Replace with an O(1) lookup
    for (size_t i = 0; i < nodes.size(); i++) {
      if (&nodes.at(i) == &p->KFD()) {
        node->handle = i;
        return SIFT_STATUS_SUCCESS;
      }
    }
    return SIFT_STATUS_OUT_OF_RANGE;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_device_read_reg32(sift_device_t dev, uint32_t *value, sift_aperture_t ap,
                                     uint64_t reg)
{
  try {
    auto &d = eng->Devices()[dev.handle];
    switch (ap) {
      case SIFT_APER_MMIO:
        *value = d->MMIOReadReg32(reg);
        break;
      case SIFT_APER_SMN:
        *value = d->SMNReadReg32(reg);
        break;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_device_write_reg32(sift_device_t dev, uint32_t value, sift_aperture_t ap,
                                      uint64_t reg)
{
  try {
    auto &d = eng->Devices()[dev.handle];
    switch (ap) {
      case SIFT_APER_MMIO:
        d->MMIOWriteReg32(reg, value);
        break;
      case SIFT_APER_SMN:
        d->SMNWriteReg32(reg, value);
        break;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_partition_read_reg32(sift_partition_t part, uint32_t *value, sift_aperture_t ap,
                                        uint64_t reg)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    switch (ap) {
      case SIFT_APER_MMIO:
        *value = p->MMIOReadReg32(reg);
        break;
      case SIFT_APER_SMN:
        *value = p->SMNReadReg32(reg);
        break;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_partition_write_reg32(sift_partition_t part, uint32_t value, sift_aperture_t ap,
                                         uint64_t reg)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    switch (ap) {
      case SIFT_APER_MMIO:
        p->MMIOWriteReg32(reg, value);
        break;
      case SIFT_APER_SMN:
        p->SMNWriteReg32(reg, value);
        break;
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

ssize_t sift_device_read(sift_device_t dev, void *data, sift_mem_region_t region, uint64_t addr,
                         size_t size)
{
  try {
    auto &d = eng->Devices()[dev.handle];
    switch (region) {
      case SIFT_LINEAR_VIDEO_MEM:
        return d->ReadVRAM(addr, data, size);
      case SIFT_LINEAR_SYSTEM_MEM:
        return d->ReadSystemRAM(addr, data, size);
      default:
        return -1;
    }
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return -1;
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return -1;
  }
}

ssize_t sift_device_write(sift_device_t dev, const void *data, sift_mem_region_t region,
                          uint64_t addr, size_t size)
{
  try {
    auto &d = eng->Devices()[dev.handle];
    switch (region) {
      case SIFT_LINEAR_VIDEO_MEM:
        return d->WriteVRAM(addr, data, size);
      case SIFT_LINEAR_SYSTEM_MEM:
        return d->WriteSystemRAM(addr, data, size);
      default:
        return -1;
    }
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return -1;
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return -1;
  }
}

ssize_t sift_partition_read(sift_partition_t part, void *data, sift_mem_region_t region,
                            uint64_t addr, size_t size)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    switch (region) {
      case SIFT_LINEAR_VIDEO_MEM:
        return p->ReadVRAM(addr, data, size);
      case SIFT_LINEAR_SYSTEM_MEM:
        return p->ReadSystemRAM(addr, data, size);
      default:
        return -1;
    }
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return -1;
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return -1;
  }
}

ssize_t sift_partition_write(sift_partition_t part, const void *data, sift_mem_region_t region,
                             uint64_t addr, size_t size)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    switch (region) {
      case SIFT_LINEAR_VIDEO_MEM:
        return p->WriteVRAM(addr, data, size);
      case SIFT_LINEAR_SYSTEM_MEM:
        return p->WriteSystemRAM(addr, data, size);
      default:
        return -1;
    }
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return -1;
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return -1;
  }
}

sift_status_t sift_partition_get_xcc_count(sift_partition_t part, int *num_xcc)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    *num_xcc = p->XCCDieIDs().size();
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

sift_status_t sift_partition_get_xcc_die_ids(sift_partition_t part, int *xcc_ids, int num_xcc_ids)
{
  try {
    auto &p = eng->Partitions()[part.handle];
    auto &ids = p->XCCDieIDs();
    if (num_xcc_ids > int(ids.size())) {
      return SIFT_STATUS_ERROR;
    }
    for (auto i = 0; i < num_xcc_ids; i++) {
      xcc_ids[i] = ids.at(i);
    }
    return SIFT_STATUS_SUCCESS;
  } catch (sift::StatusError &e) {
    sift::Logger()->error(e.what());
    return e.Status();
  } catch (std::exception &e) {
    sift::Logger()->error(e.what());
    return SIFT_STATUS_ERROR;
  }
}

const char *sift_status_get_message(sift_status_t status) { return sift::status_message(status); }
