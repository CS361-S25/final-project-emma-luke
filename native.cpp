#include <iostream>

#include "emp/data/DataFile.hpp"

#include "ConfigSetup.h"
#include "Org.h"
#include "World.h"

// create a for loop that runs from 0.25 to 0.75, in increments of 0.01
// within the for loop...
// create a new world with a new destruction percentage
// run for 1000 updates
// count cells at the end of the updates
// append the results to a csv

// for replication purposes we can have native-baseline, native-basic, and native-ideal
// for each of the tiers of results so we can easily go back and replicate each step

int main(int argc, char *argv[]) {
  MyConfigType config;
  config.Read("MySettings.cfg");
  bool success = config.Read("MySettings.cfg");
  if(!success) config.Write("MySettings.cfg");

  emp::Random random(config.SEED());
  OrgWorld world(random);
  world.DestroyHabitatRandom(config.PERCENT_DESTROYED());
  
  std::vector<size_t> available_cells;
    for (size_t i = 0; i < world.GetSize(); i++) {
        if (world.IsAvailable(i)) {
            available_cells.push_back(i);
        }
    }
  world.Resize(50,50);

  for (int update = 0; update <1000; update++) {
    world.Update();
  }
  // std::cout << "Number of organisms: " << world.CountCells() << std::endl;

}
