library(DBI)

library(ggplot2)
library(hrbrthemes)
library(ggforce)

# pdf("sensing-area-line-chart.pdf", width = 4.5, height = 4, pointsize = 12)

con <- dbConnect(RSQLite::SQLite(), "logs.db")
res <- dbSendQuery(con,
                                  "SELECT Method as Method, DeadlineThreshold as 'Deadline Threshold [ms]',
                                             Distance * 1000 as 'Sensing Radius [ms]'
                                             FROM sensing_area_details ORDER BY Method,  DeadlineThreshold DESC ;")
data <- dbFetch(res)
res <- dbSendQuery(con,
                   "SELECT COUNT(*) FROM (SELECT DISTINCT method FROM accept_rate_varing_ddl);")
number_of_methods <- dbFetch(res)
dbDisconnect(con)


cbPalette <- c("#999999", "#E69F00", "#56B4E9", "#009E73", "#F0E442",  "#D55E00", "#0072B2","#CC79A7")

my_colors = cbPalette[1:number_of_methods[1,]] #c("black","grey50","grey50","grey75","white")
names(my_colors) <- levels(factor(data$Method))
# my_scale <- scale_color_manual(name = "Method", values = my_colors)
my_scale <- scale_fill_manual(name = "Method", values = my_colors)


ggplot(data, aes(x = `Deadline Threshold [ms]`, y = `Sensing Radius [ms]`, fill = Method, group=`Deadline Threshold [ms]`)) +
geom_boxplot() +
stat_summary(fun = "mean", geom = "point", shape = 1,
           size = 2, color = "white") +
my_scale+
coord_fixed(ratio = 1) +
facet_wrap(~Method)+
scale_x_continuous("Deadline Threshold [ms]", labels = as.character(data$`Deadline Threshold [ms]`), breaks = data$`Deadline Threshold [ms]`) +

theme_bw()

# The list below summarizes the minimum, Q1 (First Quartile), median, Q3 (Third Quartile),
# and maximum values.  Summarizing these values can provide us with information about our outliers and their values.