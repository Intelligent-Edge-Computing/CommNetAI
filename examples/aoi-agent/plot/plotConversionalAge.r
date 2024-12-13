library(DBI)

library(ggplot2)
library(hrbrthemes)
library(ggforce)
library(dplyr)
library(tidyr)

# pdf("sensing-area-line-chart.pdf", width = 4.5, height = 4, pointsize = 12)

con <- dbConnect(RSQLite::SQLite(), "logs.db")
# 查询满足Phase条件的数据并按照Timestamp排序
# 定义SQL查询
query <- "
SELECT Timestamp, (Timestamp-generatedTime) AS newAge, (Timestamp-lastServerStatusGeneratedTime) AS oldAge
FROM StageOneLogs
WHERE  runID = 1;
"

res <- dbSendQuery(con, query)
data <- dbFetch(res)
# 断开数据库连接
dbDisconnect(con)

# 绘制图像
ggplot(data, aes(x = Timestamp, y = Age)) +
  geom_line() +
  geom_point() +
  # scale_x_continuous(limits = c(2500, 2600)) +  # 限制x轴范围为2500到5000
  labs(title = "Age Evolution",
       x = "Time (ms)",
       y = "Age (ms)") +
  scale_y_continuous(limits = c(0, NA)) +  # 将y轴从4开始
  theme_minimal()
