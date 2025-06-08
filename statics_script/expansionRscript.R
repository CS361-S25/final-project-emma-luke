# Load necessary libraries
library(ggplot2)
library(tidyr)
library(dplyr)

# Set path
setwd("/Users/mac/code/cs361/final-project-emma-luke/Experiment data")

# Load the expansion result dataset
data_expansion <- read.csv("expansion.csv")

# Convert to long format for ggplot
data_long <- pivot_longer(data_expansion,
    cols = c("Species_C", "Species_D", "Empty"),
    names_to = "Cell_Type",
    values_to = "Count"
)

# Plot over rounds (x-axis)
ggplot(data_long, aes(x = Rounds, y = Count, color = Cell_Type)) +
    geom_line(size = 1.2) +
    labs(
        title = "Stepwise Habitat Destruction Over Time",
        x = "Round",
        y = "Average Number of Cells"
    ) +
    scale_color_manual(values = c(
        "Species_C" = "violet",
        "Species_D" = "darkturquoise",
        "Empty" = "salmon"
    )) +
    theme_minimal()
