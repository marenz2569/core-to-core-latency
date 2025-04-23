#pragma once

#include "cpucounters.h"
#include "types.h"

namespace cclat {

struct PcmRingCounters {
  // NOLINTNEXTLINE(performance-enum-size)
  enum Direction { Left = 0, Right = 1, Up = 2, Down = 3 };

  /// Program the ring AD counters on all CHAs.
  static void programmADRingCounters(pcm::PCM* Pcm) {
    std::array<pcm::uint64, 4> CboConfigMap = {0, 0, 0, 0};

    switch (Pcm->getCPUFamilyModel()) {
    // Skylake-X
    // perfmon/SKX/events/skylakex_uncore_experimental.json
    case pcm::PCM::SKX:
      // UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_ODD
      CboConfigMap[Direction::Left] = CBO_MSR_PMON_CTL_EVENT(0xa7) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_ODD
      CboConfigMap[Direction::Right] = CBO_MSR_PMON_CTL_EVENT(0xa7) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      // UNC_CHA_VERT_RING_AD_IN_USE.UP_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.UP_ODD
      CboConfigMap[Direction::Up] = CBO_MSR_PMON_CTL_EVENT(0xa6) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_VERT_RING_AD_IN_USE.DN_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.DN_ODD
      CboConfigMap[Direction::Down] = CBO_MSR_PMON_CTL_EVENT(0xa6) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      break;
    case pcm::PCM::ICX:
    case pcm::PCM::SPR:
      // UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.LEFT_ODD
      CboConfigMap[Direction::Left] = CBO_MSR_PMON_CTL_EVENT(0xb6) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_EVEN + UNC_CHA_HORZ_RING_AD_IN_USE.RIGHT_ODD
      CboConfigMap[Direction::Right] = CBO_MSR_PMON_CTL_EVENT(0xb6) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      // UNC_CHA_VERT_RING_AD_IN_USE.UP_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.UP_ODD
      CboConfigMap[Direction::Up] = CBO_MSR_PMON_CTL_EVENT(0xb0) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_VERT_RING_AD_IN_USE.DN_EVEN + UNC_CHA_VERT_RING_AD_IN_USE.DN_ODD
      CboConfigMap[Direction::Down] = CBO_MSR_PMON_CTL_EVENT(0xb0) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      break;
    default:
      throw std::runtime_error("ChaToCoreMapper not implemented for this CPUFamilyModel");
    }

    Pcm->programCboRaw(CboConfigMap.data(), 0, 0);
  }

  /// Program the ring BL counters on all CHAs.
  static void programmBLRingCounters(pcm::PCM* Pcm) {
    std::array<pcm::uint64, 4> CboConfigMap = {0, 0, 0, 0};

    switch (Pcm->getCPUFamilyModel()) {
    // Skylake-X
    // perfmon/SKX/events/skylakex_uncore_experimental.json
    case pcm::PCM::SKX:
      // UNC_CHA_HORZ_RING_BL_IN_USE.LEFT_EVEN + UNC_CHA_HORZ_RING_BL_IN_USE.LEFT_ODD
      CboConfigMap[Direction::Left] = CBO_MSR_PMON_CTL_EVENT(0xab) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_HORZ_RING_BL_IN_USE.RIGHT_EVEN + UNC_CHA_HORZ_RING_BL_IN_USE.RIGHT_ODD
      CboConfigMap[Direction::Right] = CBO_MSR_PMON_CTL_EVENT(0xab) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      // UNC_CHA_VERT_RING_BL_IN_USE.UP_EVEN + UNC_CHA_VERT_RING_BL_IN_USE.UP_ODD
      CboConfigMap[Direction::Up] = CBO_MSR_PMON_CTL_EVENT(0xaa) + CBO_MSR_PMON_CTL_UMASK((1 + 2));
      // UNC_CHA_VERT_RING_BL_IN_USE.DN_EVEN + UNC_CHA_VERT_RING_BL_IN_USE.DN_ODD
      CboConfigMap[Direction::Down] = CBO_MSR_PMON_CTL_EVENT(0xaa) + CBO_MSR_PMON_CTL_UMASK((4 + 8));
      break;
    default:
      throw std::runtime_error("ChaToCoreMapper not implemented for this CPUFamilyModel");
    }

    Pcm->programCboRaw(CboConfigMap.data(), 0, 0);
  }
};

} // namespace cclat