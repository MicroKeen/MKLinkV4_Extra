# 容器、商店与游戏时间 v17

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

## 当前 D10：都市两处商家、专饵与热拿铁 / Current D10

当前以[都市两处商家与热拿铁](city-vendors-d10.md)及[内存核对](city-d10-memory.md)为准，D9以下保留为历史。硬件MKLink V4，本地开发，不推Git。

便利店保持在都市地图最左侧，女店员只卖热拿铁；带棚子的江边鱼摊位于右侧、靠近码头，壮硕男性摊主负责收鱼和出售渔具、鱼饵、容器。两处各自走近停车、短按进入，暂沿用09:00–21:00。

都市取消蚯蚓、米饭、面饵与鹅毛漂的出售，新增螺蛳（青鱼）、玉米（鲢/鳙/草）与栩栩如生的假饵（肉食、大个体）。普通等待间隔减15%；专饵屏蔽常见鱼后可能等更久。湿地旧商品、鱼饵、漂和原权重不变，旧库存不清除。

热拿铁35G，购买后立即饮用，6游戏小时内收放线速度与提竿窗口各+10%；再次购买刷新，不叠加；乘车扣除5游戏小时。HUD显示剩余时间，V11存档1495B保存新饵库存与剩余毫秒，原密钥、目录、装备初始化世代及所有旧档均保留。

程序Flash余14276B、地图缓存余19060B、DLM静态余3824B。商人立绘改6bit无损索引；湿地四表情8000像素逐像素核对，新增资源复用960B行缓冲，无整图解压RAM。必须使用资源2026.10.19.5。

City-only split: leftmost convenience store sells hot latte; a covered stall nearer the pier buys catches and sells tackle/bait. Wetland inventory/catalog stays intact. The saved six-hour latte buff grants10% faster retrieval/payout and10% wider strike window. Vendor portraits use palette packing without changing original wetland colours; no new full-image buffer. Matching2026.10.19.5 resources required.

## 最新 v29：路线与城市 / Latest v29: route and city

右岸车站现可短按进入路线图。“湿地湖泊”与“城市江河”之间以道路表示远距离，局部场景图随选中变清晰；旋转选择，短按乘车，长按600 ms返回。**每程180G、5个游戏小时，往返相同**，加载失败不扣费、不推进时间。城市采用选定的现代天际线、远处轮渡、气垫筏、独立窗灯/霓虹/倒影层和动态射灯，原有鱼种、装备与鱼库规则保留。

The east-bank station opens a schematic road route. Scene-backed anchors become clearer when selected. Rotate to choose, tap to travel, hold 600 ms to return. **Every trip costs 180G and five game hours in either direction**; failed loads do not charge or advance time. The city uses the selected skyline/ferry, raft, aligned emission layer and sweeping spotlights, with existing fish/tackle/cargo rules.

背景、世界与鱼类贴图改为SD资源包，切图时写入独立460 KiB Flash缓存；应用限制在0x80023000–0x80080000，缓存0x80080000–0x800F3000。先完整校验源文件，最后提交缓存头；路线页和加载人物留在固件。首次运行请先复制SD资源目录，原存档目录保留。v8独立A/B存档新增所在地，保留v1–v7与已购装备、声音、图鉴和鱼获。

World/fish art is loaded from SD packs into a dedicated 460 KiB Flash cache. Source preflight and a last-written commit header protect interrupted loading; route/loading art remains in firmware. Copy the SD resource directory before first boot and keep the original saves. Independent v8 A/B files add location while preserving v1–v7, gear, sound and catches.

完整部署、分区、协议与验证限制见 [地图与SD资源 / Maps and SD resources](maps-and-sd-resources.md)。下方带版本的章节为历史记录，以本节v29为准。


2026-10-14 根据用户文档实现。规则由平台无关 C 模块 `app/fish_economy.c`、`app/fish_game.c` 维护，JSON 契约位于 `data/economy.json`。Windows 模拟器、HPM 移植 overlay 和标准 `mklink_sample_fishing` 使用同一份规则。

## 鱼库与船尾

初始“小木桶”容量 **10,000 g / 10 kg**，可在商店购买大木桶（20 kg / 300 coins）和储鱼箱（35 kg / 750 coins），按各鱼种累计数量、总重量和成交价值；不因固定条目数量提前装满。容量内鱼获收入鱼库，恰好装满允许保存。单条超过当前容器总容量的鱼绕过容器放到船尾，限一条；船尾重量不计入鱼库容量。第二条巨鱼或容量内但无剩余空间的鱼会放流，仍计入捕获次数和图鉴最大尺寸，收鱼卡片提示去向。船尾复用已有鱼种贴图，以独立变换操作缩小绘制，避免与收鱼卡的变换互相覆盖。

行囊原通用格显示当前容器名称，初始为“小木桶”，旋转翻看 11 种鱼获和第 12 页船尾大鱼；短按返回。显示容量、种类、数量、重量和估价；商店也有鱼库入口。没有新增整屏缓冲或逐条堆分配。

## coins 与价格

文档中的 `6g / 8g` 等按货币 **coins** 解释，重量仍为 g。基础价为本种中等体型，非每克单价。价格公式：

`max(1, floor((base * size_mm² + typical_mm² / 2) / typical_mm²))`

`typical_mm = mean_x2_mm / 2`，与已有体型分布一致。平方长度让鲤鱼的上下价差较大，避免重量立方直接造成价格暴涨。未指定的趴地虎暂定白银档 16 coins，独立标记为设计假设。

| 鱼种 | 中等体型基础价 / coins |
|---|---:|
| 白条/白鲦 | 6 |
| 鲫鱼 | 8 |
| 鲤鱼 | 12 |
| 土鲶 | 12 |
| 溪蟹 | 10 |
| 马口 | 16 |
| 趴地虎 | 16 |
| 黄颡鱼 | 30 |
| 黑鱼 | 28 |
| 中华鳖 | 60 |
| 斑鳜 | 72 |

金币初始为 0，上限 1,000,000,000。出售操作一次结算鱼库及船尾全部鱼获，再清空库存；不清图鉴。若超过金币上限则整笔拒绝，避免先清鱼再截断收益。

## 左岸渔具店

左岸步行区域向西延伸到世界 x=-120，船和原水域坐标不变，相机可到 -144。木屋约在 -134，人物走到 x≤-84 时短按进入商店。走回西码头 x=96，再从行囊“登船出发”。商店内旋转选择商品，短按交易；“返回小径”退出。只有一颗编码器。

| 商品 | 价格 / coins | 交付 |
|---|---:|---|
| 玻纤竿 / 碳素竿 | 550 / 1800 | 永久解锁并装备 |
| 细线小钩 / 通用线钩 / 强韧大钩 | 120 / 200 / 450 | 装备，断线损坏后需补购 |
| 鹅毛漂 / 夜光漂 | 90 / 120 | 永久解锁并装备 |
| 蚯蚓 / 米饭 / 面饵 / 小鱼 / 虾饵 | 8 / 6 / 10 / 18 / 20 | 每包五份并装备 |

初始拥有竹竿、通用线钩、红白漂和普通假饵，假饵描述为“塑料味”，无限使用；塑料假饵追加到 bait index=5，原五种饵料索引保持。行囊只轮转已拥有的装备和有库存的特殊饵料。重复购买永久装备不扣钱；钱不足或饵料库存将超过 999 时整笔拒绝。特殊饵料每次抛竿消耗一份，即使随后收回；最后一份用完后，下次抛竿改用普通假饵。

老人按空手、普通鱼获、船尾巨鱼打招呼，并回应出售、购买、钱不足、已有装备及库存满。四个立绘分别用于招呼、赞许、交付、迟疑。最终采用用户选择的 **merchant-v3**，保留沧桑成年比例；未采用后续更阴郁版本。内饰改为用户要求的柜台近景。

## 时间与性能

平台提供单调系统毫秒：Windows `SDL_GetTicks()`、HPM `fish_hpm_millis()`。新游戏以 06:00 加系统运行秒数初始化，每 **1 秒现实时间 = 1 分钟游戏时间**，一昼夜 24 分钟。读取存档恢复游戏分钟，退出期间不快进。毫秒余量保留，uint32 回绕安全；绘图只消费 FrameStart 缓存快照，不在 PFB 内重新读钟。没有假定板卡具备电池 RTC，也没有读取日期或网络时间。

05:00–07:00 渐亮，18:00–20:00 渐暗。太阳和月亮沿抛物线移动，原背景中固定太阳及其光柱已移除；保留原图和新无太阳底图。环境用 Arm-2D RGB565 透明色叠加；天体反光仅 12 条短线，继续利用背景原有山林倒影。未增加整屏镜像采样、第二套日夜位图或水面逐像素着色器。商店与容器像素数据合计 57,820 B，均为只读区。

MCU 支持硬件浮点，但时间、价格、容量及路径仍用整数，只有 Arm-2D 公开变换使用既有 float 接口。性能判断以资源和绘制次数为限，尚未实测板端 FPS。

## 存档与移植

存档 v3 仍为 236 B，偏移200新增容器等级，保持原密钥、UUID 派生、目录和前 60 B 核心字段布局，新增金币、解锁、消耗品、鱼库、船尾和游戏分钟。可读取原 92 B v1：保留图鉴和所选装备，授予初始装备与原所选装备，原所选饵料赠五份；旧版没有鱼库，历史捕获不变成可出售库存，金币初始为 0。

迁移后先写另一槽并同步读回，再确认升级成功；失败保留原槽。新版本可以读 v1/v2/v3，旧固件不能读 v3；切换到旧版本前备份目录，切回本版本可恢复。不要用旧固件强制重置文件。具体协议见 [save-system.md](save-system.md)。

Android/微信适配只需传入单调毫秒与 ROTATE/PRESS/RELEASE 边沿，再用 Canvas 或平台绘制消费状态。规则无 SDL、Arm-2D、RT-Thread 依赖。JS 价格中间值远低于 2^53，整除用 `Math.floor`，毫秒差先 `>>>0`；所有价格、g/mm、uint32 和稳定数组索引见 JSON。平台持久化可复用小端 v3 编解码；UUID/密钥来源需由平台自行定义，不能声称跨设备密文直接可读。目前未创建 Android 或小程序运行工程。

新原图在 `assets/source/shop/` 和 `assets/source/lake-timebase-v2.png`，导出 PNG、图集与来源哈希在 `assets/processed/shop/`、`assets/shop-provenance.json`。实际提示词见 [shop-prompts.md](../assets/shop-prompts.md)，C 数组是派生产物。归档入口见 [cross-platform-assets.md](cross-platform-assets.md)。

商店采用五行滚动商品列表，选中后在底部查看价格与属性/描述；粗像素背景含壁灯和挂具，使用窄正面柜台。布局、扩容和v4兼容细节见 [bucket-and-display-rack.md](bucket-and-display-rack.md)。

当前余额在各主要页面持续显示；单鱼收获卡显示SELL售价，已解锁图鉴显示BEST纪录价，鱼库按实际存货紧凑分页。码头停稳后再次向岸旋转上岸、向水旋转登船；已取消行囊中的上下船按钮。完整当前规则以 [game-design.md](game-design.md) 为准。

## v26 金币反馈 / Coin feedback

购买成功显示 `-价格G`；出售成功按每条真实售价显示 `+售价G`，第一条立即出现，其后每250 ms一条，900 ms内上浮并淡出，最多四条。新鱼的售价记录随主存档保存；旧版只有合计的部分标明“旧鱼获合计”。交易立即结算，动画不阻塞旋转/购买，失败交易无扣款或收入浮字。完整规则见[整体设定](game-design.md)与[存档格式](save-system.md)。

Successful purchases show a negative amount, and sales emit one positive popup per fish every 250 ms. Popups rise/fade over 900 ms, with up to four visible. Exact new prices persist in the main save; aggregate-only legacy cargo is clearly labeled. Settlement is immediate and input remains responsive.
