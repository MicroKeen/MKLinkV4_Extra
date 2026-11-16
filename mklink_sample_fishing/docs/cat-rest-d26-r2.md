# D26R2 背包时钟与休息计时 / Backpack clock and rest duration

2026-11-02，MKLink V4。背包上方显示当前24小时制游戏时间，位于金币下方、状态图标左侧；所有背包选项均可查看，不与最大金币数、潮湿或拿铁图标重叠。沿用原字号。

选中“休息”，长按时详情显示“已休息2小时15分”等本次累计游戏时长，随20倍时间推进实时更新。松开或旋转离开后计数停止，仍可回到休息项查看本次结果；再次按住休息重新计时。跨午夜不归零，超过一天仍显示累计小时。上方时钟同时显示当前时间，方便确定何时停止。

累计时长仅使用4字节临时状态，不写入存档。读档清零上次休息显示，保留实际推进后的日期、时间、天气等；不会自动继续休息。V21的2147B存档及完整2026.10.31.4资源保持不变，无新图片、整屏缓冲或堆申请。

The backpack now shows the current24-hour game clock below the coin count and left of status icons. Rest details display the current session's elapsed game hours/minutes, freeze on release or cancellation, and reset on the next press. The counter continues across midnight and supports durations beyond24hours. A4-byte transient counter is excluded from saves; loading clears the display while preserving the resulting world time. V21 saves and2026.10.31.4 resources are unchanged.

## 验证结果 / Results

127／127项主机检查通过。休息计数随实际加速时长累计、松开与旋转取消后冻结、下次按住归零、读档清零均通过。90帧场景含三图背包时钟、休息进行中／结束后、跨午夜、25小时05分、最大金币与两个状态图标的排版。使用原字号与PFB绘制，未引入图片、堆分配或整屏缓存。实板仍待验收。
