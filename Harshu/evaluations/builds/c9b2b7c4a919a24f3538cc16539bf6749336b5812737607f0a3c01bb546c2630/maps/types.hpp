#pragma once
#include <array>
#include <cstdint>
#include <span>
namespace abyss::atlas {
struct Start { int offset,length,heading,team; };
struct Map {
 const char* name; int w,h; double pull; int hold; double crowd;
 std::span<const std::uint16_t> edges,lo,hi,body;
 std::span<const Start> starts;
 std::span<const std::int16_t> routes;
 std::span<const std::uint16_t> supply;
};
inline constexpr int revision=143;
}
