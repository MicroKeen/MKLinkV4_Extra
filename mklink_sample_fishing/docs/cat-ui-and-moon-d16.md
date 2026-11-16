# D16 猫互动、月相与遮雨 / Cat interaction, moon and shelter

2026-10-27，MKLink V4，D16。包含D15的全部功能及船尾鱼、伞位置修正，尚待实机验证。

## 猫

互动时仅显示78×64的小选项框，保留抚摸、喂食、离开；场景中的猫、主角和商店保持可见。好感和短句反馈放入原底部提示栏，不再显示放大立绘及大面积信息面板。

岸边沿便利店外往返走动，范围为中心左右各28世界像素，约10像素/秒；到一端坐下约2秒，到另一端蜷睡约5秒。走路按方向切换步态，睡觉有zzz。靠近玩家会停下或转向，互动期间停止行走，结束后从原位置继续。玩家接近时停在猫旁边，避免站到猫身上。船上继续只有坐姿、眨眼、睡姿及短暂互动反应。

出现候选仍为都市12:00–24:00、每小时35%、前30分钟的既有规则。整段活动范围及边缘仍在玩家视野附近时，保持当前“有猫/没猫”的状态；不按小时直接刷新或消失，也不会重置本次互动次数。离开这个区域后才能更新下一次出现。收养后也先保留岸边的猫，等玩家离开视野，再让它出现在船尾。

每次见面最多成功互动3次；抚摸+1，喂食+5，好感50收养。每日最多喂2次，罐头和刚钓到的小鱼共用，两图共享。便利店鱼罐头25G，描述改为“喵”。拟真假饵描述改为“他的饵比我的鱼都大”，仍为600G永久使用。

## 月相

24个游戏日一轮，每3天一个阶段：新月、娥眉月、上弦月、盈凸月、满月、亏凸月、下弦月、残月。新月不显示亮面，初始日期沿用娥眉月。两图共享日期和月相，仍沿用原月亮运行轨迹、远山遮挡与雨云淡化。月相由已存档的日期推导，离线不推进，不改变鱼类出现权重。

## 店面遮雨

都市便利店门前屋檐范围：店图中心（商店锚点右侧18像素）的左右各60像素；鱼摊棚内范围：摊位锚点左右各44像素。在这些范围内不增加潮湿，已有潮湿按原遮雨规则逐渐消退；营业时间外仍有效。两处棚檐之间的露天路段仍会淋湿。菜单内和购买雨伞的防潮规则继续保留。

## 存档和资源

V16仍为1511B，在原8B猫状态的空余位中保存出现状态、走动位置和方向/休息状态，没有扩充文件或加载缓冲。使用独立V16 A/B文件；读取V1–V15，保留原密钥、目录和所有旧文件。旧V15首次读取时初始化新的活动状态；之后不会通过读档刷新屏内的猫或互动次数。

月相沿用存档天气日期，无新增时钟。资源版本仍为2026.10.19.5，不需要更换SD地图包。新增内容以C逻辑和原有素材实现，没有新整屏缓存或PNG素材导出。当前检查见相邻构建与内存报告；用户已授权发布；本次未烧录设备或操作实体SD。

## English

The cat interaction is a small78×64 choice box over the live scene. Shore cats stroll across56 world pixels at10px/s, pause to sit/sleep, stop for interaction and keep their position when resuming. They respect the player's space. Boat idle poses remain seated or sleeping.

The current visible/absent visit is locked while its corridor is near the viewport, including hourly boundaries, adoption and save/reload. Visits refresh only out of sight; three interactions per encounter and two shared daily feeds remain. Canned fish says“喵”; the reusable600G lifelike lure says“他的饵比我的鱼都大”.

Eight moon phases rotate over24 game days using the saved date. Existing orbit, mountain occlusion and rain dimming remain. City convenience-store eaves and fish-stall canopies shelter walkers and dry existing wetness even after closing; the exposed gap still gets wet.

V16 keeps the1511-byte layout, packs cat presentation into unused bits, reads all previous versions and writes separate A/B files. The existing2026.10.19.5 SD packs work unchanged. Release authorized by the owner; device testing remains pending.
