library(ggplot2)
library(distributions3)
library(latex2exp)

savepdf = F

set.seed(27)

X <- Erlang(3, 20)

data.y <- c()               # Create empty list
data.x <- c()               # Create empty list
data.tag <- c()
data.shape <- c()
num_samples <- 1000
for(i in 1:num_samples ) {                 # Head of for-loop
  xi <- i/(num_samples)
  pi <- cdf(X, i/(num_samples))  #quantile(X, pi)           # Output of iteration i
  data.x[i] <- xi        # Store output in list
  data.y[i] <- pi        # Store output in list
  data.tag[i] <- "F_{Y_3}(y)"
  data.shape[i] <- 1

  data.x[i+num_samples] <- xi
  data.y[i+num_samples] <- 1-pi
  data.tag[i+num_samples] <- "1 - F_{Y_k}(y)"
  data.shape[i+num_samples] <- 2
}


data <- data.frame(
  x = data.x * 60 * 1000, # in ms
  y = data.y,
  group = data.tag,
  shape = data.shape
)

P <- ggplot(data=data, aes(x=data$x, y = data$y, group=data$group, color=factor(data$group))) +
  ylab("Probility") +
  xlab("Time Interval (y) [ms]") +
  geom_point(size=2, shape = data$shape) +
  annotate("label", x = 3000, y = 0.93, size = 4, label = TeX('$F_{Y_3}(y)$'), fill=NA, label.size = NA)+
  annotate("label", x = 3000, y = 0.1, size = 4, label = TeX('$F(y) = 1 - F_{Y_3}(y)$'), fill=NA, label.size = NA)+
  geom_vline(xintercept = 300, colour='grey',linetype='dashed' ) +
  geom_hline(yintercept = data[which(data$x==300), ]$y[2], colour='grey',linetype='dashed' ) +
  geom_point(x=300, y = data[which(data$x==300), ]$y[2], size=2, color = 'black') +
  annotate("text", x = 1000, y = 0.75, size = 5, label ='(312, 0.91)')+
  geom_segment(x = 1000, y = 0.78, xend = 500, yend = 0.875,
               arrow = arrow(length = unit(0.03, "npc")), color='black') +
  theme_bw() +
  theme(legend.position = "none")
P


# if(savepdf)ggsave("../figures/erlang-dist.pdf", P, device = "pdf", width = 3, height = 2, pointsize = 12)
