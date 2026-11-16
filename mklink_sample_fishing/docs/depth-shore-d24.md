# D24 钓深、价格与岸边衔接 / Depth, prices and shoreline

根据桌面“新建 文本文档 (3).txt”及后续确认，三张地图统一按线组级别和实际线长选择钓层。保留水平道路、末端停车位、宽沙滩、棱角潮间带及快艇直接靠礁石的场景。

| 线组 | 线长 | 钓底条件 |
|---|---:|---|
| 细线小钩 | 4.5m | 当地水深严格小于5m，并满足实际用线量 |
| 通用线钩 | 6.5m | 当地水深严格小于10m，并满足实际用线量 |
| 强韧大钩 | 20m | 只受实际线长限制，原8.5m加长 |
| 八编PE线组 | 25m | 只受实际线长限制，原10.5m加长 |
| 海钓线组 | 200m | 只受实际线长限制 |

用线量按√（水平抛距²＋实际钓深²）计算，三图均参与投放与搏鱼距离判定。湿地最深6m、江河最深16m；最长现有抛距7.2m，江河钓底最多约17.55m，因此20m强韧大钩可以覆盖两张图全部有效投点，八编也能覆盖。强度、售价和竹竿兼容条件保持原设定，初始竹竿仍不能装备强韧及以上线组。

选择钓底时先检查钓底条件与用线量，不满足则尝试中层（当地水深的一半），中层仍不够则退到表层。选择中层时只向表层回退，不会擅自改成钓底。提示“线组不够长，改钓中层／表层”，背包钓层与实际钓深同步；最终选项进入原V19存档。仍保留三个菜单项及旧固定钓深编号的读档兼容。小线组在超过5m水深处仍可尝试中层，只要实际用线量足够。

海图仍以2m/s下沉，回收按原鱼竿／状态速度；湿地和江河保留原投放节奏。线长增加后，超过100kg海鱼的突冲继续随体重增加，不再在旧的跑线幅度上限停住；两个冲刺方向和偏向远离玩家的规则保留。没有添加末段距离门槛、固定拉起等待或最低搏鱼时长。

作者修改的22条海洋价格表已导入，14条基数与上一版不同（含雀尾螳螂虾继承口虾蛄×10）。本次不会重写原有鱼获售价；存档继续保存钓到时价格。正式可编辑来源为coast-species-review.md；鱼类评语、鱼竿／物品描述仍使用原MD输入。

沙滩与礁石的左接边重新生成连续浅沙，并在32px重叠区按原像素网格逐带混合。保留棱角岩台、水池、海岸位置及停车区，不使用额外整屏缓冲或堆分配。原图与生成提示留在本地assets/source/beach/generation-d24.json；公共demo只提供实际C素材、配套资源与许可证。

完整资源版本为**2026.10.31.1／格式3**。请使用同包UF2与完整FishingGame_Resources_DO_NOT_DELETE目录，保留下载器原文件和FishingGame_Saves_DO_NOT_DELETE。V19存档2062B、密钥、旧版本文件、装备、金币、图鉴和位置保留。

验证包含三图多投点／线组／钓层／朝向的实际抛竿、长度和回退判定、旧两图强韧及以上全底覆盖、回退后存读档，以及鱼类素材／分布／收线回归。数字化核验与静态内存报告见fish-audit-d24.md及depth-shore-d24-memory.md；实机帧率、堆峰值与栈水位仍待测。

D24 resolves hook depth consistently across all three maps. Fine rigs may bottom-fish below5m, standard rigs below10m, and stronger rigs are limited by actual line length. Euclidean line range includes horizontal distance and depth. The20m strong rig and25m PE rig cover all wetland and river bottom spots. Unreachable bottom selections fall back to the middle, then surface. Edited marine price bases are imported while historical catch prices remain intact. The sand/reef join uses regenerated sand and a32px blend without extra buffers. Use the complete matching2026.10.31.1 resource folder; V19 and older saves remain supported. Local build, hardware acceptance pending.
