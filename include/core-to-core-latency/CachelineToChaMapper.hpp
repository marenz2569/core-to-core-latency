#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace cclat {

/// The map of a cha box id to a vector of cache lines that are part of the L3 slice of this cha box.
using ChaToCachelinesMap = std::map<uint64_t, std::vector<void*>>;

/// This class maps cache lines to a specific CHA box via the UNC_CHA_REQUESTS.READS_* counter.
class CachelineToChaMapper {
public:
  CachelineToChaMapper() = default;

  /// Determine the mapping of cha boxes to cache lines for an array of cache lines.
  /// \arg Cachelines The vector of cachline pointers
  /// \arg NumberOfCachelineReads The number of times each cachline is read during the benchmark.
  /// \arg SocketIndex The socket that is used for this benchmark
  /// \returns the map of cha boxes to cache lines
  [[nodiscard]] static auto run(const std::vector<void*>& Cachelines, std::size_t NumberOfCachelineReads,
                                uint64_t SocketIndex) -> ChaToCachelinesMap;
};

} // namespace cclat