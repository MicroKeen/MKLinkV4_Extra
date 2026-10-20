# 资源导入与下载器 SD 兼容 / Resource import and stock SD compatibility

2026-10-19 D4：作者确认D3已成功加载。进度条上方换回原坐船钓鱼图标，移除行走小人与额外鱼竿；复用固件素材，无需更新资源包，仍不推Git。详见 [D4加载图标](loading-boat-d4.md)。

D4 follows the user-confirmed D3 boot and reuses the original boat/fisherman icon above the progress bar. No new artwork or resource-pack copy is needed. Distribution remains local, without Git push.

2026-10-19 本地启动修正：正常启动取消额外1.5秒停留，有效地图缓存跳过NOR重配置，缓存CRC改为16 KiB一轮；真正写入前才初始化，并补齐RAM返回XIP前的指令同步。资源版本与文件不变，仍不推Git。最新说明与验证范围见 [启动修正](startup-recovery-20261019.md)。

Local startup fix (2026-10-19): no added normal-boot delay, no NOR reconfiguration for a valid cache, and 16 KiB cached-CRC steps. Writer setup is deferred until a validated replacement needs writing; the RAM wrapper synchronises instructions before XIP return. Packs are unchanged, hardware acceptance remains pending, and no Git push is authorised. See the startup-fix guide.

资源版本 **2026.10.17.1** / 包格式 **2**。2026-10-17：按作者确认，恢复下载器原有整卡 USB 访问方式；保留缺包显示、复制校验后自动隐藏的功能。近期版本只在本地，不推 Git。之前容量错误的 `NorthWetland-resource-import-2026.10.17.1-local.uf2` / ZIP 仍保持撤回，以 `.withdrawn` 保留，不能继续使用。

Resource version **2026.10.17.1**, pack format **2**. The author confirmed that exposing the SD as a disk is normal downloader behavior. The corrected local importer keeps conditional visibility and automatic detach. No Git push. The original faulty-capacity UF2/ZIP remain withdrawn; use the separately named `import-progress` local build.

## 97% 附近复位修正（本地） / Local fix for reset near 97%

2026-10-17，作者报告资源检查到“校验城市资源”97% 附近后复位，屏幕显示看门狗标记。该标记说明**上次启动前发生过看门狗复位**，不是当前进度到 97% 才触发的实时报警；现改为“上次为看门狗重启”。

检查发现：两包只读检查结束后会开始配置 NOR 缓存接口，原 `rom_xpi_nor_auto_config` 调用位于 XIP 代码，且中断仍开启。现把配置调用及返回路径移入 ILM，并在 ROM 调用期间屏蔽中断；配置和擦写前后均刷新看门狗，主循环刷新也统一走原子接口。Flash 配置和写入中仍保留看门狗超时，不用中断定时器持续掩盖卡死。

启动检查完成后显示 100% 1.5 秒，下一步先显示“正在前往”目的地，再在下一帧配置缓存，避免检查页最后一张 97% 画面掩盖阶段切换。资源版本、SD 文件、分区和存档无需改变。

本地固件为 `NorthWetland-cache-init-fix-2026.10.17.1-local.uf2`。58 项根工程和 21 项独立 demo 检查通过，包括编译实际 NOR 端口，验证配置/擦写期间的中断保护、喂狗边界、失败返回及缓存越界拦截。GNU ELF 另验证配置函数 `0x628` / 90 B 与写入函数 `0x682` / 124 B 均在 ILM。应用使用 371,712 B、余量 9,216 B；DLM 126,480 B，未增加帧缓冲。

这确认并修正了一处代码缺陷，**尚不能证明它就是该次实机复位的唯一原因**；新固件仍需实机验收。旧测试固件/报告保留，未对用户设备写入或烧录，未推 Git。详细证据见 `cache-init-fix-build-validation.json`。

The author reported a reset near 97% on the city-pack verification page. The watchdog banner describes the previous reset, not a live alarm at that percentage. NOR auto-configuration following the two read-only checks was running from XIP with IRQs enabled. It now executes and returns through ILM with IRQs masked; configuration and writes refresh the watchdog at their boundaries. The original watchdog remains enabled, with no periodic interrupt feed hiding a stall.

Boot verification shows 100% for 1.5 seconds and renders the destination before initialising the cache on the following frame. Packs, SD layout and saves are unchanged. All 58 root and 21 standalone checks pass, including the actual NOR port's critical sections, heartbeat boundaries, failure paths and cache bounds. ELF checks confirm both configuration and write helpers in ILM. App Flash is 371,712 B with 9,216 B free; DLM is unchanged at 126,480 B. This fixes a confirmed code defect; the observed on-board root cause remains unproven until the new local firmware is tested. No user-device writes/flashing or Git push occurred.

## 使用 / Usage

1. 按原方式通过 BOOT 固件升级盘安装本地 `NorthWetland-cache-init-fix-2026.10.17.1-local.uf2`。BOOT 固件升级盘与游戏缺包时出现的真实 SD 磁盘用途不同。
2. SD 原文件系统能正常挂载且容量/分区检查通过后，缺失、不匹配或损坏的资源才会使 SD 作为 USB 磁盘出现，屏幕显示“请拖入资源包”和所需版本。
3. 解压原配套 `NorthWetland-resources-2026.10.17.1.zip`，复制整个 `FishingGame_Resources_DO_NOT_DELETE` 目录到该 SD 根目录。资源版本和内容没变，无需换包。保留下载器原文件及 `FishingGame_Saves_DO_NOT_DELETE`。
4. 停止写入 3 秒后，游戏只读校验两份资源的版本和完整 CRC；新写入会中止校验。成功后再等待 1.5 秒与最后一条 USB 命令完成，自动收起磁盘并加载存档所在地。
5. 资源齐全时正常启动、游戏和切图均不弹盘。原文件系统无法挂载、扇区读取失败或分区超出真实容量时，停在 SD 错误页，不自动修复或格式化，也不把异常磁盘开放给电脑。

Install the local `import-progress` UF2 through the normal BOOT update drive. Missing/incompatible/corrupt packs expose the existing SD only after its volume and geometry pass checks. Extract the unchanged version-matched archive and copy its entire resource directory to the SD root, keeping downloader files and saves. After 3 seconds without writes, both full files are checked read-only. Any write restarts verification; success waits another 1.5 seconds and completed USB commands before one detach. Valid resources keep USB hidden. An unreadable/unmountable or out-of-bounds volume stays on an error page; no filesystem creation, repair or formatting is attempted.

**如果电脑仍提示格式化，请取消。** 新构建的 Windows 实板枚举与拖拽尚未验收；模型测试不能证明用户当前 SD 完整。

**Cancel any Windows format prompt.** The corrected build has not yet passed on-board Windows enumeration/copy testing. Model checks do not establish integrity of the user's current SD.

## 导入进度、复位诊断与图标 / Import progress, reset diagnostics and icon

当前本地固件名为 `NorthWetland-cache-init-fix-2026.10.17.1-local.uf2`。复制中显示收到的 KB 与活动条；此计数包含 USB 元数据与重写，不代表资源文件完成百分比。停止写入后，校验页显示当前地图、连续 0–100% 与耗时。缺文件时指出对应 `maps/*.fgpk` 路径；版本、大小、包标识、CRC 和读取错误分别提示。英文路径字符已纳入字体子集，中文保持 12 像素。

用户报告旧版卡在“正在检查配套资源”，怀疑看门狗重启。检查发现 SDK SD 响应循环及过长 SPI 超时可能阻塞场景喂狗：现为每个 SD 事务约 2 秒预算、每次 SPI 调用 100 ms 超时，响应轮询失败即可返回。保留原 10 秒看门狗配置，在事务边界及场景循环喂狗，刷新键用临界区保护；不在异常轮询内部持续喂狗。主机写入与场景状态使用同一毫秒时钟。

启动时在板初始化之前捕获并清除可用复位标志，确认为看门狗时显示“检测到看门狗重启”。若引导程序已经消费标志，未显示也不能排除重启。旧版实机故障根因尚未证实，新固件未在用户设备烧录验收。

资源页原符号已替换为坐木船垂钓的主角像素图标，沿用橙帽、橄榄色外套与木船配色。图标 48×40、5 位索引、整数 2 倍显示，仅占 1,266 B ROM，不依赖 SD，也不新增整屏缓冲。内置 ImageGen 生成原图，原图/提示词/PNG 预览只保留本地，标准 demo 只同步实际链接的 C/H。

The current local firmware is `NorthWetland-cache-init-fix-2026.10.17.1-local.uf2`. Copying displays received KB/activity, including host metadata and rewrites rather than an invented file percentage. Read-only verification displays the current map, linear 0–100% and elapsed time, with distinct missing-path/version/size/identity/CRC/I/O messages. Required ASCII path glyphs are included; Chinese stays at 12 pixels.

The reported stall may involve watchdog starvation in SDK SD polling, but the hardware reset cause is unconfirmed. SD transactions now have a roughly 2 s budget and blocking SPI calls a 100 ms timeout. The original 10 s watchdog remains enabled; atomic refreshes occur at bounded transaction boundaries and scene-loop boundaries, never inside failed-response polling. USB and scene timing share one clock. Available watchdog reset flags are captured before board initialization and displayed; a bootloader-consumed flag can be absent even after a reset.

The old symbol is replaced by the protagonist fishing from a wooden boat. The 48×40 icon uses packed 5-bit RGB565 indices, integer 2× rendering and 1,266 ROM bytes, without SD or another framebuffer. Built-in ImageGen source art, prompts and previews remain local; only linked C/H assets enter the demo. Resource packs and their version remain unchanged. No user-device writes/flashing or Git push occurred.

## 与原下载器一致的存储访问 / Stock storage mapping

核对本地原工程 `microlink_app/src/USB2MSC/usb2msc.c`：它使用 SDK 的 `block_count` / `block_size`，LBA 0 直接对应 SD 扇区 0，没有虚拟文件系统转换。V4 原分支导出完整容量；部分 Plus 分支限制为 245760 扇区（120 MiB）。

本版修正把原始 CSD `device_size` 当扇区数的错误。SDK 已将 SDSC/SDHC 统一为 512 B 逻辑扇区；容量必须等于 `block_count × 512`，拒绝零容量、非 512 B 或不一致参数。读写拒绝空缓冲、零长度、越界和溢出请求。

开放前只读检查现有 MBR/BPB，支持工程 FatFs 可挂载的常见 FAT12/16/32、exFAT 超级软盘或主分区布局。所有分区和卷长度必须在真实 SD 内。若现有布局完全落在 120 MiB 内，保留 Plus 分支容量视图；需要整卡的布局保留完整容量。不调整起始 LBA、不改分区表、不重建 FAT、不搬移文件。更改 USB 设备修订/序列号，区分之前错误容量的设备实例。

当前 SPI SD 是 CPU 阻塞读写。本工程启用 `FISH_SD_CPU_IO`，绕过通用 DMA 模板的 CPU→总线地址转换与接收后缓存失效；CPU 使用原指针。若日后启用 SPI DMA，编译保护要求重新处理该路径。LCD SPI/DMA 与背光 PA26 保持原适配。

The stock source maps LBA 0 directly to SD sector 0 and uses normalized SDK geometry. V4 exports full capacity; some Plus branches cap MSC at 245760 sectors. The importer validates 512-byte normalized geometry and rejects invalid/beyond-end I/O. Read-only MBR/BPB checks ensure existing volumes/partitions fit the physical card. Layouts fitting the stock 120 MiB view keep it; full-card layouts retain full capacity. No partition/FAT rewrite or file relocation occurs. Updated USB identity distinguishes the old bad-capacity instance. CPU-blocking SPI now retains CPU pointers without DMA translation or post-read invalidation; a build guard rejects mixing that mode with SPI DMA.

## 所有权交接 / Ownership handoff

- 首次开放前等待已排队存档完成、资源文件关闭，确认原文件系统可挂载，然后卸载本地 FatFs。SD 只交给电脑一方写入。
- USB 传输由场景线程在 PFB 绘图任务之间轮询；中断只更新协议状态，不在 ISR 或绘图回调内读写 SD。
- USB 开放期间普通游戏/存档入口被互斥门控阻止。校验仅允许资源加载器以只读方式挂载；下一次主机写入前先关源文件、卸载只读视图，再执行原始扇区写入。
- 校验完成后等待数据/CSW 命令收尾，关闭 USB，再卸载并清空旧挂载状态；游戏重新挂载 SD。失败不创建新文件系统、不删除原文件。

Queued saves and resource handles drain before unmounting FatFs for host ownership. Scene-thread polling serializes SD I/O; interrupts/PFB draws do not access storage. Ordinary game/save access is blocked during USB ownership. Verification grants only the resource reader a read-only view; another host write closes that file and unmounts the view first. USB data/CSW transfers finish before detach, then local access mounts fresh. No automatic filesystem repair or deletion is involved.

## 资源、存档及占用 / Packs, saves and budget

包头 64 B：`FGRP`、格式、地图、布局 CRC、长度、负载 CRC、资源版本 ID、头 CRC。固件比较编译时的精确标识与完整内容，不相信文件名或可编辑版本文字；目标 Flash 地址只由固件决定。两份资源仍为 51 个数组、每份 465796 B，缓存负载区 466944 B，余量 1148 B。新增城市鱼类仍未集成。

存档 v8 / 1318 B 保留原密钥、UUID、目录和所有旧 A/B 文件。USB 使用 1024 B 缓冲，容量检查复用一个独立 512 B 只读扇区缓冲；不新增整屏缓冲。旧商店内景继续使用无损索引资源。

The 64-byte header binds format/map/layout/length/content/version/header CRC to compiled expectations and fixed destination addresses. Existing packs remain unchanged: 51 arrays, 465796 payload bytes with 1148 bytes free. City fish remain unintegrated. Save v8 (1318 bytes), keys, UUID, directory and old A/B files remain. MSC uses a 1024-byte buffer, geometry preflight adds 512 bytes, and no framebuffer is added.

## 验证范围 / Validation limits

58 项 Windows/逻辑/协议/渲染检查与 21 项独立 demo 检查通过。新增兼容测试编译工程实际 FatFs、diskio、SD 驱动、USB 后端、所有权门控和资源状态机，以可丢弃 RAM 磁盘模拟原下载器目录/设置/旧存档，经 MSC 回调复制两份真实资源包，再验证原内容、BPB 和 120 MiB 以外保留区不变。还覆盖 SDSC/SDHC 几何、MBR/FAT32/exFAT 边界、异常卷拒绝、CPU 指针/缓存路径、校验只读、迟到写入和命令收尾。协议测试单独运行实际 MSC 类的 READ10/WRITE10/CSW/RESET。

GNU HPM5301 flash_uf2 编译和 ELF/分区/UF2/资源包检查通过；SEGGER 仅检查生成工程输入，未重新编译。没有对用户设备执行烧录、格式化、修复或试写；尚缺 Windows 实机拖拽、实际吞吐、拔线/掉电与栈水位验证。详细证据见 `cache-init-fix-build-validation.json`。

All 58 root checks and 21 standalone demo checks pass. The integration test compiles actual FatFs/diskio/SD/USB/ownership/resource code against a disposable RAM card, copies both real packs through MSC callbacks, and verifies original downloader/settings/save contents, BPB and reserved tail. Geometry failures, CPU pointer/cache behavior, read-only probes, late writes and command drain are covered; the actual MSC protocol class is tested separately. GNU/ELF/UF2/pack checks pass; SES inputs were inspected but not rebuilt. No writes, repairs or flashing were performed on user hardware. Real Windows USB copy, throughput, removal/power-loss and stack-watermark tests remain outstanding.
