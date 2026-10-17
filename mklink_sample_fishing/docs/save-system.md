# SD卡存档 v7 / SD saves v7

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
