#pragma once

#include <expected>
#include <string>
namespace four
{

using b8  = std::byte;
using i8  = std::int8_t;
using u8  = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

// enum class ResultCode : u8
// {
//   Ok   = 0,
//   Fail = 1
// };
//
// bool operator==(const ResultCode& resultCode, bool result)
// {
//   return result == (resultCode == ResultCode::Ok);
// }
//
// struct ResultStruct
// {
//   ResultCode  code{ResultCode::Ok};
//   std::string message;
//
//   constexpr operator bool() const
//   {
//     return code == ResultCode::Ok;
//   }
//
//   constexpr bool operator==(const ResultCode& resultCode) const
//   {
//     return code == resultCode;
//   }
// };
//
// template <typename T>
// using ResultType = std::expected<T, ResultStruct>;
} // namespace four
