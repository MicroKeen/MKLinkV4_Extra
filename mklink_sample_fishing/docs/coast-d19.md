# D19 海滩左岸与大陆架 / Coastal shore and continental shelf

平台为 MKLink V4，240×240 RGB565，单颗双向旋转／按下编码器。本版为本地开发，未推送。

海滩仍为1440像素长。最左0～176为公路与混凝土堤岸，64位置有现代海边鱼档：蓝色棚顶、金属柜台、冰鲜展示箱及渔具架，沿用壮硕男性摊主的表情和对话。营业06:00～16:00，收购包括其他地图带来的鱼获、出售渔具；库存与城市鱼摊相同，未擅自加入海钓装备。旋转走到鱼档或车站会停下，短按进入；打烊时无法进入。棚下可避雨。柜台前缘遮挡摊主下半身。

左侧陆地仍到480，车站在240。416～544的生成式礁石衔接沙滩与海水，码头绘制在礁石前方，船与上下岸坐标保留。远景的单岛、灯塔及转动探照灯保持。

海底先缓坡增深，在整张地图约四分之三处（1080）离开大陆架，再进入陡坡，面向开放海域。下表深度以米计，节点间线性插值；地图剖面、当前位置水深和实际钓深共用同一曲线。

| 世界X | 深度（米） | 地形 |
|---:|---:|---|
| 480 | 0 | 沙滩边缘 |
| 540 | 1.6 | 西码头／近岸 |
| 620 | 3 | 浅水 |
| 720 | 5.5 | 缓坡 |
| 840 | 8.5 | 大陆架 |
| 960 | 12 | 大陆架 |
| 1080 | 16 | 大陆架边缘 |
| 1200 | 65 | 陡坡 |
| 1320 | 105 | 深水 |
| 1368 | 114 | 船可到达的最右位置 |
| 1400 | 120 | 深水 |
| 1440 | 125 | 地形最外缘（世界边界不作为可抛落点） |

海鱼暂未安排，仍待正式鱼种与机制。本轮没有把城市鱼直接放入海滩。跨地图图鉴和公共鱼图缓存保留。

四张新素材由内置image_gen生成，仅进行原生网格缩小、调色板压缩和透明度归一化；运行时统一2倍像素显示。主图与提示词仅在本地 `assets/source/beach/`，出处记录 `coast-d19-provenance.json`，转换工具 `tools/prepare_coast_d19.py`。对外工程仅包含实际引用的C资源和配套包，不包含生成原图、PNG预览和提示词。

配套资源为 **2026.10.29.3，格式3**。请用完整资源目录同时更新common/fish.fgpk与maps下三张场景包。V17存档、密钥、目录和旧档保持兼容，不格式化SD、不改动下载器文件系统。缓存边界仍0x80080000～0x800F3000。新增素材位于场景缓存，不增加SRAM解码槽或加载缓冲。

验证及实际占用另见 `coast-d19-build-validation.json`、`coast-d19-memory.md`。主机验证不替代实机帧率、栈水位与操作验收。

The MKLink V4 beach now has a modern steel-and-blue fish stall on its western road/seawall. It buys cross-region catches and sells the existing city tackle inventory from06:00 to16:00; walking stops at the vendor, and its awning shelters the player. Generated reefs join sand and water beneath the boarding deck. Marine species remain deferred.

The gentle shelf reaches16m at worldX1080 (three quarters of the1440px map), then descends sharply:65m at1200,105m at1320,114m at the boat limit1368 and125m at the outer terrain node. Gameplay and HUD share the generated profile in data/map.json. Use the complete matching2026.10.29.3 resource folder. V17 saves, the original cache/write limits and downloader filesystem are retained. Generated masters/previews stay private; runtime C and packs are exported.
