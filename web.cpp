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
#include "SpeciesD.h"
#include "ConfigSetup.h"

emp::web::Document doc{"target"};

/**
 * @brief Main animation class for the Habitat Destruction simulation
 * 
 * Visualizes the effects of habitat destruction patterns on species persistence
 * in a cellular automaton model based on metapopulation dynamics.
 */
class Animator : public emp::web::Animate {
    private:
        // Seed to control random number generation
        static constexpr int SEED = 6;    ///< Seed for random number generator

        // Initial parameters, can change for different runs
        double destruction_percentage = 0.5;      ///< Percentage of habitat destroyed
        double initial_occupancy = 0.5;           ///< Initial occupancy of available habitat

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
        const std::string species_d_color = "orange"; ///< Color for species D

    public:
        /**
         * @brief Construct the animator and set up the simulation
         */
        Animator() : canvas(WIDTH, HEIGHT, "canvas"), 
                      random_generator(SEED), 
                      world(random_generator),
                      stats_div("stats") {
            SetupInterface();
            InitializeSimulation();
        }

        /**
         * @brief Process one frame of the simulation
         */
        void DoFrame() override {
            canvas.Clear();
            world.UpdateEcology(); // most of the work is done here...
            DrawWorld();
            UpdateStats();
        }

    private:
        /**
         * @brief Set up the web interface with instructions and controls
         */
        void SetupInterface() {
            doc << "<h2>Habitat Destruction Pattern on Species Persistence</h2>";
            doc << "<p>This simulation demonstrates the baseline scenario from the paper:</p>";
            doc << "<ul>";
            doc << "<li><span style='color: green;'>■</span> <b>Green squares</b>: Empty available habitat</li>";
            doc << "<li><span style='color: black;'>■</span> <b>Black squares</b>: Destroyed habitat (permanently unavailable)</li>";
            doc << "<li><span style='color: orange;'>■</span> <b>Orange squares</b>: Species D (superior disperser)</li>";
            doc << "</ul>";
            doc << "<p>Species D has colonization rate = 0.5, extinction rate = 0.1</p>";
            
            // Controls
            doc << "<div>";
            doc << "Current habitat destruction: " << (destruction_percentage * 100) << "%";
            doc << " (Edit destruction_percentage variable to change)";
            doc << "</div>";
            doc << "<div>";
            doc << GetToggleButton("Toggle");
            doc << " ";
            doc << GetStepButton("Step");
            doc << " ";
            emp::web::Button reset_button([this](){
                InitializeSimulation();
            }, "Reset");
            doc << reset_button;
            doc << "</div>";
            
            doc << "<br>";
            doc << canvas;
            doc << "<br>";
            doc << stats_div;
        }

        /**
         * @brief Initialize the simulation with random habitat destruction
         */
        void InitializeSimulation() {
            // Initialize grid
            world.InitializeGrid(NUM_W_BOXES, NUM_H_BOXES);
            
            // Destroy habitat randomly
            // world.DestroyHabitatRandom(destruction_percentage);
            world.DestroyHabitatGradient(destruction_percentage);
            
            // Populate with species D in available habitat
            PopulateWithSpeciesD();
            
            // Update display
            DrawWorld();
            UpdateStats();
        }

        /**
         * @brief Add species D to random available positions
         */
        void PopulateWithSpeciesD() {
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
                return species_d_color;
            }
        }

        /**
         * @brief Update the statistics display
         */
        void UpdateStats() {
            auto counts = world.CountCells();
            stats_div.Clear();
            stats_div << "<b>Cell Counts:</b> ";
            stats_div << "Species D: " << counts[1] << " | ";
            stats_div << "Empty: " << counts[2] << " | ";
            stats_div << "Destroyed: " << counts[3] << " | ";
            stats_div << "Proportion habitable: " << (1.0 - destruction_percentage);
        }
};

// Global animator instance
Animator animator;

/**
 * @brief Main entry point for the simulation
 * @return Exit status
 */
int main() {
    animator.Step();
}