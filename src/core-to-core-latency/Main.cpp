#include "core-to-core-latency/CachelineToChaMapper.hpp"
#include "core-to-core-latency/ChaCoreMapperResults.hpp"
#include "core-to-core-latency/ChaToCoreMapper.hpp"
#include "core-to-core-latency/Config.hpp"
#include "core-to-core-latency/CoreTrafficTest.hpp"
#include "core-to-core-latency/TestList.hpp"
#include "firestarter/CPUTopology.hpp"

#include <cstddef>
#include <firestarter/AlignedAlloc.hpp>
#include <iostream>

auto main(int Argc, const char** Argv) -> int {
  std::cout << "core-to-core-latency. Microarchitectual Benchmark, Version " << _CORE_TO_CORE_LATENCY_VERSION_STRING
            << "\n"
            << "Copyright (C) " << _CORE_TO_CORE_LATENCY_BUILD_YEAR << " Markus Schmidl" << "\n";

  try {
    cclat::Config Cfg{Argc, Argv};

    const auto Tests = cclat::TestList::fromCpus(Cfg.CpuBinding);

    // Bind to the first core of the cpubind list.
    firestarter::CPUTopology Topology;
    Topology.bindCallerToOsIndex(*Cfg.CpuBinding.begin());

    // Create a 4KiB page aligned memory allocation
    auto* Memory =
        firestarter::AlignedAlloc::malloc(static_cast<std::size_t>(64) * Cfg.NumberOfCachelines, /*Alignment=*/4096);

    auto ChaToCachelines = cclat::CachelineToChaMapper::run(Memory, Cfg.NumberOfCachelines,
                                                            /*NumberOfCachelineReads=*/1000, Cfg.SocketIndex);

    for (const auto& [Cha, Values] : ChaToCachelines) {
      std::cout << "Cha index: " << Cha << " contains " << Values.size() << " values" << '\n';
    }

    auto ChaToCore =
        cclat::ChaToCoreMapper::run(ChaToCachelines, /*NumberOfCachelineReads=*/1000, Cfg.CpuBinding, Cfg.SocketIndex);

    for (const auto& [Cha, Core] : ChaToCore) {
      std::cout << "CHA index: " << Cha << " Core index: " << Core << '\n';
    }

    auto CoreToChaBusyPath = cclat::CoreTrafficTest::run(
        ChaToCachelines, ChaToCore, /*NumberOfCachelineReads=*/10000000,
        /*ClusteringThreshold=*/1 / 100.0, /*DetectionThreshold=*/1 / 40.0, Cfg.SocketIndex);

    auto Results = cclat::ChaCoreMapperResults{.ChaToCore = ChaToCore, .ChasWithIngressPathsVector = CoreToChaBusyPath};

    std::ofstream OutfileStream(Cfg.OutfilePath);
    nlohmann::json JsonOutput;
    nlohmann::adl_serializer<cclat::ChaCoreMapperResults>::to_json(JsonOutput, Results);
    OutfileStream << JsonOutput << '\n';
  } catch (std::exception const& E) {
    std::cerr << E.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
