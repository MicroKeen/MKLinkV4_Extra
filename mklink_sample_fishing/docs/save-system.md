# SD卡存档 V13 / SD saves V13

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

## 当前 D7：鳙鱼与深河道 / Current D7

以本节为当前设定，后面D6及更早章节作为历史。2026-10-19，本地开发，不推Git，硬件MKLink V4。

白鲢：铜稀有度、仅表层、100cm/40kg；鳙鱼：白银、中下层、145cm/60kg，独立大头/斑驳立绘。红尾退出可钓种和正常图鉴，旧档记录、鱼获和售价保留，不转为鳙鱼。城市仍12种可钓，活动图鉴24项。评语沿用作者修改，按鱼名匹配稳定ID。

城市近岸采样点3.6/4.2米、江心16米；钓深0、0.5、2、4、6.5、9、12米和钓底，HUD/落点/鱼层使用独立剖面。湿地原地形、装备与图鉴定性钓层保持不变。

资源版本2026.10.19.2，两图均需更新。V10存档1485B，独立A/B，读取V1–V9，保留旧文件与装备；D6白鲢记录保留原130cm曲线和原售价。资源缓存剩12,936B；整包驻Flash，未新增整屏SRAM缓冲。详见 [D7设置与兼容](city-revision-d7.md) 和 [内存报告](city-d7-memory.md)。

Silver carp is bronze/surface-only at 100cm/40kg; bighead is silver/middle-lower at 145cm/60kg. Retired redtail remains compatible with historical saves. The city uses deeper banks and a 16m channel, with wetland tuning preserved. Matching 2026.10.19.2 packs and separate 1485-byte V10 saves are required; old saves/gear survive. Local only, no Git push.


## 当前 D6：城市内容 / Current D6 city content

2026-10-19，本地开发，**不推Git**。本节为当前实现，后面的版本记录为历史说明。硬件为MKLink V4。

城市江河实装12鱼种（新增9种，鲫鱼/鲤鱼/白条共享图鉴与素材、城市独立数值）；青鱼为白银，巨物不为旧装备削弱。湿地原规则保留。城市便利店09:00–21:00，湿地渔具店07:00–18:00，车站24小时。现代女店员有四个表情与14组双句台词；草地、公园码头、路灯与带雾感的都市昼夜远景已接入。翘嘴鲌/白鲢/红尾鲌跃水时更易因松线脱钩；东方鲀间歇鼓胀并磨损当前线强度，每次新中鱼恢复装备强度。

重量大于1000克显示一位小数kg（1450克→1.5 kg，70000克→70.0 kg），1000克及以下保留克；数值和售价不受显示取整影响。收鱼评语统一在 [catch-comments.md](catch-comments.md)，店员台词在 [city-merchant-dialogue.md](city-merchant-dialogue.md)，均可手改表格。中文保持12像素。

资源包必须与固件匹配 **2026.10.19.1**，两图都要更新。V9存档1470 B，追加9种鱼与来源地图字段，保留V1–V8文件、装备、声音、金币和鱼获。切回湿地仍能查看/售出城市鱼。缓存载荷449,976 B，剩余16,968 B；应用Flash与实测构建余量见本次验证报告。

D6 is local only. Twelve city fish use independent profiles while retaining shared collection IDs for returning species. The 09:00–21:00 convenience store has a four-expression clerk; wetland hours remain 07:00–18:00. Updated park art and atmospheric skyline are integrated. Weights above 1000 g use one decimal kg. Matching 2026.10.19.1 packs and separate 1470-byte V9 saves preserve legacy progress. See [D6 details](city-integration-d6.md) for deployment, migration and validation limits.

### V9 字节布局 / V9 layout

V8前缀1286 B保持字段偏移。1286–1411追加9个14 B记录（钓获次数2、最佳尺寸2、实物数量2、总克数4、总售价4）；1412–1435记录24项最佳尺寸的来源地图；1436船尾鱼来源，1437最近鱼获来源。头16 B＋认证标签16 B＋1438 B载荷＝1470 B。11–14仍为原稀有变异，15–23为城市新鱼，空船尾哨兵为24。旧档15哨兵读取后转换为24。

写入 `NorthWetland_V9_Save_A/B_DO_NOT_DELETE.sav`，读取优先V9→V8→V7→V6→V5→V1–V4。旧版本按真实长度认证后补齐字段，旧纪录来源默认为湿地；设备密钥与UUID派生不变，装备世代仍为1。PC对应 `progress_v9_0/1.sav`，旧文件只读迁移。

## 历史记录 / Earlier versions

## 最新 v29：路线与城市 / Latest v29: route and city

右岸车站现可短按进入路线图。“湿地湖泊”与“城市江河”之间以道路表示远距离，局部场景图随选中变清晰；旋转选择，短按乘车，长按600 ms返回。**每程180G、5个游戏小时，往返相同**，加载失败不扣费、不推进时间。城市采用选定的现代天际线、远处轮渡、气垫筏、独立窗灯/霓虹/倒影层和动态射灯，原有鱼种、装备与鱼库规则保留。

The east-bank station opens a schematic road route. Scene-backed anchors become clearer when selected. Rotate to choose, tap to travel, hold 600 ms to return. **Every trip costs 180G and five game hours in either direction**; failed loads do not charge or advance time. The city uses the selected skyline/ferry, raft, aligned emission layer and sweeping spotlights, with existing fish/tackle/cargo rules.

背景、世界与鱼类贴图改为SD资源包，切图时写入独立460 KiB Flash缓存；应用限制在0x80023000–0x80080000，缓存0x80080000–0x800F3000。先完整校验源文件，最后提交缓存头；路线页和加载人物留在固件。首次运行请先复制SD资源目录，原存档目录保留。v8独立A/B存档新增所在地，保留v1–v7与已购装备、声音、图鉴和鱼获。

World/fish art is loaded from SD packs into a dedicated 460 KiB Flash cache. Source preflight and a last-written commit header protect interrupted loading; route/loading art remains in firmware. Copy the SD resource directory before first boot and keep the original saves. Independent v8 A/B files add location while preserving v1–v7, gear, sound and catches.

完整部署、分区、协议与验证限制见 [地图与SD资源 / Maps and SD resources](maps-and-sd-resources.md)。下方带版本的章节为历史记录，以本节v29为准。


当前保存15个收藏项（11种基础鱼与4种稀有变异）的次数和最佳尺寸、最近鱼获、装备与所有权、特殊饵料库存、金币、鱼库、船尾鱼、容器和游戏时间。临时抛竿、搏鱼、步行动画、进店线索不存档。

写入v7，每槽1317字节。继续读取v1（92B）、v2/v3/v4（236B）、v5（292B）和v6（293B）；原密钥、设备UUID派生与SD目录保持不变。V4及完成初始化的装备不再重置。没有初始化标记的v1/v2/v3，沿用旧版本的一次装备初始化规则：保留图鉴、金币、库存、鱼获、容器和时间，把竿/线组/漂及所有权恢复初始，饵料恢复普通假饵、钓深到底。

## 文件和固件切换

```text
FishingGame_Saves_DO_NOT_DELETE/
  NorthWetland_V7_Save_A_DO_NOT_DELETE.sav
  NorthWetland_V7_Save_B_DO_NOT_DELETE.sav
  NorthWetland_V6_Save_A_DO_NOT_DELETE.sav
  NorthWetland_V6_Save_B_DO_NOT_DELETE.sav
  NorthWetland_V5_Save_A_DO_NOT_DELETE.sav
  NorthWetland_V5_Save_B_DO_NOT_DELETE.sav
  NorthWetland_Save_A_DO_NOT_DELETE.sav       原文件，只读迁移
  NorthWetland_Save_B_DO_NOT_DELETE.sav       原文件，只读迁移
  README_V7_DO_NOT_DELETE.txt
```

板卡逻辑盘为3:。V7只写自己的A/B槽，V6、V5与原文件全部只读保留，优先级为V7 > V6 > V5 > V1–V4；只在同一格式优先级内比较序号。有效V7不被旧固件写出的更高序号替代。首次迁移写新槽、同步、关闭和读回成功后，原文件仍保留。

PC使用SDL用户目录，可由FISH_SAVE_DIR覆盖。当前为progress_v7_0.sav/progress_v7_1.sav；progress_v6_0/1.sav、progress_v5_0/1.sav和progress0/1.sav只读。PC与板卡UUID独立，不能互换。切到下载器不会清理存档，切回旧固件使用它自己的旧进度，两条时间线不自动合并；建议备份整个目录到同一板卡恢复。

V7 writes separate A/B files and preserves every v6/v5/original file. Loading prefers v7, then v6, v5 and earlier formats; sequence numbers are compared only within the same format priority. Firmware switching cannot overwrite newer-format progress. Back up the entire directory; PC and board saves use different device identifiers.

## 格式

头16B（FSG1、版本7、载荷长度1285、64位序号）＋认证标签16B＋密文1285B＝1317B。字段显式小端编码，不直接复制C结构体。前204B保留基础字段位置：

| 偏移 | B | 内容 |
|---:|---:|---|
| 0 | 60 | 装备、最近鱼获、总捕获数与原11种图鉴 |
| 60 | 4 | coins |
| 64 | 3 | 竿/线钩/漂所有权 |
| 67 | 10 | 五种特殊饵料库存 |
| 77 | 110 | 原11种鱼库count:uint16、grams:uint32、value:uint32 |
| 187 | 1 | 船尾鱼索引，V7的15表示空；旧版11在迁移时转换 |
| 188 | 2 | 船尾尺寸 |
| 190 | 4 | 船尾重量 |
| 194 | 4 | 船尾售价 |
| 198 | 2 | 游戏分钟0–1439 |
| 200 | 1 | 容器等级0/1/2 |
| 201 | 1 | 装备初始化世代，仍为1 |
| 202 | 2 | 保留零 |
| 204 | 56 | 四种稀有鱼各14B：图鉴次数2、最佳尺寸2、实物数量2、总重量4、总价值4 |
| 260 | 1 | 声音：0关闭、1仅鱼讯、2全部开启 |
| 261 | 1024 | 256组price:uint16、count:uint16；空组为零，同价合并，逐鱼显示 |

索引11–14依次为黄化土鲶、长鳍马口、红色长尾鲤、白化鳖。稀有鱼独立记录，出售值为同尺寸基础鱼完整计算并取整后的10倍。通过认证后，检查索引、尺寸、计数、装备所有权、库存、容量、价值、船尾重量和时间，全部通过才应用状态。

旧版本按其真实长度和版本头认证，不能修改版本号冒充新格式。V4已完成装备初始化，迁移不再清空高级装备；先前V4及以后有效初始化标记也可避免旧V3记录导致重复初始化。

## 写入和恢复

行囊底部选“存档”并短按保存；每五分钟自动检查变化，没有变化不写。保存请求复制固定快照交给后台线程；显示“存档中”直到写入、同步、关闭、读回一致。400ms只是状态最短显示时间，不代表加密耗时。完成后“已存档”约2.5秒。

V7始终写非活动槽，不先删除或重命名有效文件。成功后才更新活动槽、序号和已保存快照；写到一半或认证失败时可回退另一槽。若无有效V7但旧版文件有效，可恢复原版本进度。两个体系都没有有效记录或有IO错误时，不静默覆盖异常文件；手动保存也不会强制清掉损坏档。保存期间的额外请求会合并排队。

HPM5301用原SD SPI1和4KiB RT-Thread工作栈，LCD保持原SPI0/DMA和PFB链路；存档回调不操作背光PA26。单鱼售价表相对v6新增1024B，完整文件1317B；复用原加密模块，没有引入整屏缓冲或新加密库。工作线程读回缓冲放在固定静态区，避免增大4 KiB线程栈负担。启动时只验证候选载荷，不在栈上复制整份游戏状态。

## 认证和验证

使用[RFC5297 AES-SIV-CMAC-256](https://www.rfc-editor.org/rfc/rfc5297)，原TinyCrypt小型子集、原项目密钥及OTP UUID派生保持不变。UUID全零/全FF拒绝初始化。按作者要求密钥已公开，源码持有者可以伪造存档；这里的认证用于检查文件完整性与设备匹配。

测试覆盖真实92/236/292/293B历史文件、V7全部字节修改/截断拒绝、稀有图鉴与库存往返、原装备保留、旧文件高序号不覆盖V7、PC文件同步/读回/重启/截断恢复和原文件不变。嵌入式GNU/SEGGER构建及分区结果见[validation.md](validation.md)。尚未做实板掉电、SD拔插、帧率或栈水位测量。

## 单鱼售价兼容 / Individual price compatibility

新捕获鱼的真实售价按相同价格合并计数，继续保留各物种的原数量、重量与总价。存档校验检查记录连续、价格不重复、数量及价格合计不超出鱼库，所有检查通过才应用。船尾鱼已有准确尺寸与售价，无需重复存表；转入容器时加入售价表。

旧v1–v6没有单鱼明细，读取后表为空，总价不变。出售时缺失明细显示一次“旧鱼获合计”，新鱼和船尾鱼逐条显示。浮字队列、上浮时间和淡出状态不存档，重启不重复交易。v1–v5声音默认为全部开启；v6原声音档位保留。

The additional 1024-byte table groups equal exact prices with counts. Validation rejects holes, duplicate prices and counts/values exceeding cargo totals before changing game state. Legacy v1–v6 species totals are preserved without inventing individual prices: their missing detail appears once as a labeled legacy total. New catches and the stern fish remain exact. Popup state is transient and never replays a transaction on restart; v6 sound settings are retained.
