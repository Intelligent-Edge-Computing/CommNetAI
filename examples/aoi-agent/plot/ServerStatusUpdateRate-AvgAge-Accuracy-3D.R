library(DBI)

library(hrbrthemes)
library(ggforce)
library(dplyr)
library(tidyr)
library(scales)
library(plotly)
library(RSQLite)
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
"
res <- dbSendQuery(con, query)
data <- dbFetch(res)
dbDisconnect(con)

# pdf("./plot/figures/age-update_rate--success_rate-3D.pdf", width = 4.5, height = 4, pointsize = 12)
#
# # 标准化函数
# normalize <- function(x) {
#   return((x - min(x)) / (max(x) - min(x)))
# }
#
# # 计算标准化的 UpdateRate 和 Age
# data$UpdateRate_norm <- normalize(data$UpdateRate)
# data$Age_norm <- normalize(data$Age)
# # 计算颜色映射值
# data$color_value <- data$UpdateRate_norm + data$Age_norm
data<- data[data$Age < 0.2, ]
fig <- plot_ly(data, x = ~Age, y = ~UpdateRate, z = ~Acc, type = 'scatter3d', mode = 'markers')
fig <- plot_ly(data, x = ~Age, y = ~UpdateRate, z = ~Acc, color = ~Acc, colors = c('#BF382A', '#0C4B8E'), type = 'scatter3d', mode = 'markers',
               marker = list(
                size = 3,
                color = ~Acc,  # 根据 Acc 值设置颜色
                opacity = 0.8  # 设置透明度为 0.8
            ))
fig <- fig %>% layout(
  scene = list(
    xaxis = list(title = 'Average Age (s)'),
    yaxis = list(title = 'Update Rate (times/s)'),
    zaxis = list(title = 'Accuracy (%)')
  )
)

fig
# dev.off()
