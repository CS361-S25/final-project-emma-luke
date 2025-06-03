#define UIT_VENDORIZE_EMP
#define UIT_SUPPRESS_MACRO_INSEEP_WARNINGS

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/web.hpp"

#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"

#include "World.h"
#include "Org.h"
#include "SpeciesC.h"
#include "SpeciesD.h"
#include "ConfigSetup.h"

emp::web::Document doc{"target"};
MyConfigType config;

/**
 * @brief Main animation class for the Habitat Destruction simulation
 * 
 * Visualizes the effects of habitat destruction patterns on species persistence
 * in a cellular automaton model based on metapopulation dynamics.
 */
class Animator : public emp::web::Animate {
    private:
        // Arena dimensions - 50x50 grid as specified in the paper
        static constexpr int NUM_H_BOXES = 50;    ///< Grid height in cells
        static constexpr int NUM_W_BOXES = 50;    ///< Grid width in cells
        static constexpr double RECT_SIDE = 10;   ///< Size of each cell in pixels
        static constexpr double WIDTH = NUM_W_BOXES * RECT_SIDE;  ///< Canvas width
        static constexpr double HEIGHT = NUM_H_BOXES * RECT_SIDE; ///< Canvas height

        emp::web::Canvas canvas;           ///< Canvas for rendering the simulation
        emp::Random random_generator;      ///< Random number generator
        OrgWorld world;                    ///< The ecosystem world

        // UI elements
        emp::web::Div stats_div;          ///< Div for displaying statistics

        // Visualization colors
        const std::string empty_color = "green";      ///< Color for empty available habitat
        const std::string destroyed_color = "black";  ///< Color for destroyed habitat
        const std::string species_c_color = "blue";   ///< Color for species C (superior competitor)
        const std::string species_d_color = "orange"; ///< Color for species D (superior disperser)

    public:
        /**
         * @brief Construct the animator and set up the simulation
         */
        Animator() : canvas(WIDTH, HEIGHT, "canvas"), 
                     random_generator(config.SEED()), 
                     world(random_generator),
                     stats_div("stats") {
            InitializeConfiguration();
            SetupInterface();
            InitializeSimulation();
        }

        /**
         * @brief Process one frame of the simulation
         */
        void DoFrame() override {
            canvas.Clear();
            world.UpdateEcology();
            DrawWorld();
            UpdateStats();
        }

    private:
        /**
         * @brief Initialize configuration from URL parameters
         */
        void InitializeConfiguration() {
            auto specs = emp::ArgManager::make_builtin_specs(&config);
            emp::ArgManager am(emp::web::GetUrlParams(), specs);
            am.UseCallbacks();
            if (am.HasUnused()) std::exit(EXIT_FAILURE);
            
            // Reset random seed based on config
            random_generator.ResetSeed(config.SEED());
        }

        /**
         * @brief Set up the web interface with instructions and controls
         */
        void SetupInterface() {
            doc << "<h2>Habitat Destruction Pattern on Species Persistence: Two-Species Competition</h2>";
            
            doc << "<p>This simulation demonstrates competition between two species:</p>";
            doc << "<ul>";
            doc << "<li><span style='color: green;'>■</span> <b>Green squares</b>: Empty available habitat</li>";
            doc << "<li><span style='color: black;'>■</span> <b>Black squares</b>: Destroyed habitat (permanently unavailable)</li>";
            doc << "<li><span style='color: blue;'>■</span> <b>Blue squares</b>: Species C (superior competitor, colonization rate = 0.2)</li>";
            doc << "<li><span style='color: orange;'>■</span> <b>Orange squares</b>: Species D (superior disperser, colonization rate = 0.5)</li>";
            doc << "</ul>";
            doc << "<p>Both species have extinction rate = 0.1. Species C can invade cells occupied by Species D.</p>";
            doc << "<p>Initially, 50% of available habitat is populated evenly by both species.</p>";
            
            // Control buttons
            doc << "<div>";
            doc << GetToggleButton("Toggle");
            doc << " ";
            doc << GetStepButton("Step");
            // doc << " ";
            // emp::web::Button reset_button([this](){
            //     // Re-read configuration and reset
            //     random_generator.ResetSeed(config.SEED());
            //     InitializeSimulation();
            // }, "Reset");
            // doc << reset_button;
            doc << "</div>";
            
            doc << "<br>";
            doc << canvas;
            doc << "<br>";
            doc << stats_div;
            doc << "<br>";
            
            // Add configuration panel
            emp::prefab::ConfigPanel config_panel(config);
            
            // Customize the sliders with appropriate ranges
            config_panel.SetRange("SEED", "1", "100", "1");                     // Seed range 1-100
            config_panel.SetRange("PERCENT_DESTROYED", "0.25", "0.75", "0.01"); // Destruction 25%-75%
            config_panel.SetRange("DESTRUCTION_PATTERN", "0", "1", "1");        // Pattern: 0=Random, 1=Gradient
            
            doc << "<h3>Configuration Parameters</h3>";
            doc << config_panel;
        }

        /**
         * @brief Initialize the simulation with habitat destruction
         */
        void InitializeSimulation() {
            // Initialize grid
            world.InitializeGrid(NUM_W_BOXES, NUM_H_BOXES);
            
            // Destroy habitat based on selected pattern using config value
            if (config.DESTRUCTION_PATTERN() == 0) {
                world.DestroyHabitatRandom(config.PERCENT_DESTROYED());
            } else {
                world.DestroyHabitatGradient(config.PERCENT_DESTROYED());
            }
            
            // Populate with both species
            PopulateWithBothSpecies();
            
            // Update display
            DrawWorld();
            UpdateStats();
        }

        /**
         * @brief Add both species to occupy 25% each of available habitat
         * 
         * Based on the paper's specification that "each species occupies 0.25 
         * of the remaining available habitat" at initialization.
         */
        void PopulateWithBothSpecies() {
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
         * @brief Render the current state of the world
         */
        void DrawWorld() {
            canvas.Clear();
            
            for (int x = 0; x < NUM_W_BOXES; x++) {
                for (int y = 0; y < NUM_H_BOXES; y++) {
                    size_t pos = y * NUM_W_BOXES + x;
                    std::string color = GetCellColor(pos);
                    DrawCell(x, y, color);
                }
            }
        }

        /**
         * @brief Draw a single cell on the canvas
         * @param x X coordinate in grid
         * @param y Y coordinate in grid
         * @param color Fill color for the cell
         */
        void DrawCell(int x, int y, const std::string& color) {
            canvas.Rect(x * RECT_SIDE, y * RECT_SIDE, RECT_SIDE, RECT_SIDE, color, "black");
        }

        /**
         * @brief Get the color for a cell based on its contents
         * @param pos Position in the world grid
         * @return Color string for the cell
         */
        std::string GetCellColor(size_t pos) {
            if (world.IsDestroyed(pos)) {
                return destroyed_color;
            } else if (!world.IsOccupied(pos)) {
                return empty_color;
            } else {
                int species = world.GetOrg(pos).GetSpecies();
                return (species == 0) ? species_c_color : species_d_color;
            }
        }

        /**
         * @brief Update the statistics display
         */
        void UpdateStats() {
            auto counts = world.CountCells();
            stats_div.Clear();
            stats_div << "<b>Cell Counts:</b> ";
            stats_div << "Species C: " << counts[0] << " | ";
            stats_div << "Species D: " << counts[1] << " | ";
            stats_div << "Empty: " << counts[2] << " | ";
            stats_div << "Destroyed: " << counts[3] << " | ";
            stats_div << "Proportion habitable: " << (1.0 - config.PERCENT_DESTROYED()) << " | ";
            stats_div << "Pattern: " << (config.DESTRUCTION_PATTERN() == 0 ? "Random" : "Gradient");
        }
};

// Global animator instance
Animator animator;

/**
 * @brief Main entry point for the simulation
 * @return Exit status
 */
int main() {
    // Initialize the animator with the first frame
    // animator.Step();
    return 0;
}