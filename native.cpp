#include <array>
#include <iostream>
#include <fstream>

#include "emp/data/DataFile.hpp"

#include "ConfigSetup.h"
#include "Org.h"
#include "SpeciesD.h"
#include "SpeciesC.h"
#include "World.h"

/**
  * @brief Add both species to occupy 25% each of available habitat
  * 
  * Based on the paper's specification that "each species occupies 0.25 
  * of the remaining available habitat" at initialization.
  */
  void PopulateWithBothSpecies(OrgWorld &world, double initial_occupancy,
     emp::Random &random_generator) {
      // Clear existing organisms
      for (size_t i = 0; i < world.GetSize(); i++) {
         if (world.IsOccupied(i)) {
             world.RemoveOrganism(i);
         }
      }
      // Count available cells (non-destroyed habitat)
      std::vector<size_t> available_cells;
      for (size_t i = 0; i < world.GetSize(); i++) {
        if (world.IsAvailable(i)) {
            available_cells.push_back(i);
          }
   }     
    // Calculate how many cells each species should occupy
    // Each species gets 25% of available habitat
    int cells_per_species = static_cast<int>(available_cells.size() * 0.25);
     // Manually shuffle the available cells for random distribution
     for (size_t i = available_cells.size() - 1; i > 0; i--) {
          size_t j = random_generator.GetUInt(i + 1);
          std::swap(available_cells[i], available_cells[j]);
    }
   // Add species C to first 25% of shuffled available cells
     for (int i = 0; i < cells_per_species && i < available_cells.size(); i++) {
            SpeciesC* new_organism = new SpeciesC(&random_generator);
            world.AddOrgAt(new_organism, available_cells[i]);
      }
            
   // Add species D to next 25% of shuffled available cells
       for (int i = cells_per_species; 
        i < cells_per_species * 2 && i < available_cells.size(); i++) {
        SpeciesD* new_organism = new SpeciesD(&random_generator);
        world.AddOrgAt(new_organism, available_cells[i]);
    }
  }
/**
 * @brief Helper function that adds species D to random available positions
 */
void PopulateWithSpeciesD(OrgWorld &world, double initial_occupancy,
                          emp::Random &random_generator) {
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
  int target_organisms =
      static_cast<int>(available_cells.size() * initial_occupancy);

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

  // Create CSV file with unique name
  std::string filename = "experiment_results.csv";
  int file_number = 1;
  std::ifstream test_file(filename);
  while (test_file.good()) {
    test_file.close();
    filename = "experiment_results_" + std::to_string(file_number) + ".csv";
    test_file.open(filename);
    file_number++;
  }
  test_file.close();
  
  std::ofstream outputfile(filename);
  outputfile << "Destruction,Species_C,Species_D,Empty,Destroyed\n";
  // Loop through different destruction percentages
  for (double destruction = 0.25; destruction < 0.76; destruction += 0.01) {
    // Initialize the world grid
    world.InitializeGrid(50, 50);

    // Set destruction
    if (config.DESTRUCTION_ROUNDS() > 0) {
      // Initialize incremental destruction
      world.InitializeIncrementalDestruction(destruction, config.DESTRUCTION_ROUNDS(), config.DESTRUCTION_PATTERN());
      
      // Populate with species before destruction starts
      PopulateWithBothSpecies(world, initial_occupancy, random);
      
      // Process destruction and ecology updates together
      for (int update = 0; update < 1000; update++) {
        // Process incremental destruction if active
        if (world.IsIncrementalDestructionActive()) {
          world.ProcessIncrementalDestruction();
        }
        world.UpdateEcology();
      }
    } else {
      // Original immediate destruction
      if (config.DESTRUCTION_PATTERN() == 0) {
        world.DestroyHabitatRandom(destruction);
      } else {
        world.DestroyHabitatGradient(destruction);
      }
      
      PopulateWithBothSpecies(world, initial_occupancy, random);
      
      // Run simulation for 1000 updates
      for (int update = 0; update < 1000; update++) {
        world.UpdateEcology();
      }
    }
    
    // create a array that gets count using CountCells from world and print
    // coutcells
    emp::array<int, 4> counts = world.CountCells(); // doesn't like this line for some reason but still works.
    
    std::cout << "Destruction: " << destruction << ", Species C: " << counts[0]
              << ", Species D: " << counts[1] << ", Empty: " << counts[2]
              << ", Destroyed: " << counts[3] << std::endl;
    
    // Write same data to CSV
    outputfile << destruction << "," << counts[0] << "," << counts[1] << "," 
               << counts[2] << "," << counts[3] << "\n";
  }
  
  outputfile.close();
  std::cout << "Results saved to " << filename << std::endl;

  return 0;
}