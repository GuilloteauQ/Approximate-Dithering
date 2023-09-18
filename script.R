library(tidyverse)


df <- read_csv("data.csv", col_names = c("nb_threads", "chunk", "time"))

time_seq <- df %>% filter(nb_threads == -1) %>% pull(time) %>% mean() * -1 


df %>%
    filter(nb_threads > 0) %>%
    group_by(nb_threads, chunk) %>%
    summarize(speedup = mean(time_seq / time), sd = sd(time_seq / time), bar = 2 * sd / sqrt(n())) %>%
    ggplot(aes(x = factor(nb_threads), y = speedup, color = factor(chunk), group = chunk)) +
    geom_point() +
    geom_errorbar(aes(ymin=speedup - bar, ymax = speedup + bar), width=.2) +
    geom_line() +
    geom_hline(yintercept = 1, linetype = "dashed") +
    # scale_y_log10() +
    theme_bw() +
    theme(legend.position = "bottom")
