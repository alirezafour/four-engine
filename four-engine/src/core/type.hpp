#pragma once

#include <expected>
#include <string>
namespace four
{

enum class ResultCode : std::uint8_t
{
  Ok   = 0,
  Fail = 1
};

bool operator==(const ResultCode& resultCode, bool result)
{
  return result == (resultCode == ResultCode::Ok);
}

struct ResultStruct
{
  ResultCode  code{ResultCode::Ok};
  std::string message;

  constexpr operator bool() const
  {
    return code == ResultCode::Ok;
  }

  constexpr bool operator==(const ResultCode& resultCode) const
  {
    return code == resultCode;
  }
};

template <typename T>
using Result = std::expected<T, ResultStruct>;
} // namespace four
