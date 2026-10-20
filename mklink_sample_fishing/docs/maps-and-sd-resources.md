# 车站、城市江河与 SD 资源 v30 / Station, city river and SD resources v30

## D12 当前资源与地图 / Current resources and maps

2026-10-20作者反馈基本稳定并授权发布。当前配套资源2026.10.19.5，城市12种可钓鱼、独立便利店/鱼摊与气垫筏均已实装；图鉴24项活动收藏。加载图标为原坐船钓鱼小人，V13保存具体位置、时间与分区天气，启动恢复当前地图和位置。两个资源包均需保留完整目录；已有同版本无需重拷。以下旧版本段落为历史，当前数值以[D12说明](rain-depth-and-lines-d12.md)和[发布说明](release-d12.md)为准。

Matching2026.10.19.5 packs contain the implemented city fish/vendors and raft. Loading uses the seated fisherman/boat icon. V13 restores exact location, world time and regional weather. Both packs and their directory structure are required. The author approved publication after reporting basic stability; older sections below are historical.

## 历史实现记录 / Implementation history


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



便利店右侧路灯使用ImageGen像素灯体和独立暖黄透明光层，18:00亮起、06:00熄灭；人物保持在光层前面。鱼库24项按实际透明轮廓居中，限制160×56展示区；实体地面与店底、码头衔接，月亮先于近岸建筑绘制。具体资源和验证见D6说明。

The park lamppost uses generated pixel sprites and an independent soft-alpha light texture, enabled from 18:00 to 06:00. Cargo portraits fit their visible bounds; shop/pier grounding and foreground moon occlusion are corrected.

## 历史记录 / Earlier versions

硬件平台为 **MKLink V4 / HPM5301**，240×240 RGB565、单颗旋转/按压编码器。当前两个钓点均为虚构地点，界面不使用真实地区名称。

## 乘车 / Travel

- 车站 **24 小时营业**，不受渔具店 07:00–18:00 营业时间影响。右岸到达车站后停步，短按打开纸质路线图。夜间候车棚和售票窗口亮暖色灯，平台有淡光；灯光随晨昏渐变、随车站一起移动，人物从灯光前经过时不会被灯光盖住。
- 路线页是车站张贴的纸质线路图：深绿色站牌标题、细印刷格线、轻微折痕、折线铁路和站点编号。虚拟画布仍为 640×480，当前湿地湖泊 (280,180) 与城市江河 (388,230) **在同一屏里完整显示**，不为两个地点拉长线路。
- 底图用淡色地图符号补充地理信息：蜿蜒河流及线路过河处的小桥、三角形山群、针叶林、池塘水纹、湿地芦苇和都市街区轮廓。符号按纸图坐标放置，避开站名；配图、线路和站点在其上方。用现有 Arm-2D 区域和线条绘制，不新增大位图或 RAM 缓冲。
- 相机根据全部已实现站点的标签和配图范围计算总览；当前视窗固定在 (222,119)，旋转只切换选中站点，**不移动镜头**。以后站点放不下时，再使用保留的 360 ms 平移、画面外编号箭头和视窗边条。标题、目的地、票价和操作说明固定。坐标均为 UI 示意，与实际钓鱼距离无关。
- 地点配图扩大为 **64×32**，从原场景中裁取代表区域：湿地显示湖岸山林与水面，都市显示 CBD 楼群、桥梁和水面。选中为全色 alpha 255，未选中为 alpha 192，均有细框，避免把未选中图片淡到无法辨认。名称、站点圆点和“当前”标记不遮挡配图主体，中文仍为 12 像素。旋转选择，短按乘车，长按 600 ms 返回。选择当前所在地免费返回。
- **每次乘车均收 180G，往返各耗 5 个游戏小时（300 分钟）**。两地之间无免费回程。金币不足时不进入加载；加载期间暂停普通游戏时间推进，成功抵达后一次增加 300 分钟，按 24 小时回绕。
- 加载显示真实进度百分比及目的地描述，不显示校验/擦写等技术文字；进度条上方是背着鱼竿行走的主角。检验、擦写、读回检查都分帧执行。只有成功提交资源缓存才扣费、切换所在地并请求存档；失败保留原金币、时间和鱼获，地图页可重试。

Each trip costs **180G and five in-game hours in either direction**. The schematic route uses two scene-backed anchors and a connecting road; selecting an anchor makes its image clearer. Rotate to select, tap to travel, hold 600 ms to return. Currency/time are changed only after a successful cache commit. Failed loads remain retryable without charging. The ordinary game clock pauses during loading.

Stations are open **24 hours** on both maps, independently of the tackle shop. Warm canopy and ticket-window lamps fade with dusk/dawn, illuminate a small platform area and remain behind the walker. The station route board retains its 640×480 virtual sheet and 224×136 viewport. The two current stops are close enough to fit together, including illustrations and labels; a shared overview at (222,119) stays still when selection changes. Only a network that outgrows the window uses the retained 360 ms camera pan, offscreen arrows and viewport rails. Cropped **64×32** framed images emphasize the wooded lake shore versus the city skyline/bridge, using full color for selection and alpha 192 otherwise. Chinese labels stay at 12 pixels. Camera state remains 16 bytes, with no large bitmap or framebuffer. SD packs are unchanged.

Pale cartographic symbols enrich the paper: a winding river with a small railway crossing, triangular mountain groups, conifers, a pond with water strokes, marsh reeds and city-block outlines. They share the sheet's virtual coordinates and sit behind scene illustrations, routes and stop marks, with clear space for names. Existing Arm-2D region/line operations draw these symbols without a new bitmap or RAM buffer.

## 城市场景 / City scene

使用作者选定的现代天际线版本，仅小幅调整部分楼间距和遮挡；跨江桥与远处白色轮渡保留。主角和世界像素网格保持一致：背景逻辑 240×120，整数 2 倍显示为 480×240。城市中原木船替换为气垫筏，主角上半身沿用原贴图。

夜间天空依建筑轮廓遮挡变暗；独立生成与日景对应的城市夜景层：暖白/琥珀窗灯、蓝紫与玫红霓虹、对应的水面倒影，以 64 色索引图存入城市资源包。背景变暗后绘制灯光，人物/码头/车站在其上方绘制，避免灯光穿透前景；两束低透明度射灯在天空中缓慢摆动并受天际线遮挡。湿地原来的背景、云、月亮与夜光漂逻辑保留。

The selected modern skyline retains its bridge and distant ferry, with only minor spacing changes. A 240×120 logical panorama renders at 2×. The city uses an inflatable raft while preserving the existing player. An authored 64-color night layer contains aligned warm-white/amber windows, blue/violet/rose neon and reflections; two moving spotlights are clipped behind the skyline. Foreground composition occludes all city light effects.

当前城市沿用原有鱼种池、鱼饵吸引权重、钓具、鱼库及商店规则，未增加新的鱼种或新的鱼类数值。

The city currently shares the existing species pool, bait weights, tackle, cargo and shop rules; this release adds no new species.

## SD 部署 / SD deployment

资源版本 **2026.10.17.1**。先安装配套 UF2；缺失、不匹配或损坏的资源会自动打开 SD 的 USB 磁盘，提示“请拖入资源包”和所需版本。将配套 ZIP 解压后，把完整资源目录复制到该盘根目录；两份地图完整校验通过后，等待写入/USB 命令完成即自动关闭磁盘并加载游戏。正常启动及切地图保持 USB 隐藏，不需要刷回原下载器固件。详细计时、互斥和测试范围见[资源导入](resource-import.md)。

Resource version **2026.10.17.1** adds automatic SD USB recovery only for missing, incompatible or damaged packs. Install the matching UF2, extract the matching archive and copy the full resource directory. After both files pass checks and host transfers drain, the disk detaches and the game loads automatically. Normal boot/travel keep USB hidden. See the import guide for ownership and validation details.

把发布包中的整个目录复制到 SD 卡根目录，与存档目录并列：

```text
FishingGame_Resources_DO_NOT_DELETE/
  maps/0_wetland.fgpk
  maps/1_city_river.fgpk
  manifest.json
  README_DO_NOT_DELETE.txt
FishingGame_Saves_DO_NOT_DELETE/    # 原存档保留 / keep existing saves
```

新固件首次启动需要 SD 资源包。缓存已经校验且与本版固件匹配时，可直接复用缓存；启动校验不执行擦写。切换到其他固件可能覆盖地图缓存，再切回本版会从 SD 免费恢复存档所在地，**不会再次收车费**。不要删除原存档目录或用资源包替换存档。

Copy the complete resource directory to the SD root beside the existing save directory. First boot requires the packs. A matching, CRC-valid cache is reused without erases. Switching firmware may overwrite the cache; returning reloads the saved map from SD without another fare. Keep saves separately.

公开仓库只包含游戏使用的共享 C 位图/字形、地图资源包和必需许可；原图、未采用版本、PNG、字体源、提示词与截图留在本地归档。

Public files contain only used shared C assets, runtime map packs and required licenses. Masters, discarded variants, PNGs, source fonts, prompts and captures stay in the local archive.

## Flash 与加载协议 / Flash and loader contract

| 地址 / Address | 用途 / Purpose |
|---|---|
| 0x80000000–0x80020000 | 下载器，不写 / Bootloader, never written |
| 0x80020000–0x80023000 | 原配置/启动头 / Existing config and boot headers |
| 0x80023000–0x80080000 | 游戏应用，372 KiB / Application, 372 KiB |
| 0x80080000–0x80081000 | 缓存提交头，4 KiB / Cache commit sector |
| 0x80081000–0x800F3000 | 地图数据，456 KiB / Map payload region |
| 0x800F3000–0x80100000 | 原末尾保留区，52 KiB，不写 / Reserved, never written |

资源包为格式 2 的 64 字节小端头：`FGRP`、格式版本、地图 ID、布局 CRC、数据长度、数据 CRC、资源版本 ID（偏移 24）、头 CRC（偏移 28），剩余字节为零。地址来自固件编译常量，SD 文件不能指定目标地址。每个包数据长 465,796 B，含 51 个外部数组（新增无损索引商店内景）、240 字节天空遮挡和 28,928 B 的 64 色夜景层；湿地夜景层为空。两地图偏移相同，资源版本、布局和本版内容 CRC 都必须匹配。

加载顺序为：校验匹配缓存 → 若无有效缓存则校验整个 SD 包 → 擦除缓存提交扇区 → 每个 4 KiB 数据块读取/擦除/写入/比对 → 完整 Flash CRC → **最后写提交头**。启动也会校验完整缓存数据 CRC 和本版编译内容标识。加载器使用 4 KiB RAM 缓冲，夜景只用 960 B 解码两行 RGB565；不引入整屏缓冲，不在 PFB draw 回调中做文件或 Flash 操作。擦写函数位于 ILM，经芯片 ROM 操作 NOR 时屏蔽中断。FatFS 由存档线程和加载器共用递归互斥锁；SD SPI1、LCD SPI0 与 PA26 背光隔离保持不变。

Both packs use the same fixed layout. A 64-byte little-endian header identifies version, map, layout, size and CRCs; write addresses are compiled constants. Whole-source preflight precedes any erase. Data is processed in 4 KiB blocks, read back and fully CRC checked before committing the header last. Boot validates cache identity and payload CRC. The writer executes from ILM through ROM with IRQs masked. Saves and resource reads share a recursive FatFS mutex. No filesystem/flash work occurs in PFB draw callbacks and no full-screen buffer is added.

单缓存掉电后可能需要从 SD 重载，故不能承诺拔卡或缓存擦写中断后仍立即显示场景；此时固件中的路线 UI、字库和加载小人仍可显示并重试。主机测试覆盖中断模型，尚未实板验证掉电、拔卡、加载耗时、帧率或栈水位。

A single interrupted cache may require an SD reload. Firmware-resident route UI/font/player sprites remain available for retry. Host tests cover interruption models; hardware power-loss/card-removal, latency, FPS and stack-watermark testing remain pending.

## 存档 / Saves

v8 文件 1318 B：在 v7 的 1285 B 明文负载尾部偏移 1285 追加一个地图 ID（0/1），认证方式与密钥不变。写入独立 `NorthWetland_V8_Save_A/B_DO_NOT_DELETE.sav`，保留全部 v1–v7 文件，读取优先 v8 → v7 → v6 → v5 → 原文件。旧档地图为湿地；已完成初始化的装备不重置，图鉴、金币、鱼库、单鱼售价和声音设置原样保留。

The 1318-byte v8 file adds one map byte at payload offset 1285. Independent V8 A/B files preserve all older files, keys and authentication. Loading priority is v8, v7, v6, v5, then original slots. Older saves default to the wetland; initialized gear, collection, coins, cargo/prices and sound settings survive.

夜景数据采用 RGB565 CLUT64：128 B小端调色板＋240×120 B索引；仅按当前PFB解码两行，共960 B RAM。日夜过渡对独立夜景层调整透明度，不再将招牌统一归成固定灯色。加载页显示目的地与钓点描述，校验/擦写阶段只用于内部进度。

The authored night layer uses RGB565 CLUT64 (128 palette bytes + 240×120 index bytes), decoded two rows at a time in 960 bytes. Dawn/dusk blend the night layer; signs retain their authored colors. Loading UI shows the destination and fishing tips while actual internal stages drive progress.
