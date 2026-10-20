# 北方湿地公园：整体设定 v31 / North Wetland: Game Design v31

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

## 当前 D9：近景、鱼库、大鱼与写盘 / Current D9

以本节及[D9说明](foreground-fight-save-d9.md)为最新实现；D7/D6及以下为历史记录。MKLink V4，本地开发，不推Git。

湿地与都市船上复用同一人物像素、2×2屏幕颗粒；岸上保留三帧36px站姿。整套前景下移8px：脚点142、水面143，船底/码头柱脚加入接水涟漪，远景与HUD不动。夜间路面压暗，灯光仍用生成式像素图和独立透明层。鱼库恢复原图鉴框，容器仅作为上方小图标，鱼获、售价、金币和页码保留。

超过6.5kg先将体力遛至20%以下再完全收起；大体型慢拉增强压力与拉线，超过20kg有额外巨物负荷。13kg长吻鮠+竹竿+强韧大钩的固定控线脚本由5.7s变为13.1s，仍保持贴底慢拉，不改为跃水。小鱼保留原收起规则。

S07/01发生在存档打开阶段的磁盘I/O。FatFs改为逐扇区CMD24、写期间4MHz，结束恢复读取频率；该兼容修正实机尚待确认。原A/B及旧档保护继续有效，无格式化或修盘。资源更新为2026.10.19.4；V10仍1485B。DLM余3,824B、资源缓存余12,936B，详见[当前内存](city-d9-memory.md)。

D9 is current: common player pixels, an 8px lower near plane and water contact, darker roads, collection-style cargo cards, large-fish stamina control, and CMD24/4MHz write compatibility for S07/01. Physical-device storage validation remains pending. Require 2026.10.19.4 packs; preserve existing saves and downloader files.

## D7：鳙鱼与深河道 / D7 history

本节记录D7设定，当前以D9为准。2026-10-19，本地开发，不推Git，硬件MKLink V4。

白鲢：铜稀有度、仅表层、100cm/40kg；鳙鱼：白银、中下层、145cm/60kg，独立大头/斑驳立绘。红尾退出可钓种和正常图鉴，旧档记录、鱼获和售价保留，不转为鳙鱼。城市仍12种可钓，活动图鉴24项。评语沿用作者修改，按鱼名匹配稳定ID。

城市近岸采样点3.6/4.2米、江心16米；钓深0、0.5、2、4、6.5、9、12米和钓底，HUD/落点/鱼层使用独立剖面。湿地原地形、装备与图鉴定性钓层保持不变。

资源版本2026.10.19.2，两图均需更新。V10存档1485B，独立A/B，读取V1–V9，保留旧文件与装备；D6白鲢记录保留原130cm曲线和原售价。资源缓存剩12,936B；整包驻Flash，未新增整屏SRAM缓冲。详见 [D7设置与兼容](city-revision-d7.md) 和 [内存报告](city-d7-memory.md)。

Silver carp is bronze/surface-only at 100cm/40kg; bighead is silver/middle-lower at 145cm/60kg. Retired redtail remains compatible with historical saves. The city uses deeper banks and a 16m channel, with wetland tuning preserved. Matching 2026.10.19.2 packs and separate 1485-byte V10 saves are required; old saves/gear survive. Local only, no Git push.


## D6：城市内容 / D6 city content history

2026-10-19，本地开发，**不推Git**。本节为当前实现，后面的版本记录为历史说明。硬件为MKLink V4。

城市江河实装12鱼种（新增9种，鲫鱼/鲤鱼/白条共享图鉴与素材、城市独立数值）；青鱼为白银，巨物不为旧装备削弱。湿地原规则保留。城市便利店09:00–21:00，湿地渔具店07:00–18:00，车站24小时。现代女店员有四个表情与14组双句台词；草地、公园码头、路灯与带雾感的都市昼夜远景已接入。翘嘴鲌/白鲢/红尾鲌跃水时更易因松线脱钩；东方鲀间歇鼓胀并磨损当前线强度，每次新中鱼恢复装备强度。

重量大于1000克显示一位小数kg（1450克→1.5 kg，70000克→70.0 kg），1000克及以下保留克；数值和售价不受显示取整影响。收鱼评语统一在 [catch-comments.md](catch-comments.md)，店员台词在 [city-merchant-dialogue.md](city-merchant-dialogue.md)，均可手改表格。中文保持12像素。

资源包必须与固件匹配 **2026.10.19.1**，两图都要更新。V9存档1470 B，追加9种鱼与来源地图字段，保留V1–V8文件、装备、声音、金币和鱼获。切回湿地仍能查看/售出城市鱼。缓存载荷449,976 B，剩余16,968 B；应用Flash与实测构建余量见本次验证报告。

D6 is local only. Twelve city fish use independent profiles while retaining shared collection IDs for returning species. The 09:00–21:00 convenience store has a four-expression clerk; wetland hours remain 07:00–18:00. Updated park art and atmospheric skyline are integrated. Weights above 1000 g use one decimal kg. Matching 2026.10.19.1 packs and separate 1470-byte V9 saves preserve legacy progress. See [D6 details](city-integration-d6.md) for deployment, migration and validation limits.



便利店右侧路灯使用ImageGen像素灯体和独立暖黄透明光层，18:00亮起、06:00熄灭；人物保持在光层前面。鱼库24项按实际透明轮廓居中，限制160×56展示区；实体地面与店底、码头衔接，月亮先于近岸建筑绘制。具体资源和验证见D6说明。

The park lamppost uses generated pixel sprites and an independent soft-alpha light texture, enabled from 18:00 to 06:00. Cargo portraits fit their visible bounds; shop/pier grounding and foreground moon occlusion are corrected.

## 历史记录 / Earlier versions

2026-10-19 D4：作者确认D3已成功加载。进度条上方换回原坐船钓鱼图标，移除行走小人与额外鱼竿；复用固件素材，无需更新资源包，仍不推Git。详见 [D4加载图标](loading-boat-d4.md)。

D4 follows the user-confirmed D3 boot and reuses the original boat/fisherman icon above the progress bar. No new artwork or resource-pack copy is needed. Distribution remains local, without Git push.

2026-10-19 本地启动修正：正常启动取消额外1.5秒停留，有效地图缓存跳过NOR重配置，缓存CRC改为16 KiB一轮；真正写入前才初始化，并补齐RAM返回XIP前的指令同步。资源版本与文件不变，仍不推Git。最新说明与验证范围见 [启动修正](startup-recovery-20261019.md)。

Local startup fix (2026-10-19): no added normal-boot delay, no NOR reconfiguration for a valid cache, and 16 KiB cached-CRC steps. Writer setup is deferred until a validated replacement needs writing; the RAM wrapper synchronises instructions before XIP return. Packs are unchanged, hardware acceptance remains pending, and no Git push is authorised. See the startup-fix guide.

## 97% 附近复位修正（本地） / Local fix for reset near 97%

2026-10-17，作者报告资源检查到“校验城市资源”97% 附近后复位，屏幕显示看门狗标记。该标记说明**上次启动前发生过看门狗复位**，不是当前进度到 97% 才触发的实时报警；现改为“上次为看门狗重启”。

检查发现：两包只读检查结束后会开始配置 NOR 缓存接口，原 `rom_xpi_nor_auto_config` 调用位于 XIP 代码，且中断仍开启。现把配置调用及返回路径移入 ILM，并在 ROM 调用期间屏蔽中断；配置和擦写前后均刷新看门狗，主循环刷新也统一走原子接口。Flash 配置和写入中仍保留看门狗超时，不用中断定时器持续掩盖卡死。

启动检查完成后显示 100% 1.5 秒，下一步先显示“正在前往”目的地，再在下一帧配置缓存，避免检查页最后一张 97% 画面掩盖阶段切换。资源版本、SD 文件、分区和存档无需改变。

本地固件为 `NorthWetland-cache-init-fix-2026.10.17.1-local.uf2`。58 项根工程和 21 项独立 demo 检查通过，包括编译实际 NOR 端口，验证配置/擦写期间的中断保护、喂狗边界、失败返回及缓存越界拦截。GNU ELF 另验证配置函数 `0x628` / 90 B 与写入函数 `0x682` / 124 B 均在 ILM。应用使用 371,712 B、余量 9,216 B；DLM 126,480 B，未增加帧缓冲。

这确认并修正了一处代码缺陷，**尚不能证明它就是该次实机复位的唯一原因**；新固件仍需实机验收。旧测试固件/报告保留，未对用户设备写入或烧录，未推 Git。详细证据见 `cache-init-fix-build-validation.json`。

The author reported a reset near 97% on the city-pack verification page. The watchdog banner describes the previous reset, not a live alarm at that percentage. NOR auto-configuration following the two read-only checks was running from XIP with IRQs enabled. It now executes and returns through ILM with IRQs masked; configuration and writes refresh the watchdog at their boundaries. The original watchdog remains enabled, with no periodic interrupt feed hiding a stall.

Boot verification shows 100% for 1.5 seconds and renders the destination before initialising the cache on the following frame. Packs, SD layout and saves are unchanged. All 58 root and 21 standalone checks pass, including the actual NOR port's critical sections, heartbeat boundaries, failure paths and cache bounds. ELF checks confirm both configuration and write helpers in ILM. App Flash is 371,712 B with 9,216 B free; DLM is unchanged at 126,480 B. This fixes a confirmed code defect; the observed on-board root cause remains unproven until the new local firmware is tested. No user-device writes/flashing or Git push occurred.

## 缺包自动导入 / Automatic resource recovery

资源版本 **2026.10.17.1**。按作者确认保留原下载器整卡 USB 方式，当前 `import-progress` 版本修正扇区数与 CPU SPI 访问，开放前检查原文件系统和容量/分区边界。缺包时显示磁盘，复制完整资源并只读校验后隐藏；电脑与游戏/存档独占交接，不重建 FAT、不改分区、不删除原文件。旧错误版保持撤回，近期不推 Git。详见[兼容说明](resource-import.md)。

Resource version **2026.10.17.1** remains unchanged. The local import-progress build restores stock direct SD access with corrected geometry/CPU SPI pointers and existing-volume checks. Exclusive host/game ownership preserves conditional import and automatic detach without repartitioning, formatting or deleting original files. The original bad build remains withdrawn. No Git push; see the compatibility guide.

## 城市鱼种扩展（开发中） / City fish expansion (in development)

城市新增鱼按作者的 `river_fish_data.csv` 设定。鲫鱼、鲤鱼、白条与湿地共享素材和图鉴，城市使用新表数值，湿地保留原数值。**新图巨物不为旧装备降低体型、力量或耐力；旧装备允许钓不上来，后续加入城市新装备。** 本阶段仍在本地开发，暂不推送；素材改稿、数值与存档集成状态见[城市江河开发记录](river-v30-worklog.md)。

City fish follow the author's CSV. Reused species share art and collection entries with separate city/wetland profiles. **Giants are not weakened to suit old tackle: existing equipment may be unable to land them; city equipment will be added later.** This expansion remains local and in development; see the linked development notes for integration status.

青鱼稀有度调整为**白银**，全长上限 170 cm、重量上限 70 kg，力量及立绘保持现有设定；项目内城市鱼种 CSV 已同步。

Black carp is **silver-tier**, retaining its 170 cm / 70 kg limits, strength and selected artwork. The local city-fish CSV reflects this change.

本地导入进度版：复制显示收到的 KB 和活动条，校验显示连续进度、当前地图及耗时；缺文件和版本错误有具体提示。SD 事务约 2 秒预算、SPI 调用 100 ms 超时，原 10 秒看门狗保持开启，在事务边界安全喂狗，启动捕获可用复位标记。实机卡顿/重启根因仍待确认。资源页改为坐船垂钓的主角图标：48×40、整数 2 倍、1,266 B ROM，缺包也能显示；无新增帧缓冲，资源包版本不变。

The local import-progress build shows received KB/activity while copying and linear verification progress, map and elapsed time, with specific path/version errors. SD transactions have a roughly 2 s budget, SPI calls a 100 ms timeout, and atomic boundary refreshes retain the original 10 s watchdog. Available reset flags are captured at boot; the observed hardware cause remains unconfirmed. A 48×40 fishing-boat protagonist icon replaces the old symbol at integer 2×, using 1,266 ROM bytes without SD dependency or a framebuffer. Resource packs are unchanged.

## 最新 v29：路线与城市 / Latest v29: route and city

本地后续调整（暂不推送）：车站全天 24 小时可进入，夜间候车棚和售票窗口常亮暖灯。车站纸质线路图保留 640×480 虚拟画布，当前两处地点靠近排列并完整同屏，旋转选站不移动镜头；以后地点放不下再启用平移。地点配图裁取代表景物并扩大为 64×32，选中全色，未选中 alpha 192，中文仍为 12 像素。底图另以淡色地图符号表现河流、小桥、山群、林地、池塘、芦苇和街区。大图缓存不增加，视窗状态仍为 16 B。详情见[地图与SD资源](maps-and-sd-resources.md)。

Local follow-up, not pushed: both stations open 24 hours with warm night lamps. The paper station diagram retains a 640×480 virtual canvas, but both current stops fit together and selection keeps the overview still. Panning is retained for a future network that no longer fits. Cropped 64×32 scene illustrations remain recognizable in both full-color selected and alpha-192 unselected states, with unchanged 12-pixel Chinese labels. Pale cartographic river/bridge, mountain, forest, pond, reed and city-block symbols enrich the paper behind stop content. Camera state remains 16 bytes without a larger framebuffer.

右岸车站现可短按进入路线图。“湿地湖泊”与“城市江河”之间以道路表示远距离，局部场景图随选中变清晰；旋转选择，短按乘车，长按600 ms返回。**每程180G、5个游戏小时，往返相同**，加载失败不扣费、不推进时间。城市采用选定的现代天际线、远处轮渡、气垫筏、独立窗灯/霓虹/倒影层和动态射灯，原有鱼种、装备与鱼库规则保留。

The east-bank station opens a schematic road route. Scene-backed anchors become clearer when selected. Rotate to choose, tap to travel, hold 600 ms to return. **Every trip costs 180G and five game hours in either direction**; failed loads do not charge or advance time. The city uses the selected skyline/ferry, raft, aligned emission layer and sweeping spotlights, with existing fish/tackle/cargo rules.

背景、世界与鱼类贴图改为SD资源包，切图时写入独立460 KiB Flash缓存；应用限制在0x80023000–0x80080000，缓存0x80080000–0x800F3000。先完整校验源文件，最后提交缓存头；路线页和加载人物留在固件。首次运行请先复制SD资源目录，原存档目录保留。v8独立A/B存档新增所在地，保留v1–v7与已购装备、声音、图鉴和鱼获。

World/fish art is loaded from SD packs into a dedicated 460 KiB Flash cache. Source preflight and a last-written commit header protect interrupted loading; route/loading art remains in firmware. Copy the SD resource directory before first boot and keep the original saves. Independent v8 A/B files add location while preserving v1–v7, gear, sound and catches.

完整部署、分区、协议与验证限制见 [地图与SD资源 / Maps and SD resources](maps-and-sd-resources.md)。下方带版本的章节为历史记录，以本节v29为准。


2026-10-17。适用 Windows 模拟器与标准 `mklink_sample_fishing`，以下为当前实现，价格和规则覆盖旧版说明。

## 功能概述 / Feature overview

硬件平台为 **MKLink V4**：HPM5301、240×240 RGB565 ST7789屏幕、单颗可按下的双向旋转编码器。Windows模拟器用于开发和验证同一份游戏规则。

The hardware platform is **MKLink V4**: HPM5301, a 240×240 RGB565 ST7789 display and one rotary encoder with a push button. The Windows simulator develops and validates the same game rules.

1. **鱼饵与道具 / Bait and tackle**：六种鱼饵对11种基础鱼使用不同吸引权重，结合水域、钓层和时段选鱼；线钩影响等待/搏鱼，鹅毛漂延长提竿窗口，夜光漂提高夜间咬钩概率。

   Six bait types have distinct attraction weights for the 11 base species, combined with habitat, fishing layer and time. Line/hook sets affect waiting/fighting, the feather float extends the hook window, and the luminous float improves nighttime bite chance.
2. **时间与昼夜 / Time and day–night cycle**：现实1秒为游戏1分钟，完整昼夜24分钟，晨昏渐变并影响鱼种活跃度；保存游戏时间，退出期间不快进。

   One real second equals one game minute; a full day lasts 24 real minutes. Dawn/dusk transitions affect species activity. Game time is saved, with no offline advancement.
3. **经济、商店、船上鱼库 / Economy, shop and boat storage**：左侧码头上岸沿路访问渔具店，07:00–18:00营业；卖鱼换coins并购买装备、饵料和容器。初始小木桶10 kg，可升级20 kg大木桶和35 kg储鱼箱。

   Disembark at the left dock and follow the path to the tackle shop, open 07:00–18:00. Sell fish for coins and buy tackle, bait and containers. The initial 10 kg bucket upgrades to a 20 kg bucket or a 35 kg fish box.
4. **稀有变异 / Rare variants**：黄化土鲶、长鳍马口、红色长尾鲤、白化鳖，在选中对应基础鱼后独立抽取1%；彩色外框、独立收藏/库存、普通计算价格×10，并有商店专属对白。

   Xanthic catfish, longfin hooksnout minnow, red longtail carp and albino softshell each have a conditional 1% chance after their base species is selected. They have rainbow frames, separate collection/storage records, ten times the ordinary calculated price and special shop dialogue.

## 设备与操作

240×240 RGB565，单颗双向旋转、可按下的编码器。旋转移动/调整，短按打开行囊或确认；长按280 ms开始蓄力、1.4 s满蓄力，松开抛竿。漂相进入可提竿窗口后按下，搏鱼时顺转收线、逆转泄力。成功收鱼才揭晓鱼种、尺寸、估算重量、星级和评语。

背包类别顺序为鱼竿、线钩、浮漂、鱼饵、钓深、储鱼容器，随后是图鉴、存档、返回。底部菜单按屏幕左到右切换，原菜单和装备索引不重排。金币图标及余额在湖面、岸上、行囊、商店、图鉴、鱼获卡和容器页持续显示，最大余额1,000,000,000。只选择已拥有的装备与有库存的特殊饵料。

## 地图、背景与场景

水域x=56–904，船中心132–828，左右码头可上下船；13节点虚拟水深0–6m，五分区为芦苇浅湾、开阔水面、泥底深槽、缓流入口、浅岸砾石。左岸延伸到x=-284，商店入口x=-260，入口±16像素短按进入。船与水域碰撞坐标不变，岸上相机可到-336，经过小路/码头世界x=0时连续移动。

原湖景的山、森林、岸、水面保留，夜空在原山林轮廓上方转为接近黑的深蓝，少量星星和圆润月牙随原视差移动；日月被远山和树冠遮挡。生成小路使用同样2像素颗粒，和西岸重叠24像素，码头在上层盖接；外屋也用2倍粗像素。渔具店为柜台近景与放大的老人上半身，保留四种表情和14种情境各两句台词，老人位于柜台后方，原柜台前沿遮挡人物下缘；左侧显示五行滚动商品列表，旋转浏览16个稳定条目，列表保留文字与选中高亮，底部左下角以2倍整数缩放显示选中装备图标，右侧按名称/售价、描述、实际效果分三行显示详情。重绘粗像素背景，壁灯、线轴和浮漂挂在墙上，柜台只有窄台沿与正面木板，没有深斜桌面。

## 鱼种、钓层与活跃时间

保留11种鱼。按落点、水深、钓层、分区、饵料和游戏时间筛选权重；趴地虎、溪蟹仅近岸底层，土鲶仅底层。默认钓到底，行囊可选表层、0.5/1/2/3/4.5/6m。铜/银/金70/25/5为基础比例，实际局部比例随条件变化；卡面稀有度不变，体型星级按物种独立评级。

活跃度是相对出现权重的百分比，100为基础值；这是虚构湿地的游戏数值，不是自然调查结论。5–7点与18–20点按小时线性插值，6点/19点达到晨昏系数，7–18点使用白天系数，20–5点使用夜间系数。午夜连续，非活跃时段仍有机会；不突破水域/钓层/饵料限制。**斑鳜没有夜间偏好：白天和夜间都是100，只在晨昏轻微增加。**

| 鱼种 | 白天 | 晨昏峰值 | 夜间 |
|---|---:|---:|---:|
| 鲫鱼 | 100 | 125 | 90 |
| 鲤鱼 | 100 | 130 | 85 |
| 马口鱼 | 130 | 115 | 60 |
| 土鲶 | 65 | 125 | 155 |
| 子陵吻虾虎鱼（趴地虎） | 115 | 110 | 80 |
| 黑鱼（乌鳢） | 125 | 130 | 65 |
| 黄颡鱼 | 65 | 130 | 155 |
| 䱗（餐条／白条） | 135 | 110 | 55 |
| 斑鳜 | 100 | 115 | 100 |
| 中华鳖 | 85 | 110 | 130 |
| 溪蟹 | 70 | 125 | 145 |

先用环境基础权重乘时间系数，再进行原来的物种/体型抽样。切换时间不改变星级阈值、体型范围、重量公式或售价公式；当前一竿的候选在抛竿时确定，等待期间经过时段边界不重新选鱼。活动表保存在 `data/species.json` 的 `game_distribution.activity_percent`。

## 等待、夜光漂与搏鱼

基础咬钩轮次由2600改为2860 ms，最多22轮，最久62,920 ms。按线钩隐蔽性和候选鱼重量抽取实际等待，保留原900 ms试饵阶段；一般等待比旧版略长约10%。

夜光漂夜间发明亮蓝绿色光，环境压暗后绘制，附轻微光晕，只显示露出水面的部分；白天维持原色。完全入夜时每轮真实咬钩概率相对提高15%，例如70%→80%（整数向下取整），上限100%，晨昏随暗度渐变。这个加成缩短同等条件下的等待，不改变鱼种权重或提竿窗口。鹅毛漂独立把线组修正后的可提竿窗口延长15%，试饵延迟不变，白天和夜间都有效；红白漂没有额外加成。

搏鱼仍以20 ms整数步进更新张力、松线、距离、爆发与体力；竿影响抛距/收线，线钩影响强度/距离/窗口/隐蔽性。v22线长为细线4.5米、通用6.5米、强韧8.5米，抛竿/预览同步受线长限制；最大出线后不能继续放线。鱼冲刺可产生10%以内的竿身弹性超距，竿形随之弯曲；持续超距2000 ms断线，收回标称范围立即解除计时。每次爆发约70%远离玩家、30%靠近玩家；鲤鱼、马口、斑鳜位移系数135%、140%、130%，其他基础鱼100%，变异继承基础鱼。尺寸、售价、星级和变异概率不变。

Line lengths are 4.5/6.5/8.5 m for fine/general/heavy tackle, limiting both actual casts and previews. Manual payout stops at nominal length. Fish surges can use up to 10% elastic rod excess; continuous excess for 2,000 ms breaks the line, while retrieving into nominal range clears the timer. Each burst is about 70% away from the player and 30% toward the player, with movement factors of 135%/140%/130% for carp/hooksnout minnow/mandarin fish and 100% for other base species. Variants inherit their parents. Size, prices, stars and rare probabilities are unchanged.

## 鱼获容器与船尾

初始小木桶10 kg，可升级大木桶20 kg和储鱼箱35 kg。按鱼种汇总数量、重量和估价，容量内恰好装满有效；单条超过当前总容量才挂船尾，限一条。普通鱼没有剩余空间时放流，仍计图鉴；第二条超大鱼放流。升级保留所有鱼获，船尾鱼若可放入新增剩余空间则自动收纳。

鱼获页用压暗容器作背景，鱼图在前景，鱼名/数量/重量/价值放在独立暗色底板，避免纹理干扰。仅显示当前存有鱼获的鱼种，按稳定鱼种索引紧凑排序；船尾有鱼时追加最后一页，没有鱼获时显示统一空库提示，页码随实际页数变化。大鱼按朝向约30°斜挂，绳索从船尾固定点连接鱼头/鳃部，左右朝向对称。

## 货币与商店

营业时间07:00 AM–06:00 PM（07:00–18:00），外屋正面亮灯/熄灯两种状态表示营业，不再挂牌。尝试进入关门商店时，主角上方显示“打烊了”1.6秒。打烊无人且无法进入或交易，18点退出店内，查看容器后也不能返回已打烊的店；不增加打烊对话。店主台词与表情可在 `docs/merchant-dialogue.md` 修改，装备描述草稿在 `docs/tackle-descriptions.md`。

币重分开：coins为货币，g/kg为重量。单鱼售价为 `max(1, floor((base * size_mm² + typical_mm²/2) / typical_mm²))`，`typical_mm=mean_x2_mm/2`。鱼种基础价仍为白条6、鲫鱼8、鲤鱼12、土鲶12、溪蟹10、马口16、趴地虎16、黑鱼28、黄颡鱼30、中华鳖60、斑鳜72。出售一次结算容器及船尾全部鱼，不清图鉴；超过金币上限整笔拒绝。

| 商品 | coins | 交付 |
|---|---:|---|
| 玻纤竿 | 550 | 永久解锁并装备 |
| 碳素竿 | 1800 | 永久解锁并装备 |
| 细线小钩 / 通用线钩 / 强韧大钩 | 120 / 200 / 450 | 装备，断线损坏后需补购 |
| 鹅毛漂 / 夜光漂 | 90 / 120 | 永久解锁并装备 |
| 蚯蚓 / 米饭 / 面饵 / 小鱼 / 虾饵 | 8 / 6 / 10 / 18 / 20 | 每包五份 |
| 大木桶 /20 kg | 300 | 替换低级容器 |
| 储鱼箱 /35 kg | 750 | 替换低级容器 |

初始竹竿、通用线钩、红白漂和无限普通假饵；特殊饵料每次抛竿消耗一份，即使收回也消耗，最后一份用完后下一竿自动用普通假饵。重复永久装备、容器降级、余额不足、库存超过999均整笔拒绝。价格唯一契约见 `data/economy.json`，C表应同步维护。

## 时间、存档、工程与发布

系统单调毫秒驱动，每1秒现实时间为1分钟游戏时间，一昼夜24分钟，05–07渐亮、18–20渐暗。新游戏起始06:00加运行秒数，读档恢复游戏分钟，退出期间不快进。毫秒余量与uint32回绕安全。

存档v7为1317B，读取v1–v6；声音字段保持偏移260，新增1024B单鱼售价分组记录，保持原基础字段、密钥、UUID派生与目录。V4装备保留，更早版本沿用一次初始化标记。包含四种稀有图鉴和实物记录；V7使用独立A/B文件，V6、V5及原文件只读保留，避免切回旧版覆盖进度。后台写非活动槽、同步并读回确认，不操作背光PA26。完整布局和文件名见[存档说明](save-system.md)。

标准工程默认 `FISH_LCD_ROTATION=0`，对应ST7789由原2转向180°，240×240窗口按驱动使用Y_SHIFT=80；其他工程未指定时保持默认2。常用Embedded Studio工程在 `hpm5301evklite_flash_uf2_debug/segger_embedded_studio/mklink_sample_fishing.emProject`，选择Debug。方向与帧率仍需实板确认。

公开仓库只保留实际运行链接的 C 位图/蒙版/字形及必需许可；原图、备用版本、PNG/图集、TTF、提示词与预览均留本地。没有新增整屏缓冲。Windows39项CTest、标准demo11项测试、GNU UF2/XIP及SEGGER构建，具体结果见验证报告。尚未实板烧录、掉电或栈水位验证，也未创建Android/微信运行应用。

商店素材统一正面、较平面的构图。外屋64×48逻辑像素加采样边框，整数2倍显示约128×96；原图重新生成到目标网格，门框约38像素，略高于36像素主角。营业亮窗开门，打烊暗窗关门；两个状态共用轮廓，熄灯状态只导出实际变化的门窗区域，避免整屋跳动及重复占用Flash。内景120×72、店主每帧40×50，全部2倍最近邻显示；柜台水平前沿遮挡人物下缘。商品改为五行文字列表，已移除九组独立商品摆拍位图及对应变换操作。背景保留RGB565灯光色，不量化丢失壁灯亮部。

左岸转动一次即以32像素/秒持续慢走，反向转动先停住，再转一格反向起步；短按停止并执行交互。走到商店门口、西码头上岸点或岸边界自动停止，菜单中不会继续行走。单帧补算上限250 ms，避免卡顿后的瞬移；计时支持uint32回绕。地面使用生成的48×46逻辑像素草地/土路贴图，2倍显示并连续铺满左岸下方，店铺下不再透出湖水。

六条竿/漂描述由 `docs/tackle-descriptions.md` 导出到运行界面，店主台词由 `docs/merchant-dialogue.md` 导出，新增汉字同步位图字库。价格和存档格式不变。

码头交互替代行囊上下船按钮：船抵达码头自动停止，再向岸方向转动一格上岸并开始慢走；步行回到码头自动停止，再向水方向转动一格登船，船保持停泊。码头方向操作保持，去掉顺逆方向上船提示。菜单仅保留图鉴、存档、返回；原DOCK菜单ID保留为内部空位，未序列化进存档。

收鱼卡显示当前鱼的出售价格（SELL＋金币图标），已解锁图鉴显示该鱼最佳纪录尺寸对应的价格（BEST）；未解锁项不显示售价。鱼库按每种实际存货汇总价值，商店“出售鱼获”显示整批售价。价格展示统一调用实际结算函数，不另外维护估价表。

岸上小人按船上渔夫身份重绘：棕色钓鱼帽、绿外套、深蓝裤，三帧14×20像素含采样边，内容高18像素，以2倍显示为36像素，与船上人物使用同样的2像素颗粒。站立帧0；移动每6像素在接触/经过两帧间交替，停住立即回站姿。共享裁切与脚底基线，左右镜像，步态计数不进存档。详情图标复用已批准素材缩为14×14并加1像素采样边，2倍显示为28像素内容，RGB565共7168B，无新增原图发布。

接岸修正v19：码头接岸段按原土路重绘，土路在世界x=0处延伸到码头下24像素，以同网格A8蒙版重叠过渡，避免竖直碎石断面。东岸镜像共用资源并有地面衬底。码头仅存56×47逻辑像素与1像素边框（58×49，RGB565/A8共8526B），运行2倍显示，替代原112×94展开数组，节省23058B像素载荷。脚底统一y=134，岸边与木板不再跳高；提示框移到放大人物头顶上方。左岸镜头最小值-336保证完整显示放大商店，门口对齐人物。43幅新PFB捕获覆盖两岸、边界移动、昼夜、上下船及商店营业/打烊比例。

## 稀有变异、云层与停步 v20

11种基础鱼外另有黄化土鲶、长鳍马口、红色长尾鲤、白化鳖，选中对应基础鱼后独立抽取1%；成功起鱼后记录为四个新索引11–14，单独图鉴/库存，彩色外框，出售价格为同尺寸普通鱼计算并取整后的10倍。携带时有四类专属招呼，普通进店5%提示收购线索。共14类28句台词，原稿保留；详见[稀有变异设定](rare-variants.md)。

行走中反向一格先停止，第二格再反向走；停在码头后另转一格上下船，不显示顺向上船之类提示。云重绘为舒展的薄云，90×10逻辑像素加边框（3312B）整数2倍显示，以原山线裁切，始终位于山后，不占用额外整屏缓存。

存档升级为v5（260B载荷、292B文件），前204B基础字段不重排，追加四种稀有进度与库存。读取v1/v2/v3/v4，V4装备保持；继续使用原密钥/UUID/目录。新V5_A/B槽与原A/B文件分开，原文件只读保留，有效V5优先加载，防止切回旧版覆盖稀有鱼记录。

## 大鱼细节 v21 / Large-fish detail v21

黑鱼、鲤鱼、土鲶各有48×28普通贴图与96×56细节贴图。收鱼卡和已解锁图鉴按实际显示比例选择：超过原96×56参考画布的1倍比例时使用细节版，较小时仍用原版，体型与屏幕显示尺寸不变。细节从已批准的原始母图重新采样，保留16色RGB565像素风、二值透明蒙版、原朝向和外形比例；不对低清贴图插值造细节。小鱼、鱼库和船尾保持原显示颗粒，四种稀有变异及所有数值规则不变。

Snakehead, carp and catfish each have a 48×28 regular tile and a 96×56 detail tile. Catch cards and unlocked guide entries select detail art when the display scale exceeds 1× the original 96×56 reference canvas; smaller presentations retain the original pixels and all on-screen sizes stay unchanged. Detail is resampled from the approved masters, retaining the 16-color RGB565 pixel style, binary alpha, facing and proportions. Small fish, storage and stern fish retain their original pixel density; rare variants and gameplay values are unchanged.

11种基础鱼取消预先展开的2倍重复像素，统一以48×28原生贴图绘制；释放133,056 B，三种细节贴图增加48,384 B，位图净节省84,672 B（约82.7 KiB），另有少量资源表与绘制代码变化。沿用Arm-2D/PFB变换操作，每帧准备一次、分块复用，不增加整屏缓冲、运行时解码或堆分配。最终Flash/RAM占用见`validation.md`。

The 11 base sprites now store native 48×28 pixels instead of duplicated 2× pixels, recovering 133,056 bytes. Three detail tiles add 48,384 bytes, for a net bitmap saving of 84,672 bytes (about 82.7 KiB), plus small table/code changes. Existing Arm-2D/PFB transforms are prepared once per frame and reused across blocks, with no additional full-screen buffer, runtime decoder or heap allocation. Final Flash/RAM usage is recorded in `validation.md`.

## 汉化、视差与内容核实 v22 / Localization, parallax and content audit v22

收鱼卡与鱼库的鱼名、尺寸类型/长度/重量、状态/水深/钓深、售价/纪录、商店价格/已拥有和装备数值说明已补齐中文。v23统一恢复原有12像素中文字体；撤销v22新增的8像素小字库，价格、鱼获数值及状态栏通过对齐和间距调整适配，不依赖运行时字体文件。左岸使用连续有符号远景视差、原图边缘镜像延展，云每900 ms漂移一像素并保持山后遮挡，无新增背景位图。

Fish names, metrics, states, depths, prices, records, ownership and tackle properties are localized using the original 12-pixel Chinese font throughout in v23. The v22 8-pixel subset is removed; price, catch metrics and HUD rows use adjusted alignment and spacing. Signed shore parallax reflects the existing panorama edge; clouds drift one pixel per 900 ms and remain behind the skyline, with no new background bitmap.

11种基础鱼各20万次实际抛竿选择，共220万次，全部15个收藏项均能抽中并完成起鱼/图鉴/售价记录。趴地虎未被条件误排除：右岸浅水虾饵钓底的专门匹配条件下抽中136363次；这不是全局概率。店主14类情境、28句台词和四种立绘的逻辑与实际PFB画面均核对通过。完整鱼种抽样、线长/竿身弹性、双向挣扎与界面规则见[v22玩法与核实](v22-gameplay-and-audit.md)。

The audit runs 200,000 actual cast selections for each of 11 base species (2.2 million total); all 15 collection entries are selected and pass landing, guide and price checks. Goby is reachable, with 136,363 selections under its targeted right-bank shallow/shrimp/bottom profile, rather than a general probability. All 14 merchant contexts, 28 lines and four portraits pass logic and actual PFB checks. The linked v22 audit documents complete results and current rules.

## 字号与提示修正 v23 / Font and hint correction v23

全部中文统一使用原有12像素字体；移除v22新增的8像素字库，紧凑栏通过对齐和间距调整适配。移除“鱼竿弯了！快收线”提示；弯竿表现、两秒弹性窗口及搏鱼规则不变。蟹的显示名改为“溪蟹”，土鲶去掉真实水域别称，地图设定不指向真实地区；物种索引和存档不变。

All Chinese text uses the original 12-pixel font. The v22 8-pixel subset is removed, with alignment and spacing adjustments for compact rows. The misleading reel-now message during rod flex is removed; rod animation, the two-second elastic window and fight rules are unchanged. Crab uses the generic display name “溪蟹” (stream crab), catfish loses its geographic alias, and the setting is a fictional wetland. Species indices and saves are preserved.

## 蜂鸣器反馈 v24 / Buzzer feedback v24

成功提竿上钩播放160 Hz/60 ms低频短脉冲；鱼实际挣扎时播放110–170 Hz不规则低频短震，每次20–50 ms、静音间隙20–70 ms，体力恢复和被动鱼种安静。成功买卖播放660→1100 Hz的上扬收银音，40 ms首音、10 ms间隔、80 ms尾音；失败交易静音。退出搏鱼或商店立即停止，使用既有PWM及单次定时器，不阻塞绘图或输入。所有音符占空比20%，详见[蜂鸣器参数](buzzer-feedback.md)，实板声音/震感待试听。

Successful hooks play a 160 Hz/60 ms low-frequency pulse. Active bursts play irregular 110–170 Hz pulses lasting 20–50 ms with 20–70 ms silent gaps, with recovery and passive species silent. Successful trades play an ascending 660→1100 Hz cashier cue (40 ms, 10 ms gap, 80 ms tail); failed trades stay silent. Leaving the fight/shop stops playback. Existing PWM and one-shot timers avoid blocking drawing/input; all notes use 20% duty. See the linked parameters; physical sound/vibration audition is pending.

## 声音设置与两页图鉴 v25 / Sound settings and two-page guide v25

行囊底部按图鉴、声音、存档、返回排列。声音提供关闭／仅鱼讯／全部开启三档，短按进入调整、旋转选择、短按确认并请求保存；调整立即生效。声音档位写入v6认证存档末尾偏移260，文件由292增至293字节，旧v1–v5迁移默认全部开启。保留原密钥、目录、索引和所有旧文件，优先V6、其次V5，再读取更早版本。

The bag footer reads guide, sound, save and return. Tap sound to edit, rotate to choose off/fish-only/all, and tap to confirm/save; changes apply immediately. Sound is authenticated in v6 at payload offset 260, increasing files from 292 to 293 bytes. v1–v5 migration defaults to all sounds. The key, directory, indices and all old files remain; loading prioritizes v6, then v5, then earlier versions.

图鉴第一页保留鱼图、纪录和评语；第二页用四行12像素文字显示时段、水域、钓层和推荐鱼饵，不显示多少米的范围。内容从实际地图/鱼饵/活跃配置生成，稀有变异继承基础鱼；未收集只显示简短线索。旋转切鱼，短按翻页，长按600 ms返回来源页面；进入时默认鱼卡页。斑鳜提示全天、晨昏稍好，不偏夜间。

The first guide page retains portraits, records and remarks. The second shows four 12-pixel rows: activity, area, layer and recommended bait, without numeric depth ranges, derived from actual game rules. Variants inherit their base rules; locked entries show a short hint. Rotate to select fish, tap to change pages, and hold 600 ms to return. Entry starts on the portrait page. Mandarin fish remain day/night-neutral with a mild dawn/dusk preference.

本次不增加背景位图；构建检查要求至少保留64 KiB应用Flash供后续地图页面，地图页本身尚未实装。SD多地图素材仍只保留规划。

No background bitmaps are added. Build checks reserve at least 64 KiB of application Flash for the future map page, which is not implemented yet. SD multi-map assets remain a recorded plan.

## 多地图资源方案已在v29实现 / SD map resources implemented in v29

此前暂存的SD资源方案已由v29实现，详见本页顶部和[资源协议](maps-and-sd-resources.md)。

## 交易金币浮字 v26 / Transaction popups v26

成功购买立即显示暖色 `-550G` 等扣款，成功出售显示浅绿色 `+11G` 等收入。出售第一条立即弹出，其余每250 ms一条；每条向上移动48像素、存续900 ms，末250 ms淡出，最多同时显示四条，放在商店柜台右侧。余额和鱼获在交易成功时一次结算并请求保存，动画不延迟结算；空鱼库、余额不足、已拥有、库存满或打烊时没有金币浮字。旋转和购买可继续操作，购买不清掉待显示的出售明细；离店清除剩余视觉队列。

新鱼获按真实单条售价记录，等价售价合并计数以节省RAM与SD空间，出售时仍按条弹出，包含稀有鱼和船尾大鱼。最多256种不同售价；枚举当前全部合法尺寸，即使为每种售价选择最轻鱼，35 kg容器最多219种，不减少鱼库容量。扩容后船尾鱼入库也保留单价。售价记录保存于v7，重启后继续准确显示。旧v1–v6鱼库只有物种合计，迁移时保留全部数量/重量/金额；缺失的明细显示一次“旧鱼获合计”，不虚构单价，新鱼和已知船尾鱼仍逐条显示。

Successful purchases immediately show a warm `-550G` charge; sales show pale-green `+11G` income per fish. The first sale popup is immediate, followed by one every 250 ms. Each rises 48 pixels over 900 ms, fading over its last 250 ms, with at most four visible over the right counter. Balances and cargo settle atomically and request a save before the animation finishes. Failed transactions produce no popup. Inputs remain active, purchases preserve the pending sale sequence, and leaving the shop clears only cosmetic feedback.

New catches retain exact individual prices, with equal prices sharing a count. Selling expands these counts into individual popups, including rare and stern catches. All valid sizes give a conservative maximum of 219 different prices in the 35 kg hold, below the 256-group capacity. The v7 save persists these records. v1–v6 cargo has only species totals: migration preserves all cargo and money, and clearly labels its unrecoverable details as one legacy total. New catches and the known stern fish still show exact individual amounts. No bitmap, heap allocation or framebuffer is added.

## 不规则挣扎反馈 v27 / Irregular struggle feedback v27

挣扎音效改为私有随机短脉冲，起音间距40–120 ms（受线程更新延迟影响），每一下同时改变频率、时长和间隙，避免机械节拍。音效随机数与鱼种、尺寸、搏鱼和运动随机数独立，存档格式仍为v7/1317 B。实际冲刺以外安静、卡顿不补播、收银与上钩提示保留；细节见[蜂鸣器反馈](buzzer-feedback.md)。当前重量理论上限为1米鲤鱼／红色长尾鲤16,461 g；1.2米土鲶／黄化土鲶15,170 g。

Struggle feedback now varies pitch, duration and silence using a private random state. Pulse onsets are 40–120 ms apart before thread delays. Fishing/size/fight/motion randomness and the v7/1317-byte save remain unchanged. Actual lulls stay silent and stalls do not queue audio. The current theoretical weight maximum is 16,461 g for a 1 m carp/red longtail carp, followed by 15,170 g for a 1.2 m catfish/xanthic catfish.

## 右岸小火车站 v28 / Small east-bank railway station v28

新地图扩展的第一步是在当前地图右侧增加小型火车站外景。右码头船位仍为828、上岸点864；继续向右走，经过原岸边与生成式草地小路，到1216的车站停住。右侧步行边界1240，岸上镜头上限1088。靠站后再旋转可继续至边界或返回；回到864码头先停住，再向湖面旋转才回船，与左岸一致。短按仍是原背包操作，本次尚未加入目的地选择、火车运行或地图切换。

车站为正立面瓦顶、木梁与暖砖墙的小站房，右侧候车棚、站钟、长椅与行李箱；ImageGen参考现有渔具店和草地生成。原图仅作本地源素材，运行时使用110×54 RGB565/A8贴图、整数2倍显示、24色，像素数组17,820 B。人物保持原36像素高度，站房底座落在同一地面。候车站台前方的简化轨道位于行走路线下方，列车尚未实装。

右岸地面复用原生成式草地贴图，96屏幕像素为一段连续拼接，原码头羽化重叠保留。超出原湖面宽度的远山/天空沿右缘镜像延续，云、月、夜色仍使用对应山脊遮挡；没有新增整屏背景。小地图标记限制在屏幕范围，靠站时位置显示“车站”。实际湖区、钓点、鱼种权重与存档格式v7不变，SD多地图位图加载仍仅记录。

The first map-expansion step adds a small railway-station exterior on the current east bank. Boat position 828 and landing 864 remain; continuing right reaches the station at 1216 and stops. Walking ends at 1240, with an onshore camera limit of 1088. Returning stops at landing 864 before another turn boards the boat. Tap keeps the existing bag action. Destination selection, train movement and map switching are not implemented in this step.

The frontal tiled-roof station has a waiting shelter, clock, bench and suitcase. Built-in ImageGen used the existing shop and path as style references. A 110×54 RGB565/A8 tile displays at exactly 2× with 24 colors (17,820 pixel-array bytes), preserving the 36-pixel actor. The existing generated meadow repeats continuously with the original dock blend. Right-edge reflection extends the original panorama and matching sky occlusion without another full-screen image. Lake geometry, encounter weights, save v7 and the deferred SD bitmap-loader plan remain unchanged.
