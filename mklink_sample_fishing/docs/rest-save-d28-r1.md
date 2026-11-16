# D28R1 休息存档合并 / Held-rest save coalescing

MKLink V4，2026-11-02。长按休息以20倍推进游戏时间，猫的小时到访／跨日喂食重置、海滩每日刷新等事件仍正常发生，但各自提出的保存请求过去会反复启动SD写入，导致频繁显示“存档中”。

休息期间累积这些请求，松开按键或旋转离开休息选项后，合并保存当时最新的位置、时间、天气、潮湿、增益、猫与赶海状态。已经开始的异步写入可正常完成，不会清掉后来积累的请求；休息结束后再保存最新状态。正常购买、出售、手动保存和错误重试不变。

仍保留每5个真实分钟的定时检查，特别长的持续休息也可写检查点，间隔不随20倍游戏时间缩短。松开后再保存一次结束状态。尚未到检查点且持续休息时掉电，恢复到最近一次成功存档；不能保证尚未写入的数据不丢失。

V22仍2147B、独立A/B，原有2026.10.31.4四个资源包不变，不增加RAM或位图。回归检查使用真实输入和时间更新，覆盖三图超过一个游戏日的长按、松开／旋转各一次最终保存、重启恢复、真实五分钟检查点及写入中继续积累请求。沿用原有FatFs回读、错误重试和下载器保护检查。实板验收待补。

Held rest advances world time20x, so cat schedules and daily resets used to trigger repeated SD writes. Requests now merge while resting and flush once on release or rotary cancellation, saving the latest state. Existing asynchronous writes finish normally without consuming later pending requests. Five-minute wall-clock checkpoints still run during unusually long holds; gameplay time does not shorten that interval.

V22 saves and2026.10.31.4 resource packs remain unchanged. No new RAM or images are needed. Tests cover all three maps, cross-day holds, release/rotation, latest-state reload, periodic checkpoints and in-flight write completion. Unwritten progress since the last successful checkpoint remains volatile on power loss. Hardware acceptance is pending.

## 构建核验 / Build evidence

131项主机检查通过（93＋38），GNU MCU固件及公共素材核验通过。应用Flash余9520B，比D28占用增加24B；DLM余1776B，AHB余480B，静态RAM不变。只有既有编译告警，实板长按／掉电仍待验。

All131 host checks pass. MCU build and runtime-asset verification pass; static RAM is unchanged. Application Flash retains9520B. Only existing compiler warnings remain; hardware validation is pending.

UF2 SHA256: `db1db99f92b8830fb156165665f98515841bc9e7abfeb22e93f76a20e42bd153`
