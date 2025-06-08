# Habitat Destruction and Species Competition Simulation

## Overview

This project implements a spatial simulation of two-species competition under habitat destruction, based on metapopulation dynamics. The simulation explores how different patterns of habitat destruction (random vs. gradient) affect species persistence in a competitive environment. The project was meant to replicate the results of the 1995 publication by Calvin Dytham titled "The Effect of Habitat Destruction Pattern on Species Persistence: A Cellular Model".

## Biological Model

### Species Characteristics

The simulation models two competing species with different ecological strategies:

- **Species C (Blue)**: Superior competitor
  - Colonization rate: 0.2
  - Extinction rate: 0.1
  - Can invade cells occupied by Species D
  - Can colonize empty cells

- **Species D (Orange)**: Superior disperser
  - Colonization rate: 0.5
  - Extinction rate: 0.1
  - Can only colonize empty cells

### Dynamics

The simulation operates on a 50x50 grid where each cell can be:
- **Green**: Empty available habitat
- **Black**: Destroyed habitat (permanently unavailable)
- **Blue**: Occupied by Species C
- **Orange**: Occupied by Species D

Each round, organisms face:
1. **Local extinction**: Each occupied cell has a probability of extinction (0.1 for both species)
2. **Colonization**: Organisms attempt to colonize neighboring cells based on their colonization rates

## Habitat Destruction Features

### Destruction Patterns

1. **Random Destruction**: Cells are destroyed randomly across the landscape
2. **Gradient Destruction**: Destruction probability decreases linearly from left (highest) to right (lowest), maintaining the overall destruction percentage

### Destruction Timing

- **Immediate Destruction** (DESTRUCTION_ROUNDS = 0): All habitat destruction occurs before the simulation starts
- **Incremental Destruction** (DESTRUCTION_ROUNDS = 1-100): Habitat is destroyed gradually over the specified number of rounds, with cells destroyed linearly over time

When a cell is destroyed incrementally:
- Any organism currently occupying that cell is killed
- The cell becomes permanently unavailable for future colonization

## Configuration Parameters

- **SEED**: Random seed for reproducibility (1-100)
- **DESTRUCTION_PATTERN**: 0 = Random, 1 = Gradient
- **PERCENT_DESTROYED**: Proportion of habitat to destroy (0.25-0.75)
- **DESTRUCTION_ROUNDS**: Number of rounds for incremental destruction (0-100, where 0 = immediate)

## File Structure

### Core Simulation Files

- **Org.h**: Base organism class defining the interface for all species
- **SpeciesC.h**: Implementation of Species C (superior competitor)
- **SpeciesD.h**: Implementation of Species D (superior disperser)
- **World.h**: Main world class managing the grid, organisms, and habitat destruction
- **ConfigSetup.h**: Configuration parameter definitions

### Application Files

- **web.cpp**: Web-based interactive visualization using Empirical
- **native.cpp**: Command-line version for batch experiments
- **MySettings.cfg**: Default configuration file

## Building and Running

### Web Version

The web version provides an interactive visualization where you can:
- Adjust parameters using sliders
- Start/stop/step through the simulation
- View real-time statistics including round count
- See the spatial dynamics unfold

### Native Version

The native version runs experiments across different destruction levels (25%-75%) and outputs results to a CSV file. It's designed for collecting data on how destruction levels affect species persistence.

## Implementation Details

### Neighborhood

Each cell has 8 neighbors. The grid has hard boundaries (no toroidal wrapping).

### Processing Order

Within each round:
1. Incremental destruction occurs (if active)
2. All organisms are processed in random order for:
   - Extinction events
   - Colonization attempts

### Initial Conditions

- Each species initially occupies 25% of the available (non-destroyed) habitat
- Initial placement is random within available cells

## Output

### Web Version
- Visual display of the grid state
- Real-time statistics: round number, species counts, empty cells, destroyed cells
- Proportion of habitable area and destruction pattern

### Native Version
- CSV file with columns: Destruction, Species_C, Species_D, Empty, Destroyed
- One row per destruction level tested
- Results after 1000 rounds of simulation

## Dependencies

This project uses the [Empirical library](https://github.com/devosoft/Empirical) for:
- Web interface components
- Random number generation
- Configuration management
- Data collection utilities