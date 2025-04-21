#include "core-to-core-latency/CachelineToChaMapper.hpp"

#include <cstddef>
#include <cstdint>

namespace cclat {

auto CachelineToChaMapper::run(void* Cachelines, std::size_t NumberOfCachelines,
                               std::size_t NumberOfCachelineReads) -> ChaToCachelinesMap {
  ChaToCachelinesMap ChaToCachelines;

  for (auto CachelineIndex = 0; CachelineIndex < NumberOfCachelines; CachelineIndex++) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto* Cacheline = static_cast<uint8_t*>(Cachelines) + static_cast<ptrdiff_t>(64 * CachelineIndex);

    // flush, read, flush and repeat. the uncore counter for CHA reads will increment if this cacheline is in the
    // counter.

    // TODO: start the counter on all CHAs

    volatile uint8_t Sum = 0;
    for (auto I = 0; I < NumberOfCachelineReads; I++) {
      Sum += *Cacheline;
      asm __volatile__("mfence\n"
                       "lfence\n"
                       "clflush [%[addr]]\n"
                       "mfence\n"
                       "lfence" ::[addr] "r"(Cacheline)
                       : "memory");
    }

    (void)Sum;

    // TODO: stop the counter on all CHAs

    // TODO: find the CHA index where approximatly NumberOfCachelineReads occured.

    auto ChaIndex = 0;

    ChaToCachelines[ChaIndex].emplace_back(Cacheline);
  }

  return ChaToCachelines;
}

} // namespace cclat