#pragma once

#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
namespace cclat {

/// The map of a core id to cha box id.
using CoreToChaMap = std::map<uint64_t, uint64_t>;

/// This class maps cache lines to a specific CHA box via the UNC_CHA_REQUESTS.READS_* counter.
class ChaToCoreMapper {
public:
  ChaToCoreMapper() = default;

  /// Determine the mapping of cores to cha boxes on one socket.
  /// \arg ChaToCachelines Cache lines associtated to cha boxes.
  /// \arg NumberOfCachelineReads The number of times each cachline is read during the benchmark.
  /// \arg Cpus The cpus that should be tested.
  /// \returns the map of core to cha boxes
  [[nodiscard]] static auto run(const ChaToCachelinesMap& ChaToCachelines, std::size_t NumberOfCachelineReads,
                                const std::set<uint64_t>& Cpus) -> CoreToChaMap;
};

} // namespace cclat