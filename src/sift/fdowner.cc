/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "fdowner.h"
#include "logging.h"
#include "statuserror.h"

namespace sift
{

FDOwner::FDOwner() : fd_(-1) {}

FDOwner::FDOwner(int fd) : fd_(fd)
{
  if (fd_ < 0) {
    Logger()->error("Passed invalid fd to FDOwner::FDOwner(int fd): {}", fd_);
    throw StatusError(SIFT_STATUS_ERROR);
  }
  Logger()->debug("FDOwner took ownership of fd {}", fd);
}

FDOwner::~FDOwner()
{
  if (fd_ >= 0)
    if (close(fd_) == -1) {
      Logger()->warn("Failed to close fd {}: {}", fd_, strerror(errno));
    }
}

FDOwner::FDOwner(FDOwner &&other)
{
  fd_ = other.fd_;
  other.fd_ = -1;
}

FDOwner &FDOwner::operator=(FDOwner &&other)
{
  fd_ = other.fd_;
  other.fd_ = -1;
  return *this;
}

int FDOwner::FD() { return fd_; }

}  // namespace sift
