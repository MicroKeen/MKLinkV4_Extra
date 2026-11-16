# D18 公共鱼图与缓存 / Shared fish assets and cache

2026-10-29，MKLink V4。保留任意地图可查看的总图鉴、跨地图鱼库和原有鱼图清晰度。本轮未加入海鱼，未推送仓库，未烧录设备或操作实体SD。

## 无损鱼图

25个内部鱼种槽位（含历史红尾）与3张大体型补充立绘，共28张图。原RGB565像素最多17种颜色，Alpha只有0和255，因此采用4/5bit索引、32项RGB565调色板和1bit透明蒙版。没有缩小尺寸、减少颜色或重新绘图；包括透明区域的RGB值在内，全部像素/蒙版逐字节还原一致。

原258,048B压缩至64,120B，减少75.2%。图鉴、鱼库、上鱼卡和船尾挂鱼共用`common/fish.fgpk`；湿地、城市、海滩包不再重复携带鱼图。

保留现有Arm-2D缩放、旋转和蒙版渲染。使用两组固定RGB565+Alpha解码槽，支持船尾鱼与新上鱼卡同时显示，普通48×28和大图96×56的像素尺寸不变。两组最多32,256B，放在原未使用的AHB SRAM，由静态链接分配；没有堆申请、整屏缓冲或DLM大数组。AHB余480B；DLM静态余3,824B，预留堆和线程栈已计入。以后增加素材时仍需遵守96×56解码上限。

## 固定分区

下载器、程序区和原缓存末端均不改变：缓存仍为0x80080000～0x800F3000，共460KiB。

| 分区 | 范围 | 数据占用 | 数据可用余量 |
|---|---|---:|---:|
| 公共鱼图区 | 0x80080000～0x800A0000 | 64,120B | 62,856B |
| 当前场景区 | 0x800A0000～0x800F3000 | 191,868B | 144,004B |

两区各有4KiB提交头。总数据余量206,860B（约202KiB），此前仅17,028B。三张场景只在SD上分别保存，运行时仍仅缓存当前场景。

开机验证公共缓存；匹配则复用。换地图只擦写场景区，公共区不参与；公共版本变化或校验损坏时才重建。各区均在源文件完整CRC预检通过后才开始擦写，先使本区提交头失效，逐扇区写入/回读，再完整CRC验证，最后提交本区头。写入中断后不能把半包视作有效数据，重试会修复受影响分区。公共区和场景区都有效，才允许进入游戏。

普通换图擦除扇区从111个降到48个；公共区不会随地图反复擦写。两个分区共享原4KiB加载缓冲，资源读取、NOR临界区、看门狗边界刷新和SD互斥规则沿用。

## 更新和兼容

资源版本 **2026.10.29.2 / 格式3**，需要完整目录：

- `common/fish.fgpk`：公共鱼图。
- `maps/0_wetland.fgpk`、`maps/1_city_river.fgpk`、`maps/2_beach.fgpk`：场景。

启动会验证全部4个文件的版本、布局、长度、CRC；缺少公共鱼图会明确提示，并按原保护规则开放SD导入。正常开机不弹磁盘。解压整包并更新整个`FishingGame_Resources_DO_NOT_DELETE`目录；不要只更新地图文件，不格式化，不删除下载器文件或存档。

存档继续使用V17独立A/B文件，1511B，保留V1～V16旧档、原密钥、UUID、目录及全部进度；资源布局变化不需要清档。

## 核验

新增实际C解码测试：三个地图中遍历全部25鱼种的普通/大体型图，与原RGB565和Alpha校验值逐一比对，并验证两张不同鱼图同时驻留。换图测试记录公共区擦除/写入计数，确认保持不变；覆盖坏公共包预检、公共区写入中断/重试、坏地图、CRC、缓存复用和加载失败不扣票价。

186张已有图鉴/鱼库等主机PFB截图与优化前逐字节相同。完整主工程81项及独立demo31项通过；另复核了新增公共区写入中断。三图冷/缓存启动和加载后页面的Arm-2D scratch申请仍为0。没有实机堆峰值、栈水位或帧率实测。

## English

The global collection and cross-region cargo remain available everywhere. All28 original fish images are losslessly encoded with4/5-bit indices and a1-bit mask:258,048B becomes64,120B, with every RGB565/alpha byte preserved. No resizing or color reduction occurs. Two fixed decode slots support a stern fish and a catch card simultaneously, using32,256B of AHB SRAM rather than dynamic allocation. Original Arm-2D transforms remain.

The existing460KiB Flash cache is split into a128KiB common bank and a332KiB scene bank, each with a4KiB commit sector. Total free data capacity rises from17,028B to206,860B. Map changes preserve the common bank and erase48rather than111sectors. Independent source CRC preflight, readback, full CRC verification and commit-last headers protect both banks; loading requires both banks to be valid.

Install the complete matching2026.10.29.2 format3 folder, including`common/fish.fgpk` and all three scene files. Downloader files and saves remain untouched. V17 save schema and old progress are retained. Tests cover exact decoding on all maps, concurrent portraits, unchanged common-bank write counts, invalid packs and interrupted writes. All186 baseline rendered frames remain byte-identical. Local build; no device flashing, physical SD operations or repository push.
