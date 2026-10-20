# 资源就绪后复位与启动耗时 / Reset after resource-ready and startup time

2026-10-19。适用 MKLink V4；以下覆盖 2026-10-17 测试版启动流程。只在本地开发，未恢复 Git 推送。

## 现象与验证边界 / Observation and limits

作者测试 `cache-init-fix` 后反馈：屏幕停在“资源已就绪，即将进入”，底部耗时增加到约2秒，随后看门狗重启。这说明两份配套资源已完成校验，故障范围转移到就绪后的交接或 NOR 初始化；尚未取得实机指令现场，不能宣布唯一根因已确定。

The author reports that the resource-ready screen reaches about 2 seconds before a watchdog reset. Both packs have passed verification by this point. This narrows the fault to handoff or subsequent NOR setup; without an on-board instruction trace, the unique hardware cause remains unconfirmed.

## 本次修正 / Changes

- SDK 1.11 的 HPM5301 `rom_xpi_nor_auto_config` 封装没有新版 SDK 1.13 中的 `fencei()`。在应用自己的 RAM 临界函数里补齐同步，之后才恢复中断并返回 XIP；不修改外部 SDK。
- 原来即使缓存完整，也会先配置 NOR。现在缓存路径只读头和完整 CRC；仅当替换源文件已通过全部预检、确实需要擦写时，才进入单独的配置阶段。失败不擦除原有效缓存。
- 正常启动移除上一版新增的1.5秒显示停留；真实 USB 导入仍保留3秒无写入、1.5秒收尾及数据/CSW 完成检查。
- 场景线程轮询入口明确处理 SETTLE，不再只依赖下一次帧回调完成交接。
- 缓存内存 CRC 每轮从4 KiB增至16 KiB，完整465,796 B从114轮减为29轮；SD预检、写入及回读仍保持4 KiB。只扩大读取跨度，不分配新缓冲。
- 使用12 B已有 NOLOAD AHB RAM保存临界操作阶段，在可用的看门狗复位标志和校验守卫成立时显示 `NOR 1/2/3/4`，分别代表配置、属性、擦除和写入。引导程序或冷启动可能清除 RAM，因此无编号不能排除这些阶段。诊断不写入 SD。

The local wrapper adds instruction synchronisation inside the RAM critical section before IRQ restoration/XIP return, matching the later SDK wrapper. A valid cache no longer configures the NOR writer. Full replacement-source preflight precedes a separate configuration step, so setup failures preserve an existing valid cache. Normal boot drops the added 1.5-second presentation delay; genuine USB ownership retains quiet/drain safeguards. The actual poll entry advances SETTLE independently of frame callbacks. Cached-memory CRC advances 16 KiB per tick (29 steps versus 114), without enlarging the 4 KiB staging buffer. Guarded 12-byte NOLOAD-RAM breadcrumbs provide best-effort warm-reset stages without SD writes.

## 耗时 / Timing

首次加载或切换到另一个地图需要读 SD、逐块擦除和写入 Flash，再回读校验，确实比同图缓存复用慢。正常缓存复用不擦写，也不再执行 NOR 自动配置。帧调度造成的 CRC 轮次已缩短，但本轮没有实机吞吐测量，不能给出实际加载秒数。启动的两包完整校验仍保留，未用跳过 CRC 来掩盖问题。

First loading and map changes require SD reads, NOR erase/program and readback verification; they are slower than reusing the same map cache. Warm cache reuse is read-only and skips auto-configuration. CRC scheduling now takes fewer frames, but no board timing is claimed. Both full source-pack checks at startup remain; validation has not been bypassed to hide the fault.

## 检查与产物 / Checks and artifacts

实际 NOR 端口回归先在旧版复现缺少指令同步，实际 USB 轮询回归先复现隐藏 SETTLE 无法前进；修正后通过。完整58项主工程和21项独立 demo检查通过，包含缓存复用零配置/零擦写、坏源零配置、配置失败保留原缓存、所有权及原下载器文件/存档保留。GNU ELF检查两处临界函数实际包含 `fence.i` 并位于ILM，布局和UF2负载通过。

固件 `NorthWetland-startup-fix-2026.10.19-local.uf2`；详细占用/散列见 `startup-fix-build-validation.json`。配套资源仍为2026.10.17.1 / 格式2，路径正确者无需重拷。未烧录或写入用户设备，实板复位和加载时间仍待验收。

Old-code regressions first demonstrated missing instruction synchronisation and a hidden SETTLE that the real poll entry did not advance. All 58 root and 21 standalone checks pass after the changes. ELF verification checks actual fence.i instructions and ILM placement, partition bounds and UF2 content. Firmware is `NorthWetland-startup-fix-2026.10.19-local.uf2`; packs remain version 2026.10.17.1/format 2, with no recopy needed for correctly placed files. No device writes/flashing were performed. On-board reset/timing acceptance remains pending.

本地对照：`sdk_env_v1.11.0/hpm_sdk/soc/HPM5300/HPM5301/hpm_romapi.h` 与 `sdk_env_v1.13.0` 同路径。官方关于 RAM 执行和屏蔽中断的说明：[先楫 Flash 参数存储说明](https://doc.hpmicro.com/service-support/technical-articles/214)。该资料支持实现约束，不是这次设备故障的诊断证据。
