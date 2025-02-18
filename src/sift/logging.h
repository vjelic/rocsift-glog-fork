/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_LOGGER_H_
#define _SIFT_LOGGER_H_
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace sift
{

inline std::shared_ptr<spdlog::logger> Logger()
{
  auto logger = spdlog::get("sift");
  if (!logger) {
    logger = spdlog::default_logger();
  }
  return logger;
}

}  // namespace sift
#endif  //  _SIFT_LOGGER_H_
