/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_STATUS_H_
#define _SIFT_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// @brief Status codes
typedef enum {
  SIFT_STATUS_SUCCESS = 0x0,
  SIFT_STATUS_ERROR = 0x1,
  SIFT_STATUS_CODE_BUG = 0x2,
  SIFT_STATUS_NOT_INITIALIZED = 0x3,
  SIFT_STATUS_NOT_PRIVILEGED = 0x4,
  SIFT_STATUS_OUT_OF_RANGE = 0x5,
} sift_status_t;

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // define _SIFT_STATUS_H_
