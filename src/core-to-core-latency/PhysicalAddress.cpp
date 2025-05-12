

#include "core-to-core-latency/PhysicalAddress.hpp"
#include <fstream>
#include <unistd.h>

// This function requires CAP_SYS_ADMIN
auto PhysicalAddress::getPageFrameNumber(void* Address) -> uint64_t {
  std::ifstream Pagemap("/proc/self/pagemap", std::ios::binary);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto PagemapOffset = reinterpret_cast<uint64_t>(Address) / getpagesize() * PhysicalAddress::PagemapEntryLength;
  Pagemap.seekg(static_cast<signed>(PagemapOffset));

  uint64_t PageframeNumber{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  Pagemap.read(reinterpret_cast<char*>(&PageframeNumber), sizeof(PageframeNumber));

  // Read the page frame number in bits 0-54.
  PageframeNumber &= 0x7FFFFFFFFFFFFF;

  return PageframeNumber;
}

auto PhysicalAddress::getPhysicalAddress(void* Address) -> uint64_t {
  auto PageframeNumber = getPageFrameNumber(Address);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto PageOffset = reinterpret_cast<uint64_t>(Address) % getpagesize();

  auto PhysicalAddress = (PageframeNumber * getpagesize()) + PageOffset;

  return PhysicalAddress;
}