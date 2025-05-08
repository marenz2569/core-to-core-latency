#pragma once

#include "core-to-core-latency/TestList.hpp"

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

  /// The index of the socket that should be used by the experiment
  unsigned SocketIndex;

  /// The number of cache lines to be used by the experiment
  unsigned NumberOfCachelines;

  /// The path where the output should be saved.
  std::string OutfilePath;

  /// The set of cpus on which to run the test
  std::set<uint64_t> CpuBinding;

  /// The list of tests that should be executed
  TestList Tests;

  Config() = delete;

  /// Parser the config from the command line argumens.
  Config(int Argc, const char** Argv);
};

} // namespace cclat