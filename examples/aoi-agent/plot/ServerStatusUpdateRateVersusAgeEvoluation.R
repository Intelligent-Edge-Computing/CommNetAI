library(DBI)

library(ggplot2)
library(hrbrthemes)
library(ggforce)
library(dplyr)
library(tidyr)
library(scales)
con <- dbConnect(RSQLite::SQLite(), "logs.db")
# 查询满足Phase条件的数据并按照Timestamp排序
# 定义SQL查询
avg_query <- "
SELECT (SUM(0.5*oldAge*oldAge - 0.5*newAge*newAge) + 0.5 * (MAX(apReceiveTime)-MAX(generatedTime))* (MAX(apReceiveTime)-MAX(generatedTime))  )/(MAX(apReceiveTime) - MIN(generatedTime)) AS Age,
       1/AVG(generatedTime - lastServerStatusGeneratedTime) AS UpdateRate,
       runID
FROM
(SELECT Timestamp, apReceiveTime, generatedTime, lastServerStatusGeneratedTime, (Timestamp-generatedTime) AS newAge, (Timestamp-lastServerStatusGeneratedTime) AS oldAge, runID
FROM StageOneLogs)
GROUP BY runID
HAVING Age < 1
"
res <- dbSendQuery(con, avg_query)
data <- dbFetch(res)
# 断开数据库连接
dbDisconnect(con)

pdf("./plot/figures/server-status-update-rate-versus-average-age.pdf", width = 4.5, height = 4, pointsize = 12)

# 绘制图像
ggplot(data, aes(x = UpdateRate, y = Age)) +
  # geom_line() +
  geom_point(shape = 21, size = 0.5) +
  # geom_smooth(method = "lm", formula = y ~ poly(x, 20), se = TRUE, fill = "grey") +
  # scale_x_continuous(limits = c(2500, 2600)) +  # 限制x轴范围为2500到5000
  labs(
       x = "Status Update Rate (times/s)",
       y = "Average Age (s)") +
  # scale_y_continuous(limits = c(0, NA)) +  # 将y轴从4开始
  theme_minimal()

# 关闭PDF设备，保存文件
dev.off()
