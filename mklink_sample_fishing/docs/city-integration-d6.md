# D6 城市内容本地版 / Local city-content build

硬件平台为 **MKLink V4（HPM5301、240×240 RGB565、旋转加按下的一颗编码器）**。本次只制作本地固件和配套资源，不推送 Git，也不操作用户设备。

This local build targets MKLink V4. It is not published to Git and has not been flashed to the user's device by the agent.

## 本次内容 / Changes

- 城市江河实装12种鱼：复用鲫鱼、鲤鱼、白条，新增翘嘴鲌、草鱼、青鱼、白鲢、赤眼鳟、鳡、红尾鲌、长吻鮠、暗纹东方鲀。体型上限、活动、饵料与钓层权重采用城市表；湿地原数值保持不变。青鱼为白银。新鱼不要求旧装备能钓获。
- 翘嘴鲌、白鲢与红尾鲌挣扎中会跃出水面，松线时更易脱钩；暗纹东方鲀间歇鼓胀增加阻力，并缓慢磨损当前鱼线强度，最低降至20。每次新中鱼按已装备线组恢复强度。
- 城市采用草地、公园码头、路灯与现代便利店；成年年轻女店员提供四个表情和多组情境台词，柜台在人物前方遮挡。营业 **09:00–21:00**，打烊后外观熄灯，无法进店。湿地仍为老人渔具店，07:00–18:00。
- 城市远景增加雾感和前后层次，保留原来的天际线、单一CBD与远处轮渡。日夜各有配套背景；夜景保留彩色霓虹与射灯。
- 鱼库按实际透明轮廓居中并适配160×56展示框，核对全部24项，修复青鱼偏移。草地实体面与店底、码头同在y136衔接，填补露水面的透明缝；月亮在近岸建筑前绘制，因此可被商店与车站正确遮挡。
- 便利店右侧路灯采用生成像素贴图，18:00亮起、06:00熄灭。灯体开/关两态与独立36×36暖黄柔光图层分开，光层保留多级透明度（上限96/255），人物在光层前方。替换本次城市公园的程序图形路灯。
- 加载页保留坐船钓鱼图标，提示轮换包含“正在整理钓具”“正在前往码头”。车站仍24小时开放，每程180G、5个游戏小时。
- 收获卡、图鉴、鱼库和商店鱼获重量 **大于1000克显示一位小数kg**，如1450克显示1.5 kg；1000克及以下保留克。只调整显示，原始重量与售价计算仍按克。
- 可编辑文字：[收鱼评语](catch-comments.md)、[城市店员台词](city-merchant-dialogue.md)。只改相应表格内容，后续重新生成字库并编译。

The city has twelve fish with independent CSV profiles, including nine new portraits and shared collection IDs for the three returning fish. The convenience store opens 09:00–21:00, with four clerk expressions and editable dialogue. The skyline gains atmospheric depth. Weights above 1000 g use one decimal kg; exact weights/prices are unchanged.

## 配套资源 / Matching resources

固件配套资源版本 **2026.10.19.1 / 2026101901，格式2**。请解压资源ZIP，把完整的 `FishingGame_Resources_DO_NOT_DELETE` 文件夹放在原SD根目录，内部保留 `maps/0_wetland.fgpk` 和 `maps/1_city_river.fgpk`。**不要只拖ZIP，也不要再套一层同名文件夹。** 两个地图包都需要更新，不能混用旧版2026.10.17.1。

正常检测到匹配资源时，USB磁盘隐藏；缺失或版本不符时使用现有条件式导入流程。不要格式化或改分区。保留原下载器全部文件，以及 `FishingGame_Saves_DO_NOT_DELETE` 整个存档目录。

Copy the extracted `FishingGame_Resources_DO_NOT_DELETE` folder to the existing SD root. Both canonical map files must match version 2026.10.19.1. Preserve all downloader files and the entire save directory. Do not format or repartition the card.

地图缓存仍为0x80080000–0x800F3000；载荷449,976 B，头4,096 B，剩余16,968 B。昼夜背景均采用240×120 CLUT64索引，以原像素网格放大，仅复用960 B两行缓冲；没有新增整屏缓冲。

## 存档 / Saves

新存档V9共1470 B，独立A/B槽。保留原密钥、设备UUID和目录；读取V1–V8并迁移，旧文件只读保留。装备初始化世代仍为1，不重置已购装备、图鉴、金币、声音或鱼获。新增9种鱼记录及最佳纪录、最近鱼获、船尾鱼的来源地图，保证城市大鱼带回湿地后尺寸、售价和图鉴仍正确。

V9 uses separate 1470-byte A/B saves and reads V1–V8 without replacing their files. Existing gear, coins, sound, collection and catches survive. Origin-map fields preserve city giant records and prices when returning to the wetland.

## 验证范围 / Validation scope

主工程63项、独立demo22项检查通过；82张PFB画面核对24项鱼库居中、营业边界、月亮遮挡和18:00柔光。像素检查见 `city-d6-visual-validation.json`，构建、分区及测试证据见 `city-content-d6-build-validation.json`。验证还覆盖真实资源包读取、城市12鱼种实际抛竿可遇、跨地图巨物价格/存档、认证V8迁移及导出HPM场景启动。

最终应用Flash占354,412 B，剩26,516 B；DLM链接占126,480 B（含预留堆与栈），剩3,824 B。两图冷/缓存启动及六页面检查中Arm-2D scratch申请为0，资源留在Flash，未在SRAM分配整包。详细范围见 [加载后内存核对](city-d6-memory.md)。主机模型不等同于实机帧率、堆峰值、栈水位或掉电验收；本次未烧录设备。

## 素材 / Art

City shop, clerk, park and lamp/light sprites were generated with the built-in ImageGen tool. Only linked C artwork and used resource-pack data are included; masters, PNG exports and prompts remain in the private root project.
