#pragma once

#include <cstdint>
#include <functional>

namespace cclat {

/// Pair of cpus used in the test
struct TestPair {
  /// The cpu that first writes to memory of the remote cpu.
  uint64_t LocalCpu{};
  /// The cpu that responds with a memory write to the local cpu.
  uint64_t RemoteCpu{};

  constexpr auto operator==(const TestPair& Other) const -> bool {
    return std::tie(LocalCpu, RemoteCpu) == std::tie(Other.LocalCpu, Other.RemoteCpu);
  }

  auto operator<(const TestPair& Other) const -> bool {
    return std::tie(LocalCpu, RemoteCpu) < std::tie(Other.LocalCpu, Other.RemoteCpu);
  }
};
} // namespace cclat

namespace std {
template <> class hash<cclat::TestPair> {
public:
  auto operator()(const cclat::TestPair& Pair) const -> std::size_t {
    return hash<uint64_t>{}(Pair.LocalCpu) ^ (hash<uint64_t>{}(Pair.RemoteCpu) << 1);
  }
};
} // namespace std