#ifndef __RE3_LIB_RE3_H__
#define __RE3_LIB_RE3_H__

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "lib/automaton.h"

namespace re3 {

struct Flags {
  bool full_match = false;
  bool case_sensitive = true;
};

class RE {
 public:
  static absl::StatusOr<RE> Create(std::string_view pattern, Flags const& flags = {});

  RE(RE const& other) : automaton_(other.automaton_->Clone()) {}

  RE& operator=(RE const& other) {
    automaton_ = other.automaton_->Clone();
    return *this;
  }

  RE(RE&&) noexcept = default;
  RE& operator=(RE&&) noexcept = default;

  absl::StatusOr<std::vector<std::string>> Match(std::string_view input) const;

 private:
  explicit RE(std::unique_ptr<AutomatonInterface> automaton) : automaton_(std::move(automaton)) {}

  std::unique_ptr<AutomatonInterface> automaton_;
};

absl::StatusOr<std::vector<std::string>> Match(std::string_view pattern, std::string_view input,
                                               Flags const& flags = {});

}  // namespace re3

#endif  // __RE3_LIB_RE3_H__
