# Load necessary library
require(ggplot2)

#set path
setwd("/Users/mac/code/cs361/final-project-emma-luke/Experiment data")
# Read the CSV file
data <- read.csv("mean_cell_counts_by_proportion_habitable.csv")

# Convert to long format for ggplot
library(tidyr)w
data_long <- pivot_longer(data, 
                          cols = c("Mean.Species.D", "Mean.Empty"), 
                          names_to = "Cell_Type", 
                          values_to = "Count")

# Plot
ggplot(data_long, aes(x = Proportion.Habitable, y = Count, color = Cell_Type)) +
  geom_line(size = 1.2) +
  labs(title = "Cell Counts by Proportion of Habitat Remaining",
       x = "Proportion of Cells Habitable",
       y = "Average Number of Cells") +
  theme_minimal()