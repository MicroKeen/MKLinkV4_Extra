# D14 线组损坏概率与泡面 / Rig damage and noodles

2026-10-26，MKLink V4，源代码与固件上传版，作者于本日授权同步；新增内容尚待实机验证。接续D13长竿、雨伞和泡面，装备价格及原有补给规则保持。

每次真正断线（张力过载或持续超出线长）只进行一次10%损坏判定。损坏才移除当前线组、自动选用备用线组并请求存档；其余90%保留当前线组和装备选择，下一竿可以继续使用。断线仍会失去本次鱼获，原有1200ms失败动画及搏鱼难度保持。

普通鱼松线脱钩、提竿过早、错过咬钩不损坏线组，也不消耗这次普通断线判定。失败动画及随后的空闲帧不重复抽取。两张地图、四种线组使用同一概率。每次独立抽取，不是累计十次必坏一次。

河豚（暗纹东方鲀）中钩后逃脱时，额外独立判定20%咬断当前线组，提示严格为“它咬断了线组”，使用断线动画。若同时发生普通断线损坏，只扣一副当前线组，不损坏备用线组。普通断线10%与河豚咬线20%独立，因此河豚因过载/超距逃脱的总损坏概率为28%；河豚松线逃脱仅有20%咬线概率。提早提竿、错过咬钩和成功收鱼不触发咬线。

提示跟随钓鱼情境：保住线组时“鱼跑了，再来一竿”，损坏时“鱼跑了，得换副线了”；没有备用时“没线用了，去店里补一副”。保持原12像素字体，为“它咬断了线组”补齐原字体缺字。

泡面仍28G、持续4个游戏小时；改为收线与放线速度提升10%，不再增加提竿窗口。热拿铁仍35G、持续6个游戏小时，收放线速度和提竿窗口各提升10%。两者仍共用状态，后购买的替换前一个，不叠加。满潮湿时泡面为88%基础收放线速度。已存档的泡面按新效果继续，剩余时间保持。

沿用V14存档（1503字节）及资源2026.10.19.5。只保存最终的装备所有权，损坏提示为临时状态；旧存档中已损坏的线组不会凭空恢复。不需重新复制SD资源包。

Each actual line break (overload or sustained range excess) makes one independent10% damage roll. Only damage removes the active rig, selects a spare and requests a save. The other90% retain the equipped rig for another cast. The fish is still lost. All four rigs and both maps use the same probability. Slack, early/missed strikes and loss-animation frames never make a damage roll. There is no guaranteed loss every tenth break.

A hooked puffer escaping adds an independent20% bite-through roll, with the exact hint “它咬断了线组”. Break escapes therefore have28% total damage probability; slack escapes have20%. At most one active rig is removed. Early/missed strikes and successful catches do not trigger biting.

Noodles remain28G for four game hours, now improving retrieval and payout by10% with no strike-window bonus. The35G six-hour latte retains10% bonuses to both handling and strike window. The latest refreshment replaces the previous one. Existing noodle timers retain their remaining duration with the new effect.

V14 saves and the existing2026.10.19.5 SD packs remain compatible. Previously lost rigs are not restored. The author approved publishing this source and firmware on2026-10-26. Device testing remains pending; no flashing was performed.
