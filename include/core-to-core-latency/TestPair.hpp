#pragma once

#include <cstdint>

namespace cclat {

/// Pair of cpus used in the test
struct TestPair {
  /// The cpu that first writes to memory of the remote cpu.
  uint64_t LocalCpu;
  /// The cpu that responds with a memory write to the local cpu.
  uint64_t RemoteCpu;
};

} // namespace cclat