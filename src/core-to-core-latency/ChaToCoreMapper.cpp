#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "firestarter/CPUTopology.hpp"

#include <cpucounters.h>
#include <cstddef>
#include <cstdint>

namespace cclat {

auto ChaToCoreMapper::run(const ChaToCachelinesMap& ChaToCachelines, const std::size_t NumberOfCachelineReads,
                          const std::set<uint64_t>& Cpus) -> CoreToChaMap {
  CoreToChaMap CoreToCha;
  firestarter::CPUTopology Topology;

  // for each core access cache lines of all cha boxes and find the cache lines with the lowest read latency.
  for (const auto& Cpu : Cpus) {

    // Use the first or second cpu of the list.
    auto ReadingCpu = *Cpus.begin();
    if (ReadingCpu == Cpu) {
      ReadingCpu = *(Cpus.begin()++);
    }

    // Read Cache lines into L3 of other than measureing core.
    Topology.bindCallerToOsIndex(ReadingCpu);

    for (const auto& [Cha, Cachelines] : ChaToCachelines) {

      volatile uint8_t Sum = 0;
      for (auto I = 0; I < NumberOfCachelineReads; I++) {
        auto* Cacheline = static_cast<uint8_t*>(Cachelines[I]);

        // flush cacheline
        asm __volatile__("mfence\n"
                         "lfence\n"
                         "clflush (%[addr])\n"
                         "mfence\n"
                         "lfence" ::[addr] "r"(Cacheline)
                         : "memory");

        // read cache line into cache of other core (this includes the shared l3 cache)
        Sum = Sum + *Cacheline;
      }
      (void)Sum;
    }

    // perform read access time measurement from the current cpu.
    Topology.bindCallerToOsIndex(Cpu);
    for (const auto& [Cha, Cachelines] : ChaToCachelines) {
      uint64_t TotalChaAccessTime = 0;

      volatile uint8_t Sum = 0;
      for (auto I = 0; I < NumberOfCachelineReads; I++) {
        uint64_t RaxStart{};
        uint64_t RdxStart{};
        uint64_t RaxStop{};
        uint64_t RdxStop{};

        auto* Cacheline = static_cast<uint8_t*>(Cachelines[I]);

        // record read access time
        __asm__ __volatile__("rdtscp" : "=a"(RaxStart), "=d"(RdxStart)::);
        Sum = Sum + *Cacheline;
        __asm__ __volatile__("rdtscp" : "=a"(RaxStop), "=d"(RdxStop)::);

        auto Start = (RdxStart << 32) + RaxStart;
        auto Stop = (RdxStop << 32) + RaxStop;

        TotalChaAccessTime += Stop - Start;
      }
      (void)Sum;

      std::cout << "Core " << Cpu << " Cha " << Cha << " access time = " << TotalChaAccessTime << "\n";
    }

    // TODO: select the correct cha based on core.
  }

  return CoreToCha;
}

} // namespace cclat
