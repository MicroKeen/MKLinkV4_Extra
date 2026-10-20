# 都市两处商家与热拿铁 / City vendors and hot latte

本地D10，尚未发布。硬件MKLink V4，240×240 RGB565，单颗可双向旋转和按下的编码器。

都市最左侧便利店（世界坐标-260）仅出售热拿铁，35G/杯；原女店员保留。购买后立即饮用，切换递上热拿铁的专用立绘，收银音和扣款浮字照常触发。6个游戏小时内收线、放线速度和提竿窗口各+10%；重复购买刷新为6小时，不叠加。剩余时间显示在HUD，写入V11存档；每次乘车经过5个游戏小时也会消耗效果时间，离线不计时。

便利店右侧、靠近码头的江边鱼摊（-92）有棚子和壮硕男性摊主，负责收鱼、出售装备、饵料和容器。两处暂沿用都市09:00–21:00营业时间，走近自动停住，短按进入。便利店不收鱼，鱼摊不卖热拿铁。两处建筑底部落在同一地面，使用2倍整数像素网格。

都市鱼摊移除蚯蚓、米饭、面饵和鹅毛漂的出售，保留小鱼、虾饵、夜光漂及原有竿线、容器。新增螺蛳30G/5份（青鱼特攻）、玉米24G/5份（鲢鱼/鳙鱼/草鱼）、栩栩如生的假饵65G/5份（肉食鱼，偏向大个体）。专饵每次抛竿消耗1份；普通假饵仍无限。旧库存可随身带往任意地图，未清除、替换或改写旧道具ID。

湿地原商品、装备、饵料权重、基础等待时间与营业时间保持不变：蚯蚓、米饭、面饵、小鱼、虾饵、普通假饵、红白漂、鹅毛漂、夜光漂及其他竿线容器。

都市每次咬钩尝试间隔为原来的85%。专饵按当地水层、时段可出现鱼群的总权重，相对普通假饵总权重乘50%，限制在25%–100%作为咬钩活性；由此在屏蔽常见鱼时可能等得更久。等待仍有65535ms上限。专饵不会让不适合的鱼突破水层、地图或稀有变异规则。仿生假饵对同一种鱼取两次合法体型抽样中的较大值，偏大但不保证巨物。

V11仅追加3种新饵库存和热拿铁剩余毫秒，共10B，完整存档1495B。V10与所有旧版只读迁移，原密钥、目录与装备初始化世代不变。新文件V11_A/B独立写入，保留所有旧存档；不格式化或修复SD。

美术使用内置image_gen生成，主图与提示保存在本地assets/source/city和assets/city-vendors-d10-prompts.md。发布工程只包含实际C代码引用的资源。新增调色板资源放地图缓存，分行解码复用既有960B缓冲，不增加整图/整屏RAM。最终Flash/SRAM预算以D10实测报告为准。

可编辑台词：city-merchant-dialogue.md（女店员）、city-fishmonger-dialogue.md（男性摊主）。新品描述与权重：data/city_items.json。每条店员对白最多16字符，字体12px。

City-only vendor split: the leftmost convenience store serves hot latte; the covered fish stall closer to the pier buys catches and sells tackle/bait. Wetland catalog is unchanged. Hot latte lasts six in-game hours and grants 10% faster retrieval/payout and a 10% larger strike window; duration survives saving and decreases during five-hour travel. New V11 saves preserve all older slots. Local build only; no publishing, device flashing or physical SD modifications.

补充：四个原有商人槽改用6bit索引，透明色单独保留，原湿地立绘像素颜色无损。都市女店员启用招呼、金币不足和递拿铁三套立绘；男性摊主四个表情。地图载荷最终447884B，比D9减少6124B，缓存剩19060B。
