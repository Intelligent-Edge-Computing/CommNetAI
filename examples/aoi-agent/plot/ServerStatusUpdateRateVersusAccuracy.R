library(DBI)

library(ggplot2)
library(hrbrthemes)
library(ggforce)
library(dplyr)
library(tidyr)
library(scales)

# pdf("sensing-area-line-chart.pdf", width = 4.5, height = 4, pointsize = 12)

con <- dbConnect(RSQLite::SQLite(), "logs.db")
# 查询满足Phase条件的数据并按照Timestamp排序
# 定义SQL查询
query <- "
WITH ResultTable AS (
    SELECT
        (SUM(0.5 * oldAge * oldAge - 0.5 * newAge * newAge) + 0.5 * (MAX(apReceiveTime) - MAX(generatedTime)) * (MAX(apReceiveTime) - MAX(generatedTime))) /
        (MAX(apReceiveTime) - MIN(generatedTime)) AS Age,
        1.0 / AVG(generatedTime - lastServerStatusGeneratedTime) AS UpdateRate,
        runID
    FROM (
        SELECT
            Timestamp,
            apReceiveTime,
            generatedTime,
            lastServerStatusGeneratedTime,
            (Timestamp - generatedTime) AS newAge,
            (Timestamp - lastServerStatusGeneratedTime) AS oldAge,
            runID
        FROM StageOneLogs
    )
    GROUP BY runID
)
SELECT
    rt.Age AS Age,
    rt.UpdateRate,
    1.0 * (r.TP + r.TN ) / (r.TP + r.TN + r.FP + r.FN) AS Acc,
    r.TP,
    r.FP,
    r.TN,
    r.FN,
    rt.runID
FROM
    ResultTable AS rt
JOIN
    Accuracy AS r
ON
    rt.runID = r.runID
WHERE Age < 1
"
res <- dbSendQuery(con, query)
data <- dbFetch(res)
dbDisconnect(con)

pdf("./plot/figures/server-status-update-rate-versus-accuracy.pdf", width = 4.5, height = 4, pointsize = 12)

# 绘制图像
ggplot(data, aes(x = UpdateRate, y = Acc)) +
  geom_point(shape = 21, size = 0.5) +
  # geom_ribbon(aes(ymin = SuccessRate- 0.1, ymax = SuccessRate + 0.1), alpha = 0.5)+
  # geom_smooth(method = "lm", formula = y ~ poly(x, 10), se = TRUE, fill = "grey") +
  labs(
       x = "Status Update Rate (times/s)",
       y = "Accuracy (%)") +
  theme_minimal()

dev.off()
