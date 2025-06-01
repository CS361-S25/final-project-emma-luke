#ifndef SPECIES_C_H
#define SPECIES_C_H

#include "Org.h"
#include "World.h"

/**
 * @brief Species C - Superior competitor with inferior dispersal ability
 * 
 * This species has a colonization rate of 0.2 and extinction rate of 0.1.
 * As the superior competitor, it can invade cells occupied by species D
 * as well as empty cells.
 */
class SpeciesC : public Organism {
    private:
        static constexpr double COLONIZATION_RATE = 0.2;  ///< Colonization rate for species c
        static constexpr double EXTINCTION_RATE = 0.1;    ///< Local extinction rate

    public:
        /**
         * @brief Construct a new SpeciesC organism
         * @param _random Pointer to random number generator
         */
        SpeciesC(emp::Ptr<emp::Random> _random) :
            Organism(_random, 0, COLONIZATION_RATE, EXTINCTION_RATE) {}

        /**
         * @brief Process species C behavior in the world
         * @param world Reference to the world
         * @param pos Current position of the organism
         */
        void ProcessInWorld(OrgWorld& world, size_t pos) override {
            // Check for extinction
            if (random->P(extinction_rate)) {
                world.RemoveOrganism(pos);
                return;
            }
            
            // Attempt colonization
            world.TryColonize(pos, colonization_rate);
        }

        /**
         * @brief Create offspring of species C
         * @return Pointer to new species C organism
         */
        emp::Ptr<Organism> CreateOffspring() override {
            return emp::Ptr<SpeciesC>(new SpeciesC(random));
        }
};

#endif