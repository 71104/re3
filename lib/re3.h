#ifndef __RE3_LIB_RE3_H__
#define __RE3_LIB_RE3_H__

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/statusor.h"

namespace re3 {

struct Flags {
  bool full_match = false;
  bool case_sensitive = true;
};

class RE {
 public:
  static absl::StatusOr<std::unique_ptr<RE>> Create(std::string_view pattern,
                                                    Flags const& flags = {});

  absl::StatusOr<std::vector<std::string>> Match(std::string_view input);
};

absl::StatusOr<std::vector<std::string>> Match(std::string_view pattern, std::string_view input,
                                               Flags const& flags = {});

}  // namespace re3

#endif  // __RE3_LIB_RE3_H__
