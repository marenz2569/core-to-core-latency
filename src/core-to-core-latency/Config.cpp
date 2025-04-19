#include "core-to-core-latency/Config.hpp"
#include "core-to-core-latency/TestList.hpp"

#include <cstdlib>
#include <cxxopts.hpp>
#include <firestarter/Config/CpuBind.hpp>
#include <iostream>

namespace cclat {

Config::Config(int Argc, const char** Argv)
    : Argc(Argc)
    , Argv(Argv) {
  const auto* ExecutableName = *Argv;

  cxxopts::Options Parser(ExecutableName);

  // clang-format off
  Parser.add_options()
    ("unroll", "The number of unrolls of the experiment loop", cxxopts::value<unsigned>()->default_value("16"))
    ("inner-iterations", "The number of iteration of the experiment loop", cxxopts::value<unsigned>()->default_value("2048"))
    ("outer-iterations", "The number of iterations used to average the result", cxxopts::value<unsigned>()->default_value("16"))
    ("outfile", "The path where the results should be saved to.", cxxopts::value<std::string>()->default_value("outfile.csv"))
    ("b,bind", "Select certain CPUs. CPULIST format: \"x,y,z\",\n\"x-y\", \"x-y/step\", and any combination of the\nabove.",
      cxxopts::value<std::string>()->default_value(""), "CPULIST")
  ;
  // clang-format on

  try {
    auto Options = Parser.parse(Argc, Argv);

    UnrollCount = Options["unroll"].as<unsigned>();
    InnerIterations = Options["inner-iterations"].as<unsigned>();
    OuterIterations = Options["outer-iterations"].as<unsigned>();

    OutfilePath = Options["outfile"].as<std::string>();

    CpuBinding = firestarter::CpuBind::fromString(Options["bind"].as<std::string>());

    Tests = cclat::TestList::fromCpus(CpuBinding);
  } catch (std::exception& E) {
    std::cerr << Parser.help() << "\n";
    std::cerr << "\n";
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    exit(EXIT_SUCCESS);
  }
}

} // namespace cclat