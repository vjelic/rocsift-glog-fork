/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _ROCSIFT_STATUSERROR_H_
#define _ROCSIFT_STATUSERROR_H_
#include <exception>

#include "rocsift/status.h"

namespace sift
{

inline const char* status_message(sift_status_t status) noexcept
{
  switch (status) {
    case SIFT_STATUS_SUCCESS:
      return "SUCCESS";
    case SIFT_STATUS_ERROR:
      return "rocsift encountered a generic fatal error";
    case SIFT_STATUS_CODE_BUG:
      return "rocsift encountered a code bug -- please reach out to rocsift maintainers";
    case SIFT_STATUS_NOT_PRIVILEGED:
      return "rocsift encountered a permissions error -- please elevate permissions as necessary";
    default:
      return "rocsift encountered an unkonwn error -- please reach out to rocsift maintainers";
  }
}

class StatusError : public std::exception
{
 public:
  StatusError(sift_status_t status) : status_(status) {}
  const char* what() const noexcept override { return status_message(status_); }
  sift_status_t Status() const noexcept { return status_; }

 private:
  sift_status_t status_;
};

}  // namespace sift

#endif  // _ROCSIFT_STATUSERROR_H_
