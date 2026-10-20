# D3：加载页变换初始化修复 / D3: loading-page transform fix

后续确认（2026-10-19）：作者已报告D3成功加载进入游戏。随后D4将加载页人物替换回原坐船钓鱼图标，见 [D4图标调整](loading-boat-d4.md)。以下保留D3交付时的定位与验证记录。

Follow-up: the author confirmed a successful D3 boot. D4 subsequently reuses the boat icon; the original D3 delivery evidence below is retained.

2026-10-19。本地固件 `NorthWetland-loading-fix-2026.10.19-d3-local.uf2`；尚未实机验收，继续禁止 Git 推送。

## 现场与定位 / Evidence and diagnosis

作者的 D2 照片记录 `S13 00000000 / PC 80029f96 / C 5 / V 008781e6`。与已交付 D2 ELF 的 SHA256 `9f9d061465e475059c4b9030ea32f63f980a290928b4f7e6dfa6d8a7f5fc8582` 核对一致。`mcause=5` 是读取访问异常。PC 对应 Arm-2D `__arm_2d_op_frontend_op_decoder+0x54`，实际指令为 `lhu a2,0(a5)`，在解引用绘图操作的 `Target.ptRegion`；`mtval` 表明区域指针无效。看门狗是 CPU 异常后的复位结果。

MCU 导出脚本从 `frame_start` 的模拟器分支开始，一直替换到 `self->hint_box`。这个范围还包含加载/路线页面的 `shore_player_operation` 变换 prepare 和提前返回，导致嵌入式代码误删它们。资源校验完成后，`loading_page` 第一次绘制行走小人时使用未 prepare 的操作。电脑版没有经过该删除，因此原来的模拟器启动测试无法覆盖这个问题。HPM 地址0是有效 ILM 向量区，空操作描述指针并不一定马上产生空地址读取异常，后续会按错误描述处理区域；不能仅用主机上的首个异常地址判断是否同一缺陷。

The D2 photograph captured stage 13, PC `0x80029f96`, load-access cause 5 and bad address `0x008781e6`. Matching the delivered ELF locates an invalid `Target.ptRegion` dereference in the Arm-2D operation decoder. The MCU export script accidentally removed the loading/route sprite's transform preparation and early return while replacing simulator-only frame code. The first destination frame invoked that unprepared operation. The original PC scene retained the code, so its successful boot test missed the export defect. Address zero is mapped ILM on HPM; a null operation descriptor can be interpreted as vector data before failing at a later access.

## 修复与回归 / Fix and regression

导出替换边界改为共享的加载/路线准备分支之前，完整保留 prepare 和提前返回；不修改 Arm-2D 库，不再增加等待时间或看门狗喂狗频率。保留 D2 异常捕获，页面版本后缀改为 D3。构建验证另检查导出的准备分支。

新增测试直接包含生成后的 `mklink_sample_fishing/src/fish/fish_scene.c`，使用真实 Arm-2D/PFB 和两份实际资源包，只替换硬件时钟与蜂鸣器。修复前，两张地图均在资源检查后的绘图中以 SegFault 失败；修复后，同一测试覆盖两张地图首次空缓存启动及已有有效缓存的再次启动，均能经过目的地加载页进入可操作场景。主工程61项、独立demo21项检查通过。GNU 编译成功；应用 Flash 372,088 B，余量8,840 B；DLM 126,480 B，余量3,824 B。没有新增帧缓冲或修改资源包。

The export boundary now preserves the shared preparation branch and early return. Arm-2D itself, watchdog timing and storage ownership are untouched. A regression test compiles the actual generated MCU scene with real PFB and resource packs. Both maps segfault before the fix; the same tests pass afterward, including cold and cached boots. All 61 root and 21 standalone checks pass. GNU application Flash uses 372,088 bytes with 8,840 free; DLM uses 126,480 with 3,824 free. No new framebuffer is added. These results reproduce and fix the software defect; real-board acceptance remains pending.

## 使用 / Use

按原 BOOT 流程只升级本 UF2。资源包仍为 `2026.10.17.1`，文件内容不变，无需重拷；原存档保留。若 D2 留下的复位记录使新固件先显示诊断页，确认后缀 D3 后短按编码器一次继续。若仍复位，保留新诊断页的 PC/C/V，用于区分新异常与旧记录。

Update only the UF2 using the existing BOOT method. Resource pack `2026.10.17.1` and saves remain compatible. If the previous D2 reset record first opens the diagnostic page, confirm D3 and tap the encoder once to retry. Capture the new page if a further reset occurs. No physical-device writes/flashing or Git push were performed by the agent.
