#include <iostream>
#include <array>


#include "emp/data/DataFile.hpp"

#include "ConfigSetup.h"
#include "Org.h"
#include "World.h"
#include "SpeciesD.h"

/**
    * @brief Add species D to random available positions
         */
        void PopulateWithSpeciesD(OrgWorld& world, double initial_occupancy, emp::Random& random_generator) {
            // Clear existing organisms
            for (size_t i = 0; i < world.GetSize(); i++) {
                if (world.IsOccupied(i)) {
                    world.RemoveOrganism(i);
                }
            }

            // Count available cells
            std::vector<size_t> available_cells;
            for (size_t i = 0; i < world.GetSize(); i++) {
                if (world.IsAvailable(i)) {
                    available_cells.push_back(i);
                }
            }
            
            // Populate initial_occupancy fraction of available cells
            int target_organisms = static_cast<int>(available_cells.size() * initial_occupancy);
            
            // Manually shuffle the available cells
            for (size_t i = available_cells.size() - 1; i > 0; i--) {
                size_t j = random_generator.GetUInt(i + 1);
                std::swap(available_cells[i], available_cells[j]);
            }
            
            for (int i = 0; i < target_organisms && i < available_cells.size(); i++) {
                emp::Ptr<SpeciesD> new_organism = new SpeciesD(&random_generator);
                world.AddOrgAt(new_organism, available_cells[i]);
     }
  }
// create a for loop that runs from 0.25 to 0.75, in increments of 0.01
// within the for loop...
// create a new world with a new destruction percentage
// run for 1000 updates
// count cells at the end of the updates
// append the results to a csv

// for replication purposes we can have native-baseline, native-basic, and
// native-ideal for each of the tiers of results so we can easily go back and
// replicate each step


int main(int argc, char *argv[]) {
  MyConfigType config;
  config.Read("MySettings.cfg");
  bool success = config.Read("MySettings.cfg");
  if (!success)
    config.Write("MySettings.cfg");


  // Create data file for recording results
  emp::Random random(config.SEED());
  OrgWorld world(random);
  double initial_occupancy = 0.5;
  // Loop through different destruction percentages
  for (double destruction = 0.25; destruction <= 0.75; destruction += 0.01) {
    // Initialize the world grid
    world.InitializeGrid(50, 50);

    // Set destruction percentage
    world.DestroyHabitatRandom(destruction);
    PopulateWithSpeciesD(world, initial_occupancy, random);
    
    world.SetupResultsFile("results.csv");
    
    // Run simulation for 1000 updates
    for (int update = 0; update < 1000; update++) {
      world.UpdateEcology();
    }
    //create a array that gets count using CountCells from world and print coutcells
    emp::array<int, 4> counts = world.CountCells();
    std::cout << "Destruction: " << destruction << ", Species C: " << counts[0]
              << ", Species D: " << counts[1] << ", Empty: " << counts[2]
              << ", Destroyed: " << counts[3] << std::endl;
  
  }
return 0;
}
