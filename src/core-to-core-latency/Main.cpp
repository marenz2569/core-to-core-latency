#include "core-to-core-latency/Config.hpp"
#include "core-to-core-latency/CoreToCoreLatencyTest.hpp"
#include "core-to-core-latency/TestList.hpp"

#include <iostream>

auto main(int Argc, const char** Argv) -> int {
  std::cout << "core-to-core-latency. Microarchitectual Benchmark, Version " << _CORE_TO_CORE_LATENCY_VERSION_STRING
            << "\n"
            << "Copyright (C) " << _CORE_TO_CORE_LATENCY_BUILD_YEAR << " Markus Schmidl" << "\n";

  try {
    cclat::Config Cfg{Argc, Argv};

    const auto Tests = cclat::TestList::fromCpus(Cfg.CpuBinding);

    cclat::CoreToCoreLatencyTest CCLat;

    auto Results = CCLat.run(Cfg);

    Results.saveCsv(Cfg.OutfilePath);
  } catch (std::exception const& E) {
    std::cerr << E.what();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}