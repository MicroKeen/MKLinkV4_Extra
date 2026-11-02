# D23 沙滩与潮间带 / Expanded beach and tidal flat

保留左侧水泥道路，改为水平路面，末端为两个空停车位；移除木码头、栏杆和立柱，快艇直接停靠礁石外缘。沙滩向左延长，礁石区改为横向展开、带裂缝和棱角的低矮潮间带，给后续赶海留出活动与物品摆放空间。灯塔和海中钓点保留。本轮只有场景和行走范围调整，六种赶海生物仍为图鉴素材占位，尚无采集操作。

| 区域 / Area | 世界坐标 / World range | 说明 / Notes |
|---|---|---|
| 岸边整体 | -384～480 | 原岸边向左增加384px |
| 水泥道路 | -384～112 | 水平铺设、两个空车位、低路沿，衔接沙滩 |
| 沙滩 | -384～384 | 768px，原为480px；末段与潮间带重叠衔接 |
| 潮间带 | 272～512 | 240px，原礁石128px；低礁石、湿沙与浅水池 |
| 可行走 | -352～480 | 车站、鱼档、沙滩、潮间带相通 |
| 车站 / 鱼档 | -312 / -112 | 建筑相对间距、尺寸和营业时间保留 |
| 下水 / 船位 | 480 / 540 | 与旧版相同 |
| 钓鱼水域 | 480～1440 | 水深、鱼种区域、陆架边缘1080px保持原坐标 |

潮间带使用新生成像素素材，延续原沙滩与灰蓝礁石配色。空旷湿沙、带裂缝与棱角的层状岩台和少量浅池取代堆叠大礁石。原木码头不再绘制；角色落脚处补齐连续地面，快艇靠在外缘，离船时显示短系缆。左侧道路另用生成素材重做，保留水平水泥路面、低路沿和末端车位，不使用旧的高墙式素材。

岸边扩展采用已有的有符号坐标表示，存档格式保持V19、2062B。新增负坐标保存与读入已覆盖；旧版正坐标和船位仍有效，不平移已有存档、不清空装备或鱼获。海图不会再在湿地商店的同名坐标处无故停步。

配套资源更新为**2026.10.30.9／格式3**。使用同包UF2与完整FishingGame_Resources_DO_NOT_DELETE目录，保留下载器原文件和FishingGame_Saves_DO_NOT_DELETE。素材尺寸、布局和CRC都有变化，需完整覆盖资源目录。

生成原图与提示留在本地assets/source/beach/generation-d23.json，公共demo只包含实际使用的C素材、配套资源和许可证。使用固定调色板读取与原有Arm-2D tile/mask接口，不增加整屏缓冲或堆分配。最终静态内存和缓存详见beach-foreshore-d23-memory.md；主机画面验证不代替实机帧率与堆栈水位测量。

D23 retains a horizontal concrete road with terminal parking on the left, removes the wooden pier and moors the speedboat directly at the reef edge. The coast extends384px westward, with a768px sand beach and240px-wide angular tidal flat for later beachcombing. The station and fish stall move west; boarding, water coordinates, depth and fish distributions remain unchanged. Signed shore positions use the existing V19 layout; older locations stay valid. The six forage portraits remain placeholders. Install the complete matching2026.10.30.9 resource folder with this firmware, preserving saves and downloader files. Local build; hardware acceptance pending.

D23 R1：停车区绘制在建筑以右，248×36道路分段复用240px行缓冲，不增加SRAM。当前道路原图与提示记录为assets/source/beach/road-parking-generation-d23-r1.json。鱼类素材、分布与收线检查见fish-audit-d23-r1.md。
