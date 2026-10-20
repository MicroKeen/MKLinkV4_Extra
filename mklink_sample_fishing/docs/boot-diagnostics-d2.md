# D2：校验97%复位现场诊断 / D2: diagnostics for reset near verification 97%

2026-10-19。作者确认 `startup-fix-2026.10.19` 仍在资源校验97%附近复位，没有 NOR 编号。先前修正没有解决实机问题；本版用于取证，不能宣称根因已修复。资源、存档及文件系统行为保持原流程，仍禁止 Git 推送。

The author confirmed that the latest startup-fix still resets near 97%, without a NOR stage. Earlier changes did not resolve the hardware fault. D2 gathers evidence and does not claim a confirmed fix. Resources, saves and filesystem behaviour remain; no Git push is authorised.

## 记录内容 / Captured evidence

新增应用自己的强符号 `exception_handler`，覆盖 SDK 返回同一 EPC 的弱默认处理。异常入口与记录函数在 ILM，仅记录 mcause、mepc 和 mtval 后等待看门狗；不返回故障指令、不读写 SD、不调用绘图或分配器，也不喂狗掩盖异常。已有 AHB NOLOAD 记录扩展到28 B，包含格式标识与全字段异或守卫。冷复位或引导程序清零可能使记录丢失，无记录不能作为未发生异常的证据。

文件锁、打开、定位、读取、关闭前后分别标记，并记录文件偏移或地图号；场景绘图和媒体轮询也有标记。复位后若记录有效，显示静止诊断页并暂停资源读取，短按编码器后才重试。正常页版本后缀为 `D2`，用于确认固件；不会自动把诊断写进原SD。

A strong application exception handler replaces the weak SDK handler that returns the same EPC. Both the handler and recorder run from ILM, capture mcause/mepc/mtval, then wait without storage, rendering, allocation or watchdog refresh. A guarded 28-byte NOLOAD-AHB record provides best-effort warm-reset retention. File lock/open/seek/read/close and GUI/media entry stages add location context. A valid record pauses resource I/O on a diagnostic page until encoder release retries. Cold-reset or bootloader clearing can remove the record; absence remains inconclusive. No SD diagnostic log is created.

| 阶段 / Stage | 含义 / Meaning |
|---|---|
| S01–S04 | NOR配置、属性、擦除、写入 / NOR config, properties, erase, program |
| S05 | 文件系统锁等待 / Filesystem lock wait |
| S06、S07、S08、S09 | 打开、定位、读取、关闭 / Open, seek, read, close |
| S10、S11、S12 | 读取、打开、关闭完成 / Read, open, close completed |
| S13、S14 | 绘图、媒体轮询 / GUI, media polling |
| S15 | 其他CPU异常 / Other CPU exception |

`Sxx` 后的十六进制值在读取/定位时为文件偏移，在打开时为地图编号。`PC` 为故障指令，`C` 为 mcause，`V` 为 mtval；横线表示没有捕获 CPU 异常。mcause在保存时加1以保留0号异常，显示时减回原值。需记录完整页面；不要仅以某一个阶段名判断故障原因。

The detail after Sxx is the hexadecimal file offset for reads/seeks, or map ID for open. PC is the faulting instruction address, C is mcause and V is mtval. Dashes indicate no captured CPU exception. The stored cause is biased by one to retain exception code zero and decoded for display. The complete record, rather than one stage label alone, is needed for diagnosis.

## 验证 / Validation

59项主工程、21项独立demo检查通过，包括新增的实际模拟器启动链：从两份真实配套文件校验，经缓存装入，到第一帧可操作场景。该测试约11秒，仅表示PC模型调度，不作为硬件加载时间。ARM2D/PFB静态截图测试仍保留；没有把它们当成硬件验收。

检查冷路径代码尺寸优化，并把图标导出改为无损逐行RLE，位图从1266 B变为525 B；与上版已交付UF2提取出的调色板和全部1920像素逐字节核对一致，视觉未改变。资源包字节与版本均不变，原文件无需重拷。GNU ELF校验异常函数为强符号且在 ILM，应用仍预留至少8 KiB。详细散列和占用见 `boot-diag-d2-build-validation.json`。

All 59 root and 21 standalone checks pass. The new real simulator boot follows both actual files through validation/cache loading into a playable frame; its roughly 11-second PC schedule is not a hardware timing claim. Icon row-RLE saves 741 bytes, with the palette and all 1920 pixels verified against the previous delivered UF2. Packs remain byte-identical. ELF checks require strong ILM-resident exception capture and at least 8 KiB application headroom. Real-board cause and acceptance remain unresolved; no hardware writes/flashing or Git push occurred.
