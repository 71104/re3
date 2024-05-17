#include "lib/re3.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"

namespace re3 {

absl::StatusOr<std::vector<std::string>> Match(std::string_view const pattern,
                                               std::string_view const input, Flags const& flags) {
  auto status_or_re = RE::Create(pattern, flags);
  if (status_or_re.ok()) {
    return std::move(status_or_re).status();
  }
  auto const& re = status_or_re.value();
  return re->Match(input);
}

}  // namespace re3
