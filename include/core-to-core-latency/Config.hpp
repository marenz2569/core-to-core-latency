#pragma once

#include <cstdint>
#include <set>
#include <string>

namespace cclat {

/// This struct contains the parsed config from the command line for core-to-core-latency.
struct Config {
  /// The argument vector from the command line.
  const char** Argv;
  /// The argument count from the command line.
  int Argc;

  /// The number of unrolls of the experiment loop
  unsigned UnrollCount;

  /// The number of iteration of the experiment loop
  unsigned InnerIterations;

  /// The number of iterations used to average the result
  unsigned OuterIterations;

  /// The path where the output should be saved.
  std::string OutfilePath;

  /// The set of cpus on which to run the test
  std::set<uint64_t> CpuBinding;

  Config() = delete;

  /// Parser the config from the command line argumens.
  Config(int Argc, const char** Argv);
};

} // namespace cclat