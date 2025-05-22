#ifndef SPECIES_D_H
#define SPECIES_D_H

#include "Org.h"
#include "World.h"

/**
 * @brief Species D - Superior disperser with inferior competitive ability
 * 
 * This species has a colonization rate of 0.5 and extinction rate of 0.1.
 * In the full model, it would be displaced by species C, but for the
 * baseline we only implement species D.
 */
class SpeciesD : public Organism {
    private:
        static constexpr double COLONIZATION_RATE = 0.5;  ///< Colonization rate for species d
        static constexpr double EXTINCTION_RATE = 0.1;    ///< Local extinction rate

    public:
        /**
         * @brief Construct a new SpeciesD organism
         * @param _random Pointer to random number generator
         */
        SpeciesD(emp::Ptr<emp::Random> _random) :
            Organism(_random, 1, COLONIZATION_RATE, EXTINCTION_RATE) {}

        /**
         * @brief Process species D behavior in the world
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
         * @brief Create offspring of species D
         * @return Pointer to new species D organism
         */
        emp::Ptr<Organism> CreateOffspring() override {
            return emp::Ptr<SpeciesD>(new SpeciesD(random));
        }
};

#endif