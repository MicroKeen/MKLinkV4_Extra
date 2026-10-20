# D9 主角、近景、鱼库、大鱼与写盘 / Foreground, cargo, fights and SD writes

2026-10-19，MKLink V4。本地构建，不推仓库，不烧录或操作设备磁盘。

## 画面 / Visuals

都市气垫船直接复用湿地木船上24×24取样框内的原人物像素：272个不透明像素保持完全一致。气垫艇来自原有生成式空艇素材，先采样至32×10，再按整数2倍导出，整艇和人物统一2×2屏幕像素颗粒。岸上继续使用湿地三帧14×20素材、整数2倍显示，头肩比例一致；站姿因腿部展开自然比坐姿高，不改变站姿36px的身高。没有重新生成另一套人物。

前景统一下移8px，地面脚点从134移至142、水面基准从135移至143。码头、商店、车站、路灯及其独立透明光束、人物、船、鱼漂、抛竿/断线效果、船尾大鱼和绳索一起移动；远景及HUD位置不移动。船底增加贴合水线的透明暗部和轻涟漪，柱脚涟漪按原素材接点镜像布置，消除分离的投影。灯光继续使用生成式像素图及原独立柔光蒙版。

夜间路面在原夜色之外单独压暗，人物和路灯光层保留在前。所测路面区域的夜间平均亮度约为白天的湿地39%、都市51%，灯光在暗路面上更明显。路灯仍18:00–06:00。

鱼库改为原图鉴框和纯色底，不再铺容器纹理。容器以32×36、1倍小图标显示在上方；重量、金币、鱼名、鱼获重量/售价、页码和总价保留。稀有亚种仍为彩色外框；空鱼库、三种容器、历史红尾与船尾大鱼均验证。

City and wetland boats now share the exact same actor pixels and a 2×2 screen grid. The original three shore poses remain at 2×. The whole near plane moves down 8px, including buildings, lamps, rig, hung fish and ropes, while scenery/HUD stay fixed. Water contact overlaps the hull and meets dock piles. Night roads are darker under the existing generated lamp texture. Cargo uses the collection card frame with a small header container icon.

## 大鱼 / Large fish

实机反馈组合：竹竿、强韧大钩、约13kg长吻鮠。排查发现长吻鮠虽设为慢拉，但旧逻辑只要收线到底就可钓起，剩余体力约77%时也会直接结束；旧基础拉力封顶后，城市巨物的重量差异也被削弱。

- 超过6.5kg、体力仍高于20%的鱼，收线进度最多到95%，先遛疲再收起。6.5kg及以下保留原收起规则，不强制小鱼耗尽体力。
- 大鱼有效收线每格消耗3点体力，避免鱼越大反而越容易耗尽。爆发原有体力消耗、恢复、疲劳衰减继续生效。
- 大体型慢拉鱼的压力建立更快，向外拉线幅度随重量增加。仍每次爆发约70%向远离玩家侧、30%向近侧，不固定反向。
- 超过20kg时，每多2kg增加1点基础负荷，上限30点，参与原疲劳/爆发计算，最终拉力上限140；旧湿地体型不受这个额外负荷影响。
- 长吻鮠保持贴底慢拉、下沉翻滚和低频长间隔节奏；慢拉不改成跃水型。白鲢/翘嘴原跃水类型、活跃鱼爆发和东方鲀被动鼓胀/磨线保留。鳙鱼不继承红尾机制。

固定种子42、每100ms观察并按张力控线的回归脚本中，13.024kg长吻鮠从5.7s、2次慢拉、6次放线，调整为13.1s、4次慢拉、17次放线，收起时体力68/368。每500ms只收一格的另一脚本为23.0s，峰值张力113/140、6次慢拉。快速硬收会断线。上述为模拟输入结果，不代表人类成功率。

同时复跑城市大体型鱼/线组/操作策略以及原4455组湿地匹配；最大体型青鱼的同一控线脚本为40.6s，55kg鳡35.3s，60kg鳙鱼36.3s。巨物没有按旧装备削弱。完整CSV见本地`build/giant-fight-d9-audit.csv`；整体参数见`data/species.json`的`fishing_rules`。

Large fish above 6.5kg must reach 20% stamina or lower before final retrieval. Small fish retain the earlier rule. Heavy slow drag gains pressure/run feedback, and urban giants above 20kg retain extra force after the old base cap. The reported bamboo-rod/heavy-hook/13kg longsnout case was reproduced in scripted input. Its controlled fight now lasts 13.1s instead of 5.7s. These are deterministic regression results, not human success rates.

## S07/01 写盘 / Storage

用户实机错误`S07/01`表示`f_open`返回FatFs磁盘I/O错误，发生于打开存档文件阶段，不能由此断言文件系统已经损坏或磁盘已满。不是存档密钥、鱼体型校验错误。

对照原MKLink下载器，原单扇区写使用CMD24，游戏原FatFs适配器却将所有写请求都交给SDK的CMD25。D9将FatFs写请求拆成逐扇区CMD24，第一处失败即停止，不盲重放已接受的数据。CMD24/25写事务临时使用4MHz，所有退出路径均恢复地图读取时的SPI频率。维持原2秒事务超时和事务边界喂狗，不改CS接线、不操作背光，不修改文件系统格式、分区或下载器文件。

这一调整由原代码路径差异和实机阶段码驱动，**尚未由用户设备确认S07/01已消失**。原D8手动安全重试、写入前校验、A/B回读校验和未读取进度保护继续保留，V10仍1485B，不更换密钥或文件名。

S07/01 identifies a FatFs disk I/O error during save open. D9 aligns writes with stock MKLink's single-sector CMD24 path and uses 4MHz only while writing, restoring read speed afterward. It stops at the first error without blindly replaying accepted sectors. No format, repair or partition change is performed. This compatibility correction still needs confirmation on the user's board.

## 资源、内存和验证 / Resources, memory and validation

配套资源 **2026.10.19.4**。更新完整资源目录，保留原存档和下载器文件；旧D8产物保留。新资源布局和载荷大小仍为454,008/466,944B，外部Flash缓存剩12,936B。

最终GNU固件：程序Flash剩23,112B；DLM剩3,824B，与D8相同，已计入堆预留、PFB及线程栈；没有新增整屏缓冲。详见[内存报告](city-d9-memory.md)。加载后两图六页面的Arm-2D scratch申请仍为0；实机堆峰值和线程栈水位未测。

66项主工程和24项独立demo测试通过。包含实际HPM存档worker/FatFs合成盘、写入错误后A/B恢复、CMD24逐扇区范围与失败停止、写入限速及异常退出恢复读速、原下载器文件/预留尾区不变、167张PFB城市/近景截图、1088个船上人物实绘像素与12组岸上姿态精确匹配。未操作物理SD卡。

Matching packs: **2026.10.19.4**. Preserve all downloader files and saves. APP flash free: 23,112B; DLM free: 3,824B; external cache free: 12,936B. Tests: 66 root + 24 standalone, with 167 city/foreground PFB frames. Physical-device storage confirmation and heap/stack peaks remain outstanding.
