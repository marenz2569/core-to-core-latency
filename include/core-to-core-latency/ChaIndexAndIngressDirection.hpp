#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <sstream>

namespace cclat {

/// The pair of CHA index and ingress direction
struct ChaIndexAndIngressDirection {
  uint64_t ChaIndex;
  uint64_t IngressDirection;

  auto operator==(const cclat::ChaIndexAndIngressDirection& Other) const -> bool {
    return std::tie(ChaIndex, IngressDirection) == std::tie(Other.ChaIndex, Other.IngressDirection);
  }
};

static auto dump(std::ostream& Stream, const ChaIndexAndIngressDirection& Cid) -> std::ostream& {
  Stream << "ChaIndex: " << Cid.ChaIndex << " IngressDirection: " << Cid.IngressDirection << "\n";
  return Stream;
}

} // namespace cclat

template <> struct std::hash<cclat::ChaIndexAndIngressDirection> {
  auto operator()(cclat::ChaIndexAndIngressDirection const& Cid) const noexcept -> std::size_t {
    std::stringstream Ss;
    std::stringstream Outstream;
    Outstream << dump(Ss, Cid).rdbuf();
    return std::hash<std::string>{}(Outstream.str());
  }
};

namespace nlohmann {

template <> struct adl_serializer<cclat::ChaIndexAndIngressDirection> {
  // NOLINTNEXTLINE(readability-identifier-naming)
  static void to_json(json& j, const cclat::ChaIndexAndIngressDirection& Data) {
    j = json::object();
    j["node_id"] = Data.ChaIndex;
    j["channel"] = Data.IngressDirection;
  }
};

} // namespace nlohmann