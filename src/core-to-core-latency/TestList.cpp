#include "core-to-core-latency/TestList.hpp"
#include "core-to-core-latency/TestPair.hpp"

#include <cstdlib>
#include <vector>

namespace cclat {

auto TestList::fromCpus(const std::set<uint64_t>& Cpus) -> TestList {
  std::vector<TestPair> Pairs;

  for (const auto& First : Cpus) {
    for (const auto& Second : Cpus) {
      /// Skip the self edge
      if (First == Second) {
        continue;
      }

      Pairs.emplace_back(TestPair{.LocalCpu = First, .RemoteCpu = Second});
    }
  }

  return TestList{.List = Pairs};
}

} // namespace cclat