# MKLink V4 组件与容量 v21 / Component and capacity audit v21

应用分区为851,968 B（832 KiB），不扩大分区。下表以GNU ELF应用净载荷为准，包含代码、只读资源、初始化映像和对齐。

The application partition remains 851,968 bytes (832 KiB). Values below are measured from the GNU ELF application payload, including code, read-only assets, initialization images and alignment.

| 阶段 / Stage | 应用占用 / App bytes | 余量 / Free bytes |
|---|---:|---:|
| v20 | 842,068 | 9,900 |
| v21 原生鱼图＋三种细节图 / Native fish tiles + three detail tiles | 757,524 | 94,444 |
| v21 去掉浮点scanf强制链接 / Remove forced floating scanf | 748,892 | 103,076 |

11种基础鱼从96×56展开数组改为48×28原生像素，恢复133,056 B；黑鱼、鲤鱼、土鲶各增加96×56细节图，共48,384 B，位图净减少84,672 B。大体型卡面/图鉴自动切换细节图，小尺寸画面保持原像素；没有新增整屏缓冲、运行时解码或堆分配。

Storing the 11 base sprites as native 48×28 pixels instead of expanded 96×56 arrays recovers 133,056 bytes. Three 96×56 detail tiles for snakehead, carp and catfish cost 48,384 bytes, giving a net bitmap saving of 84,672 bytes. Enlarged cards/guide entries switch to detail art while small presentations keep the original pixels, with no new full-screen buffer, runtime decoding or heap allocation.

SDK原先强制链接`_scanf_float`，游戏不使用scanf输入。仅在钓鱼工程移除该强制项，链接器裁去对应浮点输入及其依赖，额外节省8,632 B；最终ELF无`_scanf_float`，保留`_printf_float`供现有诊断使用。浮点绘图数学运算仍保留。没有修改共享SDK或其他demo。

The SDK previously forced `_scanf_float` into the image even though the game does not scan input. Removing that forced link in the fishing project lets the linker discard floating input support and its dependencies, saving another 8,632 bytes. The final ELF omits `_scanf_float` and retains `_printf_float` for existing diagnostics, as well as floating drawing math. The shared SDK and other demos are unchanged.

已启用函数/数据分节与`--gc-sections`，未用的busy wheel等显示组件已被裁掉；移除其源文件不会继续缩小当前固件。最终符号中没有USB主机/设备初始化或busy wheel入口。现有字库为实际界面汉字子集，SD文件名使用长文件名，exFAT兼容性保留；这些功能未为节省容量而裁撤。

Function/data sections and `--gc-sections` already discard unused helpers such as busy wheel; deleting their source files would not further shrink this image. The final symbols contain no USB host/device initialization or busy-wheel entry points. The Chinese font is an actual UI subset, saves use long filenames, and exFAT compatibility is retained.

39项模拟器测试和11项独立demo测试通过，GNU UF2/XIP、SEGGER Debug构建与分区/UF2校验通过。三种小体型卡面与v20逐像素一致；大体型保留文字/售价，细节图包含1,510个不再是重复像素的2×2单元。完整内存表与固件哈希见[验证记录](validation.md)。未执行实板烧录或性能测量。

All 39 simulator tests and 11 standalone demo tests pass, alongside GNU UF2/XIP, SEGGER Debug and partition/UF2 validation. The three small fish cards are pixel-identical to v20; large-card text/prices remain intact, and detail art contains 1,510 nonduplicated 2×2 cells. See the validation record for memory regions and firmware hash. No hardware flashing or performance measurements were performed.

后续多地图SD位图加载方案只记录于[整体设定](game-design.md)，本次未实现。

The future multi-map SD bitmap loading proposal is recorded in the game design only and is not implemented in this update.
