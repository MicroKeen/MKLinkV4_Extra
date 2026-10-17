# 车站、城市江河与 SD 资源 v29 / Station, city river and SD resources

硬件平台为 **MKLink V4 / HPM5301**，240×240 RGB565、单颗旋转/按压编码器。当前两个钓点均为虚构地点，界面不使用真实地区名称。

## 乘车 / Travel

- 右岸到达车站后停步，短按打开路线图。上方偏中为“湿地湖泊”，下方为“城市江河”。仅用道路连接锚点，中间断线表示远距离、非比例示意；不铺满整张地形图。
- 每个锚点背后显示局部场景图：未选中 alpha 55，选中 alpha 205；当前所在地另有标记。旋转选择，短按乘车，长按 600 ms 返回。选择当前所在地免费返回。
- **每次乘车均收 180G，往返各耗 5 个游戏小时（300 分钟）**。两地之间无免费回程。金币不足时不进入加载；加载期间暂停普通游戏时间推进，成功抵达后一次增加 300 分钟，按 24 小时回绕。
- 加载显示真实进度百分比及目的地描述，不显示校验/擦写等技术文字；进度条上方是背着鱼竿行走的主角。检验、擦写、读回检查都分帧执行。只有成功提交资源缓存才扣费、切换所在地并请求存档；失败保留原金币、时间和鱼获，地图页可重试。

Each trip costs **180G and five in-game hours in either direction**. The schematic route uses two scene-backed anchors and a connecting road; selecting an anchor makes its image clearer. Rotate to select, tap to travel, hold 600 ms to return. Currency/time are changed only after a successful cache commit. Failed loads remain retryable without charging. The ordinary game clock pauses during loading.

## 城市场景 / City scene

使用作者选定的现代天际线版本，仅小幅调整部分楼间距和遮挡；跨江桥与远处白色轮渡保留。主角和世界像素网格保持一致：背景逻辑 240×120，整数 2 倍显示为 480×240。城市中原木船替换为气垫筏，主角上半身沿用原贴图。

夜间天空依建筑轮廓遮挡变暗；独立生成与日景对应的城市夜景层：暖白/琥珀窗灯、蓝紫与玫红霓虹、对应的水面倒影，以 64 色索引图存入城市资源包。背景变暗后绘制灯光，人物/码头/车站在其上方绘制，避免灯光穿透前景；两束低透明度射灯在天空中缓慢摆动并受天际线遮挡。湿地原来的背景、云、月亮与夜光漂逻辑保留。

The selected modern skyline retains its bridge and distant ferry, with only minor spacing changes. A 240×120 logical panorama renders at 2×. The city uses an inflatable raft while preserving the existing player. An authored 64-color night layer contains aligned warm-white/amber windows, blue/violet/rose neon and reflections; two moving spotlights are clipped behind the skyline. Foreground composition occludes all city light effects.

当前城市沿用原有鱼种池、鱼饵吸引权重、钓具、鱼库及商店规则，未增加新的鱼种或新的鱼类数值。

The city currently shares the existing species pool, bait weights, tackle, cargo and shop rules; this release adds no new species.

## SD 部署 / SD deployment

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

资源包 64 字节小端头：`FGRP`、版本、地图 ID、布局 CRC、数据长度、数据 CRC、头 CRC，剩余字节为零。地址来自固件编译常量，SD 文件不能指定目标地址。每个包数据长 455,404 B，含 49 个背景/世界/鱼类数组、240 字节天空遮挡和 28,928 B 的 64 色夜景层；湿地夜景层为空。两地图偏移相同，Arm-2D tile 指针保持稳定。

加载顺序为：校验匹配缓存 → 若无有效缓存则校验整个 SD 包 → 擦除缓存提交扇区 → 每个 4 KiB 数据块读取/擦除/写入/比对 → 完整 Flash CRC → **最后写提交头**。启动也会校验完整缓存数据 CRC 和本版编译内容标识。加载器使用 4 KiB RAM 缓冲，夜景只用 960 B 解码两行 RGB565；不引入整屏缓冲，不在 PFB draw 回调中做文件或 Flash 操作。擦写函数位于 ILM，经芯片 ROM 操作 NOR 时屏蔽中断。FatFS 由存档线程和加载器共用递归互斥锁；SD SPI1、LCD SPI0 与 PA26 背光隔离保持不变。

Both packs use the same fixed layout. A 64-byte little-endian header identifies version, map, layout, size and CRCs; write addresses are compiled constants. Whole-source preflight precedes any erase. Data is processed in 4 KiB blocks, read back and fully CRC checked before committing the header last. Boot validates cache identity and payload CRC. The writer executes from ILM through ROM with IRQs masked. Saves and resource reads share a recursive FatFS mutex. No filesystem/flash work occurs in PFB draw callbacks and no full-screen buffer is added.

单缓存掉电后可能需要从 SD 重载，故不能承诺拔卡或缓存擦写中断后仍立即显示场景；此时固件中的路线 UI、字库和加载小人仍可显示并重试。主机测试覆盖中断模型，尚未实板验证掉电、拔卡、加载耗时、帧率或栈水位。

A single interrupted cache may require an SD reload. Firmware-resident route UI/font/player sprites remain available for retry. Host tests cover interruption models; hardware power-loss/card-removal, latency, FPS and stack-watermark testing remain pending.

## 存档 / Saves

v8 文件 1318 B：在 v7 的 1285 B 明文负载尾部偏移 1285 追加一个地图 ID（0/1），认证方式与密钥不变。写入独立 `NorthWetland_V8_Save_A/B_DO_NOT_DELETE.sav`，保留全部 v1–v7 文件，读取优先 v8 → v7 → v6 → v5 → 原文件。旧档地图为湿地；已完成初始化的装备不重置，图鉴、金币、鱼库、单鱼售价和声音设置原样保留。

The 1318-byte v8 file adds one map byte at payload offset 1285. Independent V8 A/B files preserve all older files, keys and authentication. Loading priority is v8, v7, v6, v5, then original slots. Older saves default to the wetland; initialized gear, collection, coins, cargo/prices and sound settings survive.

夜景数据采用 RGB565 CLUT64：128 B小端调色板＋240×120 B索引；仅按当前PFB解码两行，共960 B RAM。日夜过渡对独立夜景层调整透明度，不再将招牌统一归成固定灯色。加载页显示目的地与钓点描述，校验/擦写阶段只用于内部进度。

The authored night layer uses RGB565 CLUT64 (128 palette bytes + 240×120 index bytes), decoded two rows at a time in 960 bytes. Dawn/dusk blend the night layer; signs retain their authored colors. Loading UI shows the destination and fishing tips while actual internal stages drive progress.
