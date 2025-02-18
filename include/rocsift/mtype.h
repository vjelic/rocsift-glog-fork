/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_MTYPE_H
#define _SIFT_MTYPE_H

#include <cstdint>
#include <cstdbool>

#include "rocsift/status.h"
#include "rocsift/chipid.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef enum {
  SIFT_MTYPE_NC,
  SIFT_MTYPE_WC,
  SIFT_MTYPE_RW,
  SIFT_MTYPE_CC,
  SIFT_MTYPE_UC,
  SIFT_MTYPE_C_RW_US,
  SIFT_MTYPE_C_RW_S,
  SIFT_MTYPE_C_RO_US,
  SIFT_MTYPE_C_RO_S,
  SIFT_MTYPE_RESERVED,
  SIFT_MTYPE_COUNT  // Helper to count the number of enum values
} sift_mtype_t;

static const char* sift_mtype_strings[] = {"NC",      "WC",     "RW",      "CC",     "UC",
                                           "C_RW_US", "C_RW_S", "C_RO_US", "C_RO_S", "RESERVED"};

/// @brief Returns decoded MTYPE string for the chip
///
/// @param chipid to evaluate
/// @param mtype to evaluate
//  @param decode for returning string decode
/// @returns SIFT_STATUS_SUCESS on proper decode
inline sift_status_t decode_mtype(uint8_t mtype, const char** decode, sift_chipid_t id)
{
  sift_mtype_t mt = SIFT_MTYPE_COUNT;

  if (is_mi100(id) || is_mi200(id) || is_mi3xx(id)) {
    switch (mtype) {
      case 0:
        mt = SIFT_MTYPE_NC;
        break;
      case 1:
        mt = SIFT_MTYPE_RW;
        break;
      case 2:
        mt = SIFT_MTYPE_CC;
        break;
      case 3:
        mt = SIFT_MTYPE_UC;
        break;
    }
  } else if (is_navi10(id) || is_navi21(id) || is_navi31(id)) {
    switch (mtype) {
      case 0:
        mt = SIFT_MTYPE_C_RW_US;
        break;
      case 1:
        mt = SIFT_MTYPE_RESERVED;
        break;
      case 2:
        mt = SIFT_MTYPE_C_RO_S;
        break;
      case 3:
        mt = SIFT_MTYPE_UC;
        break;
      case 4:
        mt = SIFT_MTYPE_C_RW_S;
        break;
      case 5:
        mt = SIFT_MTYPE_RESERVED;
        break;
      case 6:
        mt = SIFT_MTYPE_C_RO_US;
        break;
      case 7:
        mt = SIFT_MTYPE_RESERVED;
        break;
    }
  }

  if (mt != SIFT_MTYPE_COUNT && mt < SIFT_MTYPE_COUNT) {
    *decode = sift_mtype_strings[mt];
    return SIFT_STATUS_SUCCESS;
  }

  return SIFT_STATUS_OUT_OF_RANGE;
}

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _SIFT_MTYPE_H_
