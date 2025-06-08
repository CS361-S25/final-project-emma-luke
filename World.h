#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/data/DataFile.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include <array>
#include <vector>

#include "Org.h"

/**
 * @brief World class managing the habitat destruction simulation
 *
 * Manages organisms and their interactions with a grid-based world
 * where some cells are permanently destroyed (unavailable habitat).
 */
class OrgWorld : public emp::World<Organism> {
private:
  emp::Random &random;
  emp::Ptr<emp::Random> random_ptr;
  std::vector<bool>
      destroyed_cells; ///< Track which cells are destroyed habitat
  int grid_width;
  int grid_height;
  
  // New members for incremental destruction
  emp::vector<size_t> cells_to_destroy; ///< Queue of cells scheduled for destruction
  int destruction_rounds_remaining; ///< Rounds left for incremental destruction
  int cells_per_round; ///< Number of cells to destroy each round
  int extra_cells_first_rounds; ///< Extra cells to destroy in first rounds for remainder

public:
  /**
   * @brief Construct a new OrgWorld
   * @param _random Reference to random number generator
   */
  OrgWorld(emp::Random &_random)
      : emp::World<Organism>(_random), random(_random) {
    random_ptr.New(_random);
  }

  /**
   * @brief Destructor - cleanup resources
   */
  ~OrgWorld() {
  }

  /**
   * @brief Initialize the world with a grid structure
   * @param width Grid width
   * @param height Grid height
   */
  void InitializeGrid(int width, int height) {
    grid_width = width;
    grid_height = height;
    SetPopStruct_Grid(width, height);
    destroyed_cells.resize(width * height, false);
  }

  /**
   * @brief Destroy habitat cells randomly
   * @param destruction_percentage Percentage of cells to destroy (0.0 to 1.0)
   */
  void DestroyHabitatRandom(double destruction_percentage) {
    int total_cells = grid_width * grid_height;
    int cells_to_destroy =
        static_cast<int>(total_cells * destruction_percentage);

    // Reset all cells to not destroyed
    std::fill(destroyed_cells.begin(), destroyed_cells.end(), false);

    // Randomly destroy cells
    int destroyed_count = 0;
    while (destroyed_count < cells_to_destroy) {
      size_t pos = random.GetUInt(total_cells);
      if (!destroyed_cells[pos]) {
        destroyed_cells[pos] = true;
        // Remove any organism at this position
        if (IsOccupied(pos)) {
          RemoveOrganism(pos);
        }
        destroyed_count++;
      }
    }
  }

  /**
 * @brief Destroy habitat cells in a gradient pattern
 * @param destruction_percentage Average percentage of cells to destroy (0.0 to 1.0)
 * 
 * Creates a gradient where the leftmost column has the highest destruction
 * and rightmost column has the lowest. The destruction probability varies
 * linearly across columns while maintaining the overall destruction percentage.
 */
void DestroyHabitatGradient(double destruction_percentage) {
    // Reset all cells to not destroyed
    std::fill(destroyed_cells.begin(), destroyed_cells.end(), false);
    
    // Calculate the destruction range
    // When average is 0.5, we want left at 0.75 and right at 0.25
    // This gives a spread of 0.5 (0.75 - 0.25)
    double spread = 0.5;
    double max_destruction = destruction_percentage + (spread / 2.0);
    double min_destruction = destruction_percentage - (spread / 2.0);
    
    // Ensure we stay within valid bounds [0, 1]
    if (max_destruction > 1.0) {
        double excess = max_destruction - 1.0;
        max_destruction = 1.0;
        min_destruction = std::max(0.0, min_destruction - excess);
    }
    if (min_destruction < 0.0) {
        double deficit = -min_destruction;
        min_destruction = 0.0;
        max_destruction = std::min(1.0, max_destruction + deficit);
    }
    
    // Process each column from left to right
    for (int col = 0; col < grid_width; col++) {
        // Calculate destruction probability for this column
        // Linear interpolation from max (left) to min (right)
        double column_destruction_prob = max_destruction - 
            (col * (max_destruction - min_destruction) / (grid_width - 1));
        
        // Destroy cells in this column based on the probability
        for (int row = 0; row < grid_height; row++) {
            size_t pos = row * grid_width + col;
            
            if (random.P(column_destruction_prob)) {
                destroyed_cells[pos] = true;
                // Remove any organism at this position
                if (IsOccupied(pos)) {
                    RemoveOrganism(pos);
                }
          }
        }
      }
  }

  /**
   * @brief Initialize incremental habitat destruction
   * @param destruction_percentage Percentage of cells to destroy (0.0 to 1.0)
   * @param rounds Number of rounds to spread destruction over (0 = immediate)
   * @param pattern Destruction pattern: 0=Random, 1=Gradient
   */
  void InitializeIncrementalDestruction(double destruction_percentage, int rounds, int pattern) {
    if (rounds == 0) {
      // Immediate destruction using original methods
      if (pattern == 0) {
        DestroyHabitatRandom(destruction_percentage);
      } else {
        DestroyHabitatGradient(destruction_percentage);
      }
      return;
    }
    
    // Reset destruction state
    std::fill(destroyed_cells.begin(), destroyed_cells.end(), false);
    cells_to_destroy.clear();
    
    int total_cells = grid_width * grid_height;
    int total_to_destroy = static_cast<int>(total_cells * destruction_percentage);
    
    // Determine which cells to destroy based on pattern
    if (pattern == 0) {
      // Random pattern - create random order
      emp::vector<size_t> all_cells;
      for (size_t i = 0; i < total_cells; i++) {
        all_cells.push_back(i);
      }
      emp::Shuffle(random, all_cells);
      
      // Take first total_to_destroy cells
      for (int i = 0; i < total_to_destroy; i++) {
        cells_to_destroy.push_back(all_cells[i]);
      }
    } else {
      // Gradient pattern - calculate destruction probability per column
      double spread = 0.5;
      double max_destruction = destruction_percentage + (spread / 2.0);
      double min_destruction = destruction_percentage - (spread / 2.0);
      
      // Ensure bounds
      if (max_destruction > 1.0) {
        double excess = max_destruction - 1.0;
        max_destruction = 1.0;
        min_destruction = std::max(0.0, min_destruction - excess);
      }
      if (min_destruction < 0.0) {
        double deficit = -min_destruction;
        min_destruction = 0.0;
        max_destruction = std::min(1.0, max_destruction + deficit);
      }
      
      // Select cells based on gradient probabilities
      for (int col = 0; col < grid_width; col++) {
        double column_destruction_prob = max_destruction - 
            (col * (max_destruction - min_destruction) / (grid_width - 1));
        
        for (int row = 0; row < grid_height; row++) {
          size_t pos = row * grid_width + col;
          if (random.P(column_destruction_prob)) {
            cells_to_destroy.push_back(pos);
          }
        }
      }
      
      // Shuffle to randomize destruction order within the gradient pattern
      emp::Shuffle(random, cells_to_destroy);
    }
    
    // Calculate cells per round
    destruction_rounds_remaining = rounds;
    cells_per_round = cells_to_destroy.size() / rounds;
    extra_cells_first_rounds = cells_to_destroy.size() % rounds;
  }
  
  /**
   * @brief Process one round of incremental destruction
   * @return Number of cells destroyed this round
   */
  int ProcessIncrementalDestruction() {
    if (destruction_rounds_remaining <= 0 || cells_to_destroy.empty()) {
      return 0;
    }
    
    // Calculate how many cells to destroy this round
    int cells_this_round = cells_per_round;
    if (extra_cells_first_rounds > 0) {
      cells_this_round++;
      extra_cells_first_rounds--;
    }
    
    int destroyed_count = 0;
    
    // Destroy cells from the front of the queue
    while (destroyed_count < cells_this_round && !cells_to_destroy.empty()) {
      size_t pos = cells_to_destroy.front();
      cells_to_destroy.erase(cells_to_destroy.begin());
      
      // Destroy the cell
      destroyed_cells[pos] = true;
      
      // Kill any organism at this position
      if (IsOccupied(pos)) {
        RemoveOrganism(pos);
      }
      
      destroyed_count++;
    }
    
    destruction_rounds_remaining--;
    return destroyed_count;
  }
  
  /**
   * @brief Check if incremental destruction is active
   * @return True if there are still rounds of destruction remaining
   */
  bool IsIncrementalDestructionActive() const {
    return destruction_rounds_remaining > 0;
  }

  /**
   * @brief Check if a cell is destroyed habitat
   * @param pos Position to check
   * @return True if cell is destroyed
   */
  bool IsDestroyed(size_t pos) const {
    return pos < destroyed_cells.size() && destroyed_cells[pos];
  }

  /**
   * @brief Check if a cell is available (not destroyed)
   * @param pos Position to check
   * @return True if cell is available habitat
   */
  bool IsAvailable(size_t pos) const {
    return pos < destroyed_cells.size() && !destroyed_cells[pos];
  }

  /**
   * @brief Update all organisms for one simulation step
   */
  void UpdateEcology() {
    // Process each organism
    emp::vector<size_t> occupied_positions;
    for (size_t i = 0; i < GetSize(); i++) {
      if (IsOccupied(i) && !IsDestroyed(i)) {
        occupied_positions.push_back(i);
      }
    }

    // Manually shuffle for random processing order
    for (size_t i = occupied_positions.size() - 1; i > 0; i--) {
      size_t j = random.GetUInt(i + 1);
      std::swap(occupied_positions[i], occupied_positions[j]);
    }

    // Process organisms for extinction and colonization
    for (size_t pos : occupied_positions) {
      if (IsOccupied(pos)) {
        ProcessOrganism(pos);
      }
    }
  }

  /**
   * @brief Try to colonize a single neighboring cell based on colonization rate
   * @param pos Position of colonizing organism
   * @param colonization_rate Rate of colonization
   * 
   * Each organism can produce at most one offspring per round.
   * The offspring is placed in a randomly selected available neighbor.
   * Species C can colonize empty cells and displace species D.
   * Species D can only colonize empty cells.
   */
  void TryColonize(size_t pos, double colonization_rate) {
    if (!IsOccupied(pos) || IsDestroyed(pos))
      return;

    // First check if colonization occurs this round
    if (!random.P(colonization_rate))
      return;

    // Get the colonizing organism's species
    int colonizer_species = pop[pos]->GetSpecies();

    // Get neighboring positions
    std::vector<size_t> neighbors =
        GetNeighborPositions(pos, grid_width, grid_height);

    // Find all valid colonization targets
    std::vector<size_t> valid_targets;
    
    for (size_t neighbor_pos : neighbors) {
      if (IsDestroyed(neighbor_pos)) continue;
      
      if (colonizer_species == 0) {  // Species C (superior competitor)
        // Can colonize empty cells and cells occupied by species D
        if (!IsOccupied(neighbor_pos) || 
            (IsOccupied(neighbor_pos) && pop[neighbor_pos]->GetSpecies() == 1)) {
          valid_targets.push_back(neighbor_pos);
        }
      } else if (colonizer_species == 1) {  // Species D (superior disperser)
        // Can only colonize empty cells
        if (!IsOccupied(neighbor_pos)) {
          valid_targets.push_back(neighbor_pos);
        }
      }
    }
    
    // If there are no valid targets, colonization fails
    if (valid_targets.empty())
      return;
    
    // Randomly select one target from valid options
    size_t target_index = random.GetUInt(valid_targets.size());
    size_t target_pos = valid_targets[target_index];
    
    // Remove existing organism if present (competitive displacement)
    if (IsOccupied(target_pos)) {
      RemoveOrganism(target_pos);
    }
    
    // Create and place offspring
    emp::Ptr<Organism> offspring = pop[pos]->CreateOffspring();
    AddOrgAt(offspring, target_pos);
  }

  /**
   * @brief Remove organism from the world
   * @param i Position index
   */
  void RemoveOrganism(size_t i) {
    if (IsOccupied(i)) {
      pop[i] = nullptr;
    }
  }

  /**
   * @brief Get positions of all neighboring cells
   * @param pos Center position
   * @param width Grid width
   * @param height Grid height
   * @return Vector of neighboring positions
   */
  std::vector<size_t> GetNeighborPositions(size_t pos, int width, int height) {
    std::vector<size_t> neighbors;
    int x = pos % width;
    int y = pos / width;

    // Check all 8 neighbors
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        if (dx == 0 && dy == 0)
          continue; // Skip center position

        int nx = x + dx;
        int ny = y + dy;

        // Check bounds (no toroidal wrapping for this simulation)
        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
          size_t neighbor_pos = ny * width + nx;
          neighbors.push_back(neighbor_pos);
        }
      }
    }
    return neighbors;
  }

  /**
   * @brief Count organisms of each species
   * @return Array with counts [species_c, species_d, empty, destroyed]
   */
  std::array<int, 4> CountCells() {
    int species_c = 0;
    int species_d = 0;
    int empty = 0;
    int destroyed = 0;

    for (size_t i = 0; i < GetSize(); i++) {
      if (IsDestroyed(i)) {
        destroyed++;
      } else if (!IsOccupied(i)) {
        empty++;
      } else {
        int species = pop[i]->GetSpecies();
        if (species == 0)
          species_c++;
        else if (species == 1)
          species_d++;
      }
    }

    return {species_c, species_d, empty, destroyed};
  }

private:
  /**
   * @brief Process a single organism's extinction and colonization
   * @param pos Position of organism to process
   */
  void ProcessOrganism(size_t pos) {
    if (!IsOccupied(pos) || IsDestroyed(pos))
      return;

    // Process the organism using its ProcessInWorld method
    pop[pos]->ProcessInWorld(
        *this, pos); // where attempts at extinction and colonization happen
  }
};

#endif