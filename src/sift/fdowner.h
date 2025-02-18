/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_FDOWNER_H_
#define _SIFT_FDOWNER_H_

namespace sift
{

class FDOwner
{
 private:
  int fd_;

 public:
  FDOwner();
  FDOwner(int fd);
  ~FDOwner();

  // copy constructor
  FDOwner(const FDOwner &other) = delete;

  // move constructor
  FDOwner(FDOwner &&other);

  // move assignment operator
  FDOwner &operator=(FDOwner &&other);

  int FD();
};

}  // namespace sift

#endif  // _SIFT_FDOWNER_H_
