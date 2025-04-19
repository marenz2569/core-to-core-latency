#pragma once

#include "core-to-core-latency/TestPair.hpp"

#include <cstdint>
#include <set>
#include <vector>

namespace cclat {

/// Create the list of tests from the cpu binding
struct TestList {
  std::vector<TestPair> List;

  /// Create the list of tests from the to be tested cpus.
  /// \arg Cpus The cpus that should be tested
  /// \returns the list of cpu combination that need to be tested
  static auto fromCpus(const std::set<uint64_t>& Cpus) -> TestList;
};

} // namespace cclat