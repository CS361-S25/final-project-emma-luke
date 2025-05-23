#ifndef ORG_H
#define ORG_H

#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

// Forward declaration
class OrgWorld;

/**
 * @brief Base class for all organisms in the habitat destruction simulation
 * 
 * This abstract base class defines the common interface for organisms
 * in the metapopulation model with extinction and colonization dynamics.
 */
class Organism {
    protected:
        emp::Ptr<emp::Random> random;   ///< Random number generator for stochastic behaviors
        int species;                    ///< Species identifier (0=c, 1=d)
        double colonization_rate;       ///< Rate of colonization for this species
        double extinction_rate;         ///< Rate of local extinction

    public:
        /**
         * @brief Construct a new Organism
         * @param _random Pointer to random number generator
         * @param _species Species identifier
         * @param _colonization_rate Colonization rate
         * @param _extinction_rate Extinction rate
         */
        Organism(emp::Ptr<emp::Random> _random, int _species, 
                double _colonization_rate, double _extinction_rate) :
            random(_random), species(_species), 
            colonization_rate(_colonization_rate), 
            extinction_rate(_extinction_rate) {}

        virtual ~Organism() = default;

        /**
         * @brief Get species identifier
         * @return Species ID (0=c, 1=d)
         */
        int GetSpecies() const { return species; }

        /**
         * @brief Get colonization rate
         * @return Colonization rate
         */
        double GetColonizationRate() const { return colonization_rate; }

        /**
         * @brief Get extinction rate
         * @return Extinction rate
         */
        double GetExtinctionRate() const { return extinction_rate; }

        /**
         * @brief Process organism behavior within the world context
         * @param world Reference to the world
         * @param pos Current position in the world
         */
        virtual void ProcessInWorld(OrgWorld& world, size_t pos) = 0;

        /**
         * @brief Create offspring of this organism
         * @return Pointer to new organism offspring
         */
        virtual emp::Ptr<Organism> CreateOffspring() = 0;
        
};

#endif