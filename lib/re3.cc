#include "lib/re3.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "lib/parser.h"

namespace re3 {

absl::StatusOr<RE> RE::Create(std::string_view const pattern, Flags const& flags) {
  auto status_or_automaton = Parse(pattern);
  if (status_or_automaton.ok()) {
    return RE(std::move(status_or_automaton).value());
  } else {
    return std::move(status_or_automaton).status();
  }
}

absl::StatusOr<std::vector<std::string>> Match(std::string_view const pattern,
                                               std::string_view const input, Flags const& flags) {
  auto status_or_re = RE::Create(pattern, flags);
  if (status_or_re.ok()) {
    return std::move(status_or_re).status();
  }
  auto const& re = status_or_re.value();
  return re.Match(input);
}

}  // namespace re3
