#include "emp/config/ArgManager.hpp"

EMP_BUILD_CONFIG(MyConfigType,
    VALUE(SEED, int, 12, "What value should the random seed be?"), 
    VALUE(NUM_ORG, int, 100, "What is the size of the population?"),
    VALUE(PERCENT_DESTROYED, float, 0.5, "What percent of habitant should be destroyed?")
  )
