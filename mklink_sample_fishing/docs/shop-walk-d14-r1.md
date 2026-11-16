# D14R1 商店滚轮与岸上快走 / Shop wheel and fast walking

2026-10-26，MKLink V4，本地更新，接续D14。

湿地渔具店、都市鱼摊和便利店的商品列表均反转滚轮切换方向。商品的类别排序、价格和短按交易保持原规则。

岸上第一档为原32世界像素/秒；同方向再转一格进入第二档快走，64世界像素/秒。反向一格先停止，再转一格以反向慢走启动，同向继续旋转升级快走；短按仍先停下再交互。累计多格输入按格处理，最高两档。

快走的步态按实际行进距离推进，不改变主角大小或像素素材。商店、鱼摊、码头、车站和道路边界仍会自动停车；停在码头后再向水面旋转才上船。船上原10/20像素/秒两档速度保持。

V14存档与2026.10.19.5资源包兼容，无需重拷SD资源。读取存档后仍停在保存的位置，不自动继续走动。没有新增素材、动态分配或状态字段。

All three vendor lists reverse wheel navigation. Shore walking now has two gears:32px/s slow and64px/s fast. A second detent in the same direction selects fast walking; reverse once to stop, then again to start slow in the other direction. Pressing stops movement before interaction. Batched wheel events retain per-detent behavior. Interaction points and route boundaries still stop the player without overshoot, and boarding requires another detent at the dock. Existing boat speeds, V14 saves and SD packs remain compatible. Local build; device validation is pending.
