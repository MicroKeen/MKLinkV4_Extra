# D29 三图分布与稀有度 / Habitats and rarity

MKLink V4，2026-11-02。V22存档与2026.10.31.4资源包不变。

多数鱼不再按看不见的“芦苇浅湾／缓流入口”分区，湿地和江河的普通鱼水平倍率均为3，左右对称。钓层、饵料、时间、天气和实际线长仍有效。仅趴地虎与溪蟹保留两侧近岸限制；海图口虾蛄（含变异）、鲻鱼保留近岸，鮟鱇与蓝鳍金枪保留外海至深海，皇带鱼仍限右侧深海。其他海鱼各区均有权重，仍受钓层与深度可达性限制。

## 基准池 / Neutral encounter pool

在同一钓点、当前钓层和时段，先用普通假饵作为中性参照，按各稀有度的可出现总权重校准铜70／银27／金3。三档都有候选时约为70%／27%／3%；缺少某档时在现有档之间按基准比重分配，金池仍受5%上限保护。之后保留实际饵料、天气、诱鱼灯和线组的相对影响，0吸引力仍为0，不会补入被玩家主动避开的铜鱼。地区稀有度按地区profile读取（例如江河鲤鱼为银），不误用湿地档位。

这样不会因为某层仅一种铜鱼、却有多种银鱼而把银级堆成常见。虾饵仍能提高肉食鱼比例，但原始偏区与候选数量不再共同放大翘嘴、长吻鮠。江河湖心白天、不计特殊线组筛选：普通假饵表层白鲦约35.6%、翘嘴27.0%；中层白鲦约35.6%、赤眼鳟34.4%、翘嘴9.2%；钓底长吻鮠约7.7%。虾饵中层赤眼鳟约53.7%、白鲦22.3%、翘嘴14.6%；钓底长吻鮠约20.3%。这些是具体组合结果，不是全地图固定概率。[示例](encounters-d29-examples.csv)。

The neutral eligible pool starts at bronze70/silver27/gold3. Actual bait/tackle/weather/light effects are applied afterward, preserving intentional bronze filtering. Regional rarity profiles are honored; missing tiers do not create fish outside their habitat. This fixes silver-heavy layers caused by candidate counts and hidden spatial bias.

## 金级概率

旧版条件归一化可把斑鳜提高到92.75%，江河金级合计最高47.27%，海图部分组合甚至只有金级鱼。现在普通金级池每次判定上限5%；低于5%的原权重比例保留，高于时先判定金级／普通池，再按该池内饵料、时段等权重选鱼。无普通候选时，金级判定失败会继续等待，每2860ms重试；不会强制出金鱼，不会永久卡在首次空判定。十秒提示计时在失败重试间保留。该上限指单次判定；只有金鱼可吸引的组合，最终钓获当然仍可能全是金鱼，但等待更久。

银级、尺寸采样、搏鱼数据未调整。四种淡水变异和雀尾螳螂虾仍为母种上钩后1%／有猫2.5%；皇带鱼仍按原暴雨、深海、线组条件独立1%／2.5%。

## 鳖

鳖保留金级、钓底与原体型。出现基础权重1→3，小鱼饵1→4，虾饵1→6；普通假饵2保留，米饭、面饵、螺蛳、玉米与大型假饵不吸引它。夜间活跃倍率130%，白天85%。各水域均可钓底遇到，推荐虾饵，其次小鱼。

对可用钓层／时段／线组／天气的同一网格扫描，湿地虾饵、鳖可参与的钓底组合平均占比由0.38%变为1.61%；这是条件网格平均值，不是所有玩家每竿的固定概率。全水层平均会因表层／中层不出鳖而更低。[扫描结果](encounters-d29-audit.json)。

## 猫

原65%只覆盖12:00～24:00每小时的前30分钟，实际时间覆盖约32%。改为每小时85%整小时候选，仍只在都市便利店外，未收养且12:00～24:00时有效。玩家看着的整段区域维持当时有猫／没猫状态；要改变状态需离开该区域视野再回来，不会当面刷新。读档不重抽，每次三次交互、每日两次喂食、好感50收养和三图跟随均保留。

## 图鉴

游戏习性页保持12像素字体，水域改为全水域／两侧近岸／外海至深海等可辨认描述；共享鱼在江河显示当地可购买饵料，海鱼推荐纳入沙蚕。网页附按地图顶部位置条比例的示意，详细Excel保留原始权重。网页和Excel仅本地提供。

## English

Most fish now have broad horizontal availability; freshwater weights are symmetric. Only a few shore/offshore specialists retain spatial restrictions. Depth, real line length, bait, activity and weather still apply. Ordinary gold fish share at most5% per encounter attempt; rare-only pools wait and retry rather than guaranteeing gold. Special1%/2.5% branches and combat settings are unchanged.

Softshell turtles retain gold rarity, gain encounter weight3, shrimp6 and smallfish4, with bottom-only/night activity intact. City cat candidates now cover the full hour at85% between noon and midnight, preserving deterministic saves and no visible spawning/despawning. The in-game12px guide and local-only website/Excel use recognizable locations and current bait recommendations.

## 构建核验 / Build evidence

131项主机检查通过（93＋38），实际GNU MCU固件和运行素材核验通过。357840组位置／钓层／时段／饵料／天气／线组／诱鱼灯情景复核，普通金池不超过5%。96日1152个猫候选小时实际938次出现，整小时稳定、视野锁定、三图跟随、互动与存档检查通过。图鉴保持12px、字体841字形；本地网页49种／53配置／52实际贴图，Excel265个价格公式核对。主机启动测试按V22已有4ms存档精度核对第二次自动存读档，允许至多3ms量化误差。

应用Flash余9308B，较D28R2净增加84B；DLM余1776B，AHB余480B。静态RAM不增加；未新增位图／堆对象，调用阶段使用局部计数器。实板手感、掉电与堆栈峰值仍需验收。

All131 host checks pass. The actual MCU build and runtime-asset checks pass. The357840-scenario audit preserves the ordinary-gold ceiling and all fish remain reachable. Application Flash retains9308B with unchanged static RAM totals. Hardware acceptance remains pending.

UF2 SHA256: `6e21d4a4fcfa72c4674195210463bbfe7d3c06e14d2d2cbcd34e07d9c65c905f`
