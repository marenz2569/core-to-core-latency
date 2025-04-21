#include "core-to-core-latency/CachelineToChaMapper.hpp"

#include <cstddef>
#include <cstdint>

namespace cclat {

auto CachelineToChaMapper::run(void* Cachelines, std::size_t NumberOfCachelines) -> ChaToCachelinesMap {
  ChaToCachelinesMap ChaToCachelines;

  for (auto CachelineIndex = 0; CachelineIndex < NumberOfCachelines; CachelineIndex++) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto* Cacheline = static_cast<uint8_t*>(Cachelines) + static_cast<ptrdiff_t>(64 * CachelineIndex);

    // read, flush and repeat. the uncore counter for CHA reads will increment if this cacheline is in the counter.

    auto ChaIndex = 0;

    ChaToCachelines[ChaIndex].emplace_back(Cacheline);
  }

  return ChaToCachelines;
}

} // namespace cclat