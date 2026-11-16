# D27 全鱼种搏鱼平衡 / Whole-roster fight balance

MKLink V4，2026-11-02。原版使用高碳长竿＋八编PE，约40kg青鱼在30格／秒连续硬收时，64个种子全部能在平均0.8秒内钓起；10格／秒也全部成功。收线固定收益过高，八编同时提供高强度与低反馈，部分战斗在首次挣扎前结束。

## 统一修订 / Shared changes

- 所有水域按鱼重降低每格收线收益：原收益×100／[100＋min(整kg,40)×6]，仍保留鱼竿效率、疲劳、潮湿与增益差异。收益始终至少1，安全时任何剩余距离都能继续收线。
- 非被动鱼延长持续发力段：原时长×3，上限为周期减1000ms，不缩短已有更长段；小体型仍按自身拉力，保留600ms预警和鱼种周期差异。被动鱼不凭空增加突冲。
- 大于6.5kg的挣扎段张力目标额外传递50%冲击；八编响应45%→100%，舒适重量30→40kg，仍保留强度200、25m线长；海钓响应35%→80%，舒适重量60kg、强度240和200m线长保留。严重超配响应上限180%→220%，避免弱线对极端体型过早进入同一压力平台。
- 安全收线对大鱼确实收回鱼线时，清除松线计时；避免疲劳阶段低负荷反而在持续收线时自动脱钩。所有倍率计算结束后才做最小正进度保护，修正深水低收益被整数舍入成0的情况。
- 不改鱼种尺寸、重量、稀有度、价格、出现概率或装备售价；不加末段距离锁、体力准入或强制战斗时间。微小鱼仍适合轻装备，过强装备与长竿的小鱼劣势保留。

## 整体复核 / Coverage

覆盖三图40个可垂钓编号、44组地图／鱼种配置，包含条件变异和皇带鱼；每组下限、常见、上限体型，合法鱼竿／线组及实际线长组合，8个种子与7种输入策略。另对40kg青鱼和该鱼种上限70kg，各用64种子、四种兼容鱼竿与八编线组复现。

策略分别为每秒2／5／10／30格持续单向收线、即时控张力、延迟200ms控张力，以及挣扎／预警停收、平静期加快。按真实输入驱动；不读取隐藏拉力，不改鱼位置或跳过体力。慢收仍可能以更久时间换取较低风险；不把所有鱼强制做成必须反向操作的同一个模板。

成功数是固定种子的程序策略对比，不等于玩家胜率。极限鱤鱼等活跃鱼仍明显难于同重量的沉稳鱼。赶海9种生物继续使用各自QTE，与水中搏鱼分开。逐鱼种数值与策略对比见[数值和审计](fight-d27-audit.md)。

只更改C逻辑和数值表。V21的2147B存档、历史鱼获价格及2026.10.31.4资源保持；已安装该资源时只更新固件。实板手感仍需玩家验收。

D27 reviews all40 catchable IDs across44 region/species profiles. Shared mass drag reduces the excessive per-detent retrieval gain; longer species-specific exertion and stronger heavy-fish load make blind continuous reeling risky. PE/marine rigs retain their strength and reach advantages, while severe mismatches no longer flatten at the old response cap. Safe reeling always progresses, including deep water; large tired fish no longer trigger unavoidable slack losses while line is actually being retrieved. No final-distance, stamina or minimum-duration landing gate is added. Sizes, rarity, prices, saves and resource packs remain unchanged. The deterministic strategy audit is not a prediction of human win rates; hardware feel remains pending.
