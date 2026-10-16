# 标准钓鱼 demo 验证

HPM SDK 1.11.0；GNU RISC-V GCC 13.2；SEGGER Embedded Studio 8.24。2026-10-16 编译检查。

GNU UF2 的 2965 个块、地址、Family ID 和应用净载荷逐块校验通过；SES XML、281 个唯一源文件及 include 路径检查通过。

独立 demo 的 13 项主机测试全部通过：玩法、搏鱼、两页图鉴、地图装备、尺寸、经济/扩容/时钟、稀有变异概率/价格/对白、存档 v1–v6 迁移、声音与单鱼售价恢复、编码器事件、SD/背光隔离及蜂鸣器音效。

GNU 和 SEGGER flash_uf2/debug 编译成功；GNU flash_xip/debug 也构建并校验通过。编译器仍报告原 Arm-2D/perf_counter/RT-Thread/board 的宏或未使用变量警告，未更改第三方库以消除它们。

Windows主机45项检查通过。新增画面覆盖15种鱼已解锁/未解锁习性页和声音三档；原接岸、云层、稀有鱼、店主表情与弯竿验证保留。人物与外屋统一整数2倍显示。

v21大鱼细节：黑鱼、鲤鱼、土鲶在放大收鱼卡/图鉴中使用96×56细节贴图；11种基础鱼以48×28原生像素存储。三种小体型鱼图区域逐像素不变，文字/售价在v22汉化；位图净节省84,672 B，无新增整屏缓冲。

v21 detail art: enlarged snakehead, carp and catfish use 96×56 detail tiles; all base sprites store native 48×28 pixels. Small portrait areas are pixel-identical; labels/prices are localized in v22. Net bitmap saving: 84,672 bytes; no additional full-screen buffer.

v22：中文鱼名/数值，4.5/6.5/8.5米线长、两秒弯竿缓冲、双向且70%偏远离玩家的冲刺。220万次抛竿核实全部15个收藏项；14类店主情境/28句台词/4种实际表情，24万个岸上远景像素及14幅弯竿画面通过检查。SD多地图加载仅记录。

v23：全部中文恢复原有12像素字体，移除v22新增的8像素小字库；商店名称/售价、鱼获尺寸/重量及线组属性调整对齐和间距。移除“鱼竿弯了！快收线”提示，显示名改为溪蟹/土鲶，地图设定不指向真实地区。42项模拟器检查通过。

v23 restores the original 12-pixel Chinese font throughout and removes the v22 8-pixel subset. Shop names/prices, catch metrics and line properties use adjusted alignment and spacing. The misleading rod-flex reel-now hint is removed, crab/catfish use generic names and the setting is fictional. All 42 simulator checks pass.

v24：复用PA10/GPTMR0通道2蜂鸣器，上钩160 Hz/60 ms，挣扎140 Hz/30 ms（每240 ms），成功交易660→1100 Hz上扬短音。非阻塞有限时长、平静/退出停止、失败交易静音和时钟回绕已通过主机测试，尚未实板试听。

v24 uses the existing PA10/GPTMR0 channel 2 buzzer: 160 Hz/60 ms on hooking, 140 Hz/30 ms every 240 ms during bursts, and an ascending 660→1100 Hz cashier cue on successful trades. Host checks cover finite nonblocking notes, lull/exit stopping, silent failed trades and clock wrap; hardware audition is pending.

v25：行囊加入关闭/仅鱼讯/全部开启三档，声音写入293字节v6存档，v1–v5文件只读迁移。图鉴保持12像素字号，两页显示鱼卡/习性，旋转选鱼、短按翻页、长按600 ms返回；36幅新增PFB画面检查通过。不新增背景位图，构建强制保留至少64 KiB Flash供后续地图页。

v25 adds three saved sound modes, a 293-byte v6 save with read-only v1–v5 migration, and a two-page 12-pixel guide. Rotate to select fish, tap to change pages, hold 600 ms to return. Thirty-six new PFB frames pass. No background bitmaps are added; builds require at least 64 KiB of free Flash for the future map page.

v26：成功购买显示扣款，出售按真实单鱼售价每250 ms连续弹出收入，最多四条，900 ms上浮淡出。新售价分组持久化到1317字节v7，完整保留v1–v6。旧档缺失明细标注旧鱼获合计；15幅交易PFB画面验证，原收银音继续通过测试。

v26 shows purchase charges and exact per-fish sale income every 250 ms, with four rising/fading popups lasting 900 ms. A compact journal persists in the 1317-byte v7 save while v1–v6 remain read-only. Legacy aggregate cargo is labeled, and 15 transaction PFB frames are checked.

| 工具链/区域 | 占用或预留 | 容量 | 余量 |
|---|---:|---:|---:|
| GNU / APP_FLASH | 758,916 B | 851,968 B | 93,052 B |
| GNU / ILM | 2,112 B | 131,072 B | 128,960 B |
| GNU / DLM | 101,376 B | 130,304 B | 28,928 B |
| GNU / AHB_SRAM | 0 B | 32,768 B | 32,768 B |
| SES / APP_FLASH | 750,664 B | 851,968 B | 101,304 B |
| SES / ILM | 27,608 B | 131,072 B | 103,464 B |
| SES / DLM | 75,448 B | 130,304 B | 54,856 B |
| SES / AHB_SRAM | 0 B | 32,768 B | 32,768 B |

Flash 包含代码、只读字形/位图、初始化映像和对齐。RAM 包含两块 240×24 RGB565 PFB、游戏状态、4 KiB 存档线程栈及原 RT/C 堆栈预留。SES 放在 DLM 顶部的固定栈按实际大小计入，未将其下方空闲间隙误记为已用。

显示方向：钓鱼工程使用 ST7789_ROTATION=0（相对原 2 转向 180°），240×240 对应 Y_SHIFT=80。公共驱动允许覆盖参数，其他工程未指定时仍默认 2。方向需在实板上最终确认。

验证未执行实板烧录、SD 拔插、掉电实验、帧率/栈水位测量。主机 HAL 桩证明 SD 回调不操作 PA26，不能替代实板电气验证。

旧存档兼容性：保留原密钥、目录、UUID和物种索引。写1317字节v7（新增单鱼售价表），读92/236/292/293字节v1–v6。V4及以后装备保留，更早版本沿用一次初始化世代。V7独立A/B槽保留全部旧文件，优先V7、其次V6、V5、再原版本，避免旧固件覆盖进度。密钥按作者要求公开。

仓库附带 GNU 固件 SHA-256：`35bf4030f9fc3ba377c4f0aaad1112b27bd2cb0126cdaf51681e4493723a2fd7`。

可重新运行 `python scripts/verify_build.py --write-report`（默认检查标准 flash_uf2 构建目录）。
