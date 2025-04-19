#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cclat {

struct CoreToCoreLatencyResult {
  uint64_t LocalCore;
  uint64_t RemoteCore;
  uint64_t MinCycles;
  uint64_t AverageCycles;
  uint64_t MaxCycles;
};

struct CoreToCoreLatencyResults {
  std::vector<CoreToCoreLatencyResult> Results;

  /// Save the results as a CSV file.
  /// \arg FilePath The path of the file where the CSV should be saved.
  void saveCsv(const std::string& Filepath) {
    std::ofstream Outfile(Filepath);

    if (Outfile.bad()) {
      throw std::runtime_error("I/O error while writing CSV");
    }

    Outfile << "LocalCore,RemoteCore,MinCycles,AverageCycles,MaxCycles\n";

    for (const auto& Entry : Results) {
      Outfile << Entry.LocalCore << "," << Entry.RemoteCore << "," << Entry.MinCycles << "," << Entry.AverageCycles
              << "," << Entry.MaxCycles << "\n";
    }
  }
};

} // namespace cclat