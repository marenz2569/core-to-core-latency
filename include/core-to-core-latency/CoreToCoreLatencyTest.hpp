#pragma once

#include "core-to-core-latency/Config.hpp"
#include "core-to-core-latency/CoreToCoreLatencyResults.hpp"

namespace cclat {

class CoreToCoreLatencyTest {
public:
  CoreToCoreLatencyTest() = default;

  /// Run the latency tests
  /// \arg Cgf The config for the core to core latency run
  [[nodiscard]] auto run(const cclat::Config& Cfg) -> CoreToCoreLatencyResults;
};

} // namespace cclat