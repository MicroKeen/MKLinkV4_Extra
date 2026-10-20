# 漂相、距离与搏鱼行为 v22 / Fishing behavior v22

## 当前 D12：存档、分区天气与线组 / Current D12

以[完整D12设定与修复记录](rain-depth-and-lines-d12.md)为准。V13仍1503B，独立A/B并保留旧档、原密钥和目录；保存岸/船位置、朝向、时间、天气、潮湿与拿铁，启动资源加载后恢复原位置。修复SD单扇区写入CRC、延迟响应和有界忙等待；主机验证不代替实机测试；作者已反馈当前版本基本稳定，掉电/拔卡等异常测试仍未专项验收。

两地天气独立，雨期2～48游戏小时；中近景长雨线增强景深，都市夜间/清晨保留向下延展的薄雾。都市码头增加台阶，船及水中人物再下移8像素，岸上人物略缩小；鱼摊缩小并在06:00–16:00绘出柜台后摊主，便利店仍09:00–21:00，路灯加高。

删除最后收线距离锁；青鱼和鱤鱼冲刺产生更明显真实位移，70%朝远离玩家方向。线组120/200/450G，都市新增600G八编PE线组，“像一条钢缆，会吓跑小鱼”。真正断线损坏当前线组，备件自动切换；金币不足120G且没有细线小钩即可免费补一副，无鱼获和其他线组条件。对应补给台词见两地店主台词.md。

V13 preserves outdoor position, clock and regional weather without increasing file size. The SD writer handles CRC16 and delayed responses with bounded waits. Independent2–48h rain, city geometry/mist, stronger giant surges and paid/free rig replacement are included. Existing2026.10.19.5 packs remain compatible. The author reports basic stability and approved publication; power-loss/card-removal and hardware peak tests remain outstanding. 以下版本章节为历史。

## 当前 D11：雨天、远景薄雾与潮湿 / Current D11

以[雨天与潮湿说明](rain-and-wetness-d11.md)为当前设定。两图加入自动降雨、远景薄雾、像素雨线及仅在水面出现的扩散小涟漪，前景主角、码头与夜光漂保持清楚。天气独立于钓鱼随机数，晴雨缓慢过渡。

持续淋雨60个游戏分钟达到100%潮湿，线性降低收放线速度，满值减20%；晴天或棚下120个游戏分钟完全晾干。热拿铁与潮湿相乘，满潮湿拿铁为88%基础收放线速度，提竿窗口仍+10%。坐车会推进天气且在车内晾干；加载与离线不计时。

潮湿与拿铁改为18×18状态小图标：图标背景内填充分别显示潮湿程度和拿铁剩余时间，没有长文字条或大底框。

雨天原始出现权重：鲤鱼110%、白条92%，湿地土鲶/黄颡115%，都市长吻鮠115%、翘嘴112%、鱤鱼110%。其他鱼、水层与饵料排除规则、体型以及条件1%稀有变异规则保留。

V12存档1503B，追加8B天气进度和潮湿状态，独立V12_A/B并保留全部旧档、道具、图鉴、地图与拿铁。沿用D10资源2026.10.19.5，已复制的包无需重拷，不新增位图或整屏RAM。

Both maps gain gradual rain, distant haze and water rings. Wetness fills in60game minutes and reduces retrieval/payout speed by up to20%; drying takes120game minutes. Compact status icons show wetness and remaining latte. Slight fish-weight changes retain habitat/bait exclusions and conditional1% variants. V12 adds8bytes; D10 map packs stay compatible. Local only, no publishing or hardware flashing.

有效数值及完整调参入口见[数值平衡指南](balance-tuning-guide.md)，鱼饵偏好见[鱼饵设定](bait-design.md)。

试饵/提竿上下界保留，各鱼种漂相、体重拉力和匿名爆发提示保留。上钩不显示鱼种图标。

“距离”显示鱼到船的水平距离（米，一位小数），世界标尺25px/m；非零距离向上取一位。鱼竿额定抛距为3.52、4.64、5.60米，实际抛距与预览还受4.5/6.5/8.5米线长及靠岸落点限制。最大出线后不能再放线；鱼可拉出至多10%的竿身弹性超距，弯竿持续2000 ms后断线，收回范围立即解除计时。每次爆发约70%远离玩家、30%靠近玩家，方向每次爆发只抽一次。详细数值与核实见[v22玩法与核实](v22-gameplay-and-audit.md)。水深继续独立筛选鱼种与钓层。

鱼体力只在安全张力下的主动挣扎或有效收线时消耗，静止休息不会持续扣体力。耗尽进入停歇，取消主动爆发、水花和双向冲刺；保留轻微水面起伏。回复周期与恢复阈值由`data/species.json`的`fishing_rules`统一维护。收线距离归零即收鱼，不要求体力先为零。松线、过载和持续弹性超距仍有风险。

失败后进入1200ms动画：错过提竿漂下沉，断线竿尖回弹，脱钩/过早提竿收回空钩；底部只显示“线断了”“脱钩了”“错过了”等简短结果。结束后恢复操作。完整控制器审计见[当前验证记录](v22-gameplay-and-audit.md)，不作为人类胜率。
