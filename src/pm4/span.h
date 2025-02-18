/* Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved */

#ifndef _SIFT_PM4_SPAN_H_
#define _SIFT_PM4_SPAN_H_

#include <cstddef>
namespace sift
{
namespace pm4
{
template <typename T>
class span
{
 private:
  const T* arr_;
  const size_t length_;

 public:
  inline constexpr span(const T* arr, size_t length) noexcept : arr_(arr), length_(length) {}
  inline constexpr span<T> subspan(size_t offset, size_t count) const
  {
    return span<T>{arr_ + offset, count};
  }
  inline constexpr size_t size() const noexcept { return length_; }
  inline constexpr bool empty() const noexcept { return length_ == 0; }

  inline constexpr T& operator[](size_t idx) { return arr_[idx]; }
  inline constexpr const T& operator[](size_t idx) const { return arr_[idx]; }
};

};  // namespace pm4
};  // namespace sift
#endif
