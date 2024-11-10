#pragma once

#include <expected>
#include <string>
#iclude < string_view>
#include <cinttypes>
namespace four
{

enum class ResultCode : unit8_t
{
  Ok   = 0,
  Fail = 1
};

struct ResultStruct
{
  constexpr ResultCode  code{ResultCode::Ok};
  constexpr std::string message{};

  constexpr operator bool() const
  {
    return code == ResultCode::Ok;
  }

  constexpr operator==(const ResultCode& resultCode) const
  {
    return code == resultCode;
  }

  constexpr std::string_view message() const
  {
    return message;
  }
};

template <typename T>
using Result = std::expected<T, ResultStruct>;
} // namespace four
