# 标准钓鱼 demo 验证

HPM SDK 1.11.0；GNU RISC-V GCC 13.2；SEGGER Embedded Studio 8.24。2026-10-15 编译检查。

GNU UF2 的 2937 个块、地址、Family ID 和应用净载荷逐块校验通过；SES XML、279 个唯一源文件及 include 路径检查通过。

独立 demo 的 12 项主机测试全部通过：玩法、搏鱼、图鉴、地图装备、尺寸、经济/扩容/时钟、稀有变异概率/价格/对白、存档 v1/v2/v3/v4 迁移及 v5 恢复、编码器事件和 SD/背光隔离。

GNU 和 SEGGER flash_uf2/debug 编译成功；GNU flash_xip/debug 也构建并校验通过。编译器仍报告原 Arm-2D/perf_counter/RT-Thread/board 的宏或未使用变量警告，未更改第三方库以消除它们。

Windows模拟器42项检查通过。接岸修正核对15,950个边界/重叠像素、31,900次昼夜镜头对照及2,256个放大角色像素；云层48幅对照核对798,792个山体/前景像素不变并正确遮挡8,090个云像素。另有34幅稀有鱼卡/图鉴/鱼库/对白/船尾画面。人物与外屋统一整数2倍显示。

v21大鱼细节：黑鱼、鲤鱼、土鲶在放大收鱼卡/图鉴中使用96×56细节贴图；11种基础鱼以48×28原生像素存储。三种小体型鱼图区域逐像素不变，文字/售价在v22汉化；位图净节省84,672 B，无新增整屏缓冲。

v21 detail art: enlarged snakehead, carp and catfish use 96×56 detail tiles; all base sprites store native 48×28 pixels. Small portrait areas are pixel-identical; labels/prices are localized in v22. Net bitmap saving: 84,672 bytes; no additional full-screen buffer.

v22：中文鱼名/数值，4.5/6.5/8.5米线长、两秒弯竿缓冲、双向且70%偏远离玩家的冲刺。220万次抛竿核实全部15个收藏项；14类店主情境/28句台词/4种实际表情，24万个岸上远景像素及14幅弯竿画面通过检查。SD多地图加载仅记录。

v23：全部中文恢复原有12像素字体，移除v22新增的8像素小字库；商店名称/售价、鱼获尺寸/重量及线组属性调整对齐和间距。移除“鱼竿弯了！快收线”提示，显示名改为溪蟹/土鲶，地图设定不指向真实地区。42项模拟器检查通过。

v23 restores the original 12-pixel Chinese font throughout and removes the v22 8-pixel subset. Shop names/prices, catch metrics and line properties use adjusted alignment and spacing. The misleading rod-flex reel-now hint is removed, crab/catfish use generic names and the setting is fictional. All 42 simulator checks pass.

| 工具链/区域 | 占用或预留 | 容量 | 余量 |
|---|---:|---:|---:|
| GNU / APP_FLASH | 751,672 B | 851,968 B | 100,296 B |
| GNU / ILM | 2,112 B | 131,072 B | 128,960 B |
| GNU / DLM | 93,776 B | 130,304 B | 36,528 B |
| GNU / AHB_SRAM | 0 B | 32,768 B | 32,768 B |
| SES / APP_FLASH | 742,456 B | 851,968 B | 109,512 B |
| SES / ILM | 27,608 B | 131,072 B | 103,464 B |
| SES / DLM | 67,848 B | 130,304 B | 62,456 B |
| SES / AHB_SRAM | 0 B | 32,768 B | 32,768 B |

Flash 包含代码、只读字形/位图、初始化映像和对齐。RAM 包含两块 240×24 RGB565 PFB、游戏状态、4 KiB 存档线程栈及原 RT/C 堆栈预留。SES 放在 DLM 顶部的固定栈按实际大小计入，未将其下方空闲间隙误记为已用。

显示方向：钓鱼工程使用 ST7789_ROTATION=0（相对原 2 转向 180°），240×240 对应 Y_SHIFT=80。公共驱动允许覆盖参数，其他工程未指定时仍默认 2。方向需在实板上最终确认。

验证未执行实板烧录、SD 拔插、掉电实验、帧率/栈水位测量。主机 HAL 桩证明 SD 回调不操作 PA26，不能替代实板电气验证。

旧存档兼容性：保留原项目密钥、目录、UUID派生及基础鱼索引。写292字节v5并读取原92/236字节v1/v2/v3/v4。v4装备不再初始化；更早版本沿用一次初始化世代。V5使用独立命名A/B槽并保留原文件，存在有效V5时优先读取，避免旧游戏版本覆盖稀有鱼记录。密钥按作者要求公开。

仓库附带 GNU 固件 SHA-256：`663ad3eb5582a66499c52a067e061d54e6645d16fb12ca81cc064a374c6a230c`。

可重新运行 `python scripts/verify_build.py --write-report`（默认检查标准 flash_uf2 构建目录）。
