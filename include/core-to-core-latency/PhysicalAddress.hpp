#pragma once

#include <cstdint>
#include <unistd.h>

/// Helper function to map from logical to physical addresses via the linux pagemap interface.
/// https://www.kernel.org/doc/html/latest/admin-guide/mm/pagemap.html
struct PhysicalAddress {
  /// One entry of the pagemap is 8 bytes long
  constexpr static uint64_t PagemapEntryLength = 8;

  /// Get the page frame number from a logical address. This requires CAP_SYS_ADMIN priviledges.
  /// \arg Address The logical address
  /// \return The page frame number.
  static auto getPageFrameNumber(void* Address) -> uint64_t;

  /// Get the physical address from a logical one.
  /// \arg Address The logical address
  /// \return The associated physical address
  static auto getPhysicalAddress(void* Address) -> uint64_t;
};