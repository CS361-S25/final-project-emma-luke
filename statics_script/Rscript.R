# Load necessary libraries
require(ggplot2)
library(tidyr)

# Set path
setwd("/Users/mac/code/cs361/final-project-emma-luke/Experiment data")

# Read the CSV file
data <- read.csv("ideal_result_gradient.csv")

# Convert to long format for ggplot
data_long <- pivot_longer(data,
     cols = c("Species_C", "Species_D", "Empty"),
     names_to = "Cell_Type",
     values_to = "Count"
)

# Plot
ggplot(data_long, aes(x = Destruction, y = Count, color = Cell_Type)) +
     geom_line(size = 1.2) +
     labs(
          title = "Number of Cells Occupied under gradient destruction",
          x = "Proportion of Cells Habitable",
          y = "Average Number of Cells"
     ) +
     theme_minimal()
