#ifndef __RE3_LIB_AUTOMATON_H__
#define __RE3_LIB_AUTOMATON_H__

#include <string_view>

namespace re3 {

class AutomatonInterface {
 public:
  virtual ~AutomatonInterface() = default;
  virtual bool Run(std::string_view input) const = 0;
};

}  // namespace re3

#endif  // __RE3_LIB_AUTOMATON_H__
