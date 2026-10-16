# MKLink V4 组件与容量 v26 / Component and capacity audit v26

应用分区为851,968 B（832 KiB），不扩大分区。下表以GNU ELF应用净载荷为准，包含代码、只读资源、初始化映像和对齐。

The application partition remains 851,968 bytes (832 KiB). Values below are measured from the GNU ELF application payload, including code, read-only assets, initialization images and alignment.

| 阶段 / Stage | 应用占用 / App bytes | 余量 / Free bytes |
|---|---:|---:|
| v20 | 842,068 | 9,900 |
| v21 原生鱼图＋三种细节图 / Native fish tiles + three detail tiles | 757,524 | 94,444 |
| v21 去掉浮点scanf强制链接 / Remove forced floating scanf | 748,892 | 103,076 |
| v22 汉化、竿身弹性及视差 / Localization, rod flex and parallax | 756,350 | 95,618 |
| v23 恢复原字号并清理小字库 / Restore original text size and remove compact font | 751,672 | 100,296 |
| v24 蜂鸣器与收银音 / Buzzer pulses and cashier cue | 753,616 | 98,352 |
| v25 声音存档与两页图鉴 / Saved sound settings and two-page guide | 757,104 | 94,864 |
| v26 逐鱼金币浮字与售价存档 / Per-fish coin popups and price journal | 758,916 | 93,052 |

11种基础鱼从96×56展开数组改为48×28原生像素，恢复133,056 B；黑鱼、鲤鱼、土鲶各增加96×56细节图，共48,384 B，位图净减少84,672 B。大体型卡面/图鉴自动切换细节图，小尺寸画面保持原像素；没有新增整屏缓冲、运行时解码或堆分配。

Storing the 11 base sprites as native 48×28 pixels instead of expanded 96×56 arrays recovers 133,056 bytes. Three 96×56 detail tiles for snakehead, carp and catfish cost 48,384 bytes, giving a net bitmap saving of 84,672 bytes. Enlarged cards/guide entries switch to detail art while small presentations keep the original pixels, with no new full-screen buffer, runtime decoding or heap allocation.

SDK原先强制链接`_scanf_float`，游戏不使用scanf输入。仅在钓鱼工程移除该强制项，链接器裁去对应浮点输入及其依赖，额外节省8,632 B；最终ELF无`_scanf_float`，保留`_printf_float`供现有诊断使用。浮点绘图数学运算仍保留。没有修改共享SDK或其他demo。

The SDK previously forced `_scanf_float` into the image even though the game does not scan input. Removing that forced link in the fishing project lets the linker discard floating input support and its dependencies, saving another 8,632 bytes. The final ELF omits `_scanf_float` and retains `_printf_float` for existing diagnostics, as well as floating drawing math. The shared SDK and other demos are unchanged.

已启用函数/数据分节与`--gc-sections`，未用的busy wheel等显示组件已被裁掉；移除其源文件不会继续缩小当前固件。最终符号中没有USB主机/设备初始化或busy wheel入口。现有字库为实际界面汉字子集，SD文件名使用长文件名，exFAT兼容性保留；这些功能未为节省容量而裁撤。

Function/data sections and `--gc-sections` already discard unused helpers such as busy wheel; deleting their source files would not further shrink this image. The final symbols contain no USB host/device initialization or busy-wheel entry points. The Chinese font is an actual UI subset, saves use long filenames, and exFAT compatibility is retained.

45项Windows主机检查和13项独立demo测试通过，GNU UF2/XIP、SEGGER Debug构建与分区/UF2校验通过。三种小体型鱼图区域与v20逐像素一致；v22文字/售价已汉化，细节图包含1,510个不再是重复像素的2×2单元。完整内存表与固件哈希见[验证记录](validation.md)。未执行实板烧录或性能测量。

All 45 Windows host checks and 13 standalone demo tests pass, alongside GNU UF2/XIP, SEGGER Debug and partition/UF2 validation. The three small portrait areas are pixel-identical to v20; text and price labels are localized in v22, and detail art contains 1,510 nonduplicated 2×2 cells. See the validation record for memory regions and firmware hash. No hardware flashing or performance measurements were performed.

后续多地图SD位图加载方案只记录于[整体设定](game-design.md)，本次未实现。

The future multi-map SD bitmap loading proposal is recorded in the game design only and is not implemented in this update.

v24复用蜂鸣器驱动和单次定时器，GNU较v23增加1,944 B Flash、80 B DLM；无需PCM音频素材或独立音频线程。上钩、挣扎和收银音参数见[蜂鸣器说明](buzzer-feedback.md)，未实板试听。

v24 reuses the buzzer driver and one-shot timer, adding 1,944 bytes of Flash and 80 bytes of DLM over v23 in the GNU build, with no audio assets or dedicated audio thread. See the buzzer notes for hook/burst/cashier parameters; hardware audition is pending.

v25新增声音三档及存档字段、四行习性页，GNU Flash较v24增加3,488 B，DLM保持93,856 B；没有新增背景位图。当前空闲94,864 B（92.6 KiB），构建要求至少预留65,536 B供后续地图页，尚未实装地图页面。

v25 adds saved sound modes and a four-row habitat page, adding 3,488 bytes of GNU Flash while DLM stays at 93,856 bytes. No background bitmaps are added. Free Flash is 94,864 bytes (92.6 KiB); builds require at least 65,536 bytes for the future map page, which is not implemented yet.

v26新增交易浮字与256组售价表，GNU Flash比v25增加1,812 B，DLM增加7,520 B至101,376 B（余28,928 B）。Flash占用758,916 B，剩余93,052 B（90.9 KiB），仍超过后续地图页的64 KiB保留量。存档从293增至1317 B；复用原AES-SIV、PFB和12像素字库，仅补充4个缺失字形，没有新增位图或整屏缓冲。单鱼售价表与浮字队列为固定内存。

v26 adds coin feedback and a 256-group price journal, costing 1,812 bytes of GNU Flash and 7,520 bytes of DLM over v25. Flash uses 758,916 bytes with 93,052 bytes free (90.9 KiB); DLM uses 101,376 bytes with 28,928 free. The 64 KiB map-page reserve remains enforced. Saves grow from 293 to 1317 bytes, reusing AES-SIV, PFB and the 12-pixel font with four missing glyphs added. No bitmap, full-screen buffer or dynamic allocation is introduced. Hardware latency and stack watermarks remain unmeasured.
