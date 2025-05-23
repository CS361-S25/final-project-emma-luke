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
  emp::Ptr<emp::DataMonitor<int>> data_node_species_c;
  emp::Ptr<emp::DataMonitor<int>> data_node_species_d;
  emp::Ptr<emp::DataMonitor<int>> data_node_empty;
  emp::Ptr<emp::DataMonitor<int>> data_node_destroyed;
  std::vector<bool>
      destroyed_cells; ///< Track which cells are destroyed habitat
  int grid_width;
  int grid_height;

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
    if (data_node_species_c)
      data_node_species_c.Delete();
    if (data_node_species_d)
      data_node_species_d.Delete();
    if (data_node_empty)
      data_node_empty.Delete();
    if (data_node_destroyed)
      data_node_destroyed.Delete();
    random_ptr.Delete();
  }

  emp::DataMonitor<int> &GetSpeciesCDataNode() {
    if (!data_node_species_c) {
      data_node_species_c.New();
      OnUpdate([this](size_t) {
        data_node_species_c->Reset();
        int count = 0;
        for (size_t i = 0; i < GetSize(); i++) {
          if (IsOccupied(i) && !IsDestroyed(i) && pop[i]->GetSpecies() == 0) {
            count++;
          }
        }
        data_node_species_c->AddDatum(count);
      });
    }
    return *data_node_species_c;
  }

  emp::DataMonitor<int> &GetSpeciesDDataNode() {
    if (!data_node_species_d) {
      data_node_species_d.New();
      OnUpdate([this](size_t) {
        data_node_species_d->Reset();
        int count = 0;
        for (size_t i = 0; i < GetSize(); i++) {
          if (IsOccupied(i) && !IsDestroyed(i) && pop[i]->GetSpecies() == 1) {
            count++;
          }
        }
        data_node_species_d->AddDatum(count);
      });
    }
    return *data_node_species_d;
  }

  emp::DataMonitor<int> &GetEmptyDataNode() {
    if (!data_node_empty) {
      data_node_empty.New();
      OnUpdate([this](size_t) {
        data_node_empty->Reset();
        int count = 0;
        for (size_t i = 0; i < GetSize(); i++) {
          if (!IsOccupied(i) && !IsDestroyed(i)) {
            count++;
          }
        }
        data_node_empty->AddDatum(count);
      });
    }
    return *data_node_empty;
  }

  emp::DataMonitor<int> &GetDestroyedDataNode() {
    if (!data_node_destroyed) {
      data_node_destroyed.New();
      OnUpdate([this](size_t) {
        data_node_destroyed->Reset();
        int count = 0;
        for (size_t i = 0; i < GetSize(); i++) {
          if (IsDestroyed(i)) {
            count++;
          }
        }
        data_node_destroyed->AddDatum(count);
      });
    }
    return *data_node_destroyed;
  }

  emp::DataFile &SetupResultsFile(const std::string &filename) {
    auto &file = SetupFile(filename);
    auto &node_c = GetSpeciesCDataNode();
    auto &node_d = GetSpeciesDDataNode();
    auto &node_empty = GetEmptyDataNode();
    auto &node_destroyed = GetDestroyedDataNode();

    file.AddVar(update, "update", "Update");
    file.AddTotal(node_c, "species_c", "Number of Species C organisms");
    file.AddTotal(node_d, "species_d", "Number of Species D organisms");
    file.AddTotal(node_empty, "empty", "Number of empty cells");
    file.AddTotal(node_destroyed, "destroyed", "Number of destroyed cells");
    file.PrintHeaderKeys();
    return file;
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

    // Shuffle for random processing order
    emp::Shuffle(random, occupied_positions);

    // Process organisms for extinction and colonization
    for (size_t pos : occupied_positions) {
      if (IsOccupied(pos)) {
        ProcessOrganism(pos);
      }
    }
  }

  /**
   * @brief Try to colonize an empty neighboring cell
   * @param pos Position of colonizing organism
   * @param colonization_rate Rate of colonization
   * @return True if colonization was successful
   */
  bool TryColonize(size_t pos, double colonization_rate) {
    if (!IsOccupied(pos) || IsDestroyed(pos))
      return false;

    // Get neighboring positions
    std::vector<size_t> neighbors =
        GetNeighborPositions(pos, grid_width, grid_height);

    // Find available (empty and not destroyed) neighbors
    std::vector<size_t> available_neighbors;
    for (size_t neighbor_pos : neighbors) {
      if (!IsOccupied(neighbor_pos) && IsAvailable(neighbor_pos)) {
        available_neighbors.push_back(neighbor_pos);
      }
    }

    if (available_neighbors.empty())
      return false;

    // Attempt colonization based on rate
    if (random.P(colonization_rate)) {
      size_t target =
          available_neighbors[random.GetUInt(available_neighbors.size())];
      emp::Ptr<Organism> offspring = pop[pos]->CreateOffspring();
      AddOrgAt(offspring, target);
      return true;
    }

    return false;
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