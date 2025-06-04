#include "emp/config/ArgManager.hpp"

EMP_BUILD_CONFIG(MyConfigType,
    VALUE(SEED, int, 10, "What value should the random seed be?"), 
    VALUE(DESTRUCTION_PATTERN, int, 0, "Destruction pattern: 0=Random, 1=Gradient"),
    VALUE(PERCENT_DESTROYED, float, 0.5, "What percent of habitant should be destroyed?")
  )
