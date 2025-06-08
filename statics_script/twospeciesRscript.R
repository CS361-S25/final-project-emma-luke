# Load necessary libraries
library(ggplot2)
library(tidyr)
library(dplyr)

# Set path
setwd("/Users/mac/code/cs361/final-project-emma-luke/Experiment data")

# Load both datasets
data_random <- read.csv("ideal_result_random.csv")
data_gradient <- read.csv("ideal_result_gradient.csv")

# Add a column to indicate destruction type
data_random$Destruction_Type <- "Random"
data_gradient$Destruction_Type <- "Gradient"

# Combine both datasets
data_combined <- rbind(data_random, data_gradient)

# Convert to long format for ggplot
data_long <- pivot_longer(data_combined,
    cols = c("Species_C", "Species_D", "Empty"),
    names_to = "Cell_Type",
    values_to = "Count"
)

# Plot side-by-side with facet
ggplot(data_long, aes(x = Destruction, y = Count, color = Cell_Type)) +
  geom_line(size = 1.2) +
  facet_wrap(~Destruction_Type) +
  labs(title = "Cell Occupancy Under Different Habitat Destruction Patterns",
       x = "Proportion of Cells Habitable",
       y = "Average Number of Cells") +
  scale_color_manual(values = c(
    "Species_C" = "violet",
    "Species_D" = "darkturquoise",
    "Empty" = "salmon"
  )) +
  theme_minimal()

