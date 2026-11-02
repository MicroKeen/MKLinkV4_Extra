# D20 海滩员工与车站布局 / Coastal staff and station layout

MKLink V4，240×240 RGB565，单颗旋转／按下编码器。本版在D19海滩基础上调整NPC与建筑，海鱼仍待加入。

海边鱼档员工改为现代蓝色工作服、棒球帽和浅色防水围裙，单独使用四张42×50的原生像素立绘：招呼、成交时高兴、余额不足等情况下犯愁、看到巨物或稀有鱼时惊讶。四张图共用裁切范围与调色板，避免切换时人物尺寸与肤色跳变；场景中柜台后的小员工也使用同一形象。城市摊主和湿地店主保持原形象。柜台前缘继续遮挡员工下半身。

海滩左岸现在按车站→鱼档→沙滩礁石→码头排列：

| 项目 | 世界范围／坐标 | 原生像素 | 屏幕像素 |
|---|---|---|---|
| 单层小车站 | 建筑X0～192，进站点X72 | 96×48 | 192×96 |
| 海边鱼档 | 建筑X216～328，交互点X272 | 56×38 | 112×76 |
| 公路与堤岸 | X0～352 | 88×36地块重复两次 | 统一2倍像素 |
| 礁石区 | X416～544 | 64×40 | 128×80 |
| 西码头 | 上岸X480，船X540 | 沿用D19 | 沿用D19 |

车站与鱼档间保留24像素的建筑间隙，基底均落在Y142的岸上行走平面。车站保留时钟、售票窗和站台雨棚，整体比鱼档宽大；鱼档降低为较小的独立档口。两者使用原生像素图后2倍显示，不用非整数运行时缩放。

进入海滩后的到站位置改为X72，沿岸走动会依次停在鱼档和车站；短按打开对应界面。车站仍24小时营业，鱼档仍06:00～16:00。原海滩存档的世界位置仍可读取，不重置装备、图鉴、天气或鱼获。大陆架、125米外缘地形和最右船位114米水深保持D19设定。

新配套资源为 **2026.10.30.1，格式3**，需整体更新common/fish.fgpk和maps下三张场景包。V17存档仍1511B，原密钥、目录、旧档及下载器文件系统保持。资源新增位于场景缓存，不新增堆、整屏缓冲或AHB鱼图解码槽；编译与内存核验另见本版报告。

素材使用内置image_gen生成，像素转换工具为 `tools/prepare_coast_staff_d20.py`；本地原图、提示和出处记录位于 `assets/source/beach/` 的 `employee-*-v1.png`、`station-compact-v1.png`、`coast-d20-prompts.md` 和 `coast-d20-provenance.json`。公共工程仅导出C运行素材与资源包。立绘沿用 `docs/city-fishmonger-dialogue.md` 的情境／表情映射及台词。

The beach now has a modern uniformed employee with four aligned expression portraits and a matching outdoor sprite. The compact railway station occupies the far west, followed by a smaller fish stall and the beach/reef dock. Station and stall render at192×96 and112×76 screen pixels on the same native2x grid. Their interaction points move toX72 andX272; arrivals and walking stops use the new coordinates. Existing V17 saves remain valid, and D19's deep shelf and other maps are retained. Install the complete matching2026.10.30.1 format3 resource folder. No repository push or device flashing is performed by this local update.
