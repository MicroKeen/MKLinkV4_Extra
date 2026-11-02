# D15 流浪猫、鱼获与装备 / Cat, cargo and equipment

2026-10-26，MKLink V4，本地开发版，尚待实机检查；接续已发布D14与本地D14R1。

## 流浪猫

都市便利店外下午至夜间会随机出现橘色流浪猫。游戏12:00–24:00，每小时独立35%出现，出现时停留前30分钟，后30分钟离开。读档不会重新抽取，同一轮离开再回来不会重置互动次数。猫位于便利店和鱼摊之间；经过时停下，短按互动，旋转选择。

抚摸好感+1；喂鱼罐头好感+5。每次见面最多成功互动3次。便利店新增鱼罐头25G/罐，最多999罐；没带罐头或达到上限时不消耗物品和互动次数。好感达到50后猫加入船尾，可随玩家前往湿地。

每天最多喂食2次，两地共享，罐头与刚钓到的小鱼合并计数；跨午夜重置。乘车推进5小时也会推进每日限制。次数、好感、罐头和同一轮互动进度进入存档，离线不推进时间。

收养后钓起不超过500g的小鱼，可在鱼卡中旋转选择“收下 / 喂猫”，短按确认；鳖、溪蟹与河豚不用于喂猫。喂食只移除本次鱼获，保留图鉴和钓获纪录；此前已有鱼获不会被扣除。鱼库满时仍可把本次刚钓起的小鱼喂猫。达到每日上限后显示“吃饱了”，仍可收下鱼。

岸边有踱步、坐下、眨眼、蜷睡；船上日常仅坐姿、眨眼和睡姿，睡觉时有缓慢上飘的“zzz”。抚摸有蹭手动作，投喂有低头吃食动作，成功交互弹爱心。猫在船尾随船移动，两地图使用同一套像素密度。

船上的猫以24×24画布绘出，比岸边32×32更紧凑，仍保持2像素绘制网格。猫落在船尾内侧，脚下由原船舷遮挡，随船轻微起伏；上岸后以停泊船的方向固定，不随岸上主角转身换边。“zzz”和喂食爱心移到猫的上方外侧，避开主角与鱼竿。船素材本身不变。

大鱼在船尾外侧从头部系绳，向外下方倾斜，和猫的船内位置分开。船体吃水线以下的鱼身以约41%不透明度融入原水面，交界有细小水纹；鱼头、绳结和船体一起起伏，水面高度保持稳定。

船尾鱼独立按实际体长缩放，以4cm对应1屏幕像素换算，可见鱼身长度限制20～42像素，约为64像素船长的三分之一至三分之二。按各素材实际透明边界归一化，高分辨率鱼图不再被显示得更大；图鉴、鱼库立绘及鱼的体长/重量数值保持原规则。

买过雨伞后，下雨时自动撑开，停雨时自动收起；岸上手持，船上后移到人和猫之间。伞杆为原生1像素、游戏显示2像素宽，不再叠加粗黑描边；沿用24×28画布、368B索引资源。既有防潮效果和道具拥有状态保留。

原稀有变异概率确认是对应基础鱼的1%；猫在船上提升至2.5%。仅作用于黄化土鲶、长鳍马口、红色长尾鲤、白化鳖四个既有变异，不改变普通鱼的基础选择权重；概率不是全部抛竿中的2.5%。

### 当前短台词

| 场景 | 台词 |
|---|---|
| 默认 | 一只在等人的流浪猫 |
| 抚摸 | 它蹭了蹭你的手 |
| 喂食 | 吃得还挺香 |
| 没有罐头 | 没带鱼罐头，去店里看看 |
| 当天喂满 | 今天吃饱了，明天再喂吧 |
| 本次互动用完 | 让它歇会儿吧 |
| 收养 | 这回赖上你的船了 |

以上为文案对照；运行文案在`src/fish/fish_cat.c`，修改后需重新生成字体并构建。

## 鱼获与装备

- 当前饵料、钓层和落点组合没有可咬钩鱼种时，进入等待10秒后提示“没有鱼？调整饵料或钓深”。
- 修复20kg大木桶/35kg储鱼箱已有鱼、剩余空间不足时，超过10kg的本次鱼获未使用空船尾位而被放流。现在先装容器，空间不足且船尾空着时挂到船尾；已有船尾鱼不会被覆盖。针对约11kg青鱼验证保存、加载和出售。
- 初始竹竿只可配细线小钩、通用线组；强韧大钩及八编PE线组需玻纤竿或更好的鱼竿。购买仍能收入背包，不会自动给竹竿装上。换回竹竿或读旧档时优先切换已有通用/细线；无兼容线时保留所有物品，阻止抛竿，需更换鱼竿或补线。
- 钓层只显示“钓底、顶层、中层”，按此顺序循环。中层取落点实际水深的一半，钓底随实际水深；保留原8个存档ID，旧中间深度统一映射中层。
- 栩栩如生的假饵600G，一次购买永久使用；旧档已有正库存视为已拥有，不再扣次数。其他消耗饵仍按原规则消耗。
- 继承本地D14R1：三个商店列表滚轮反向；岸上32/64像素每秒两档，反向先停，交互点自动停下。

## 存档、素材与检查

V15文件1511B，比V14增加8B，保存猫的罐头、好感、每日喂食与同一轮互动进度。使用独立V15 A/B文件，保留原密钥、目录和V1–V14文件；读旧档默认尚未收养猫，不重置装备、图鉴、经济或位置。D15产生的新进度不会反向写入旧版文件。

配套SD版本仍为2026.10.19.5，地图包字节保持不变，只需更新UF2。猫、罐头和爱心的实际索引像素/调色板合计1664B，固件直接使用，复用960B行缓冲，无新增整屏缓冲。构建移除强制链接且界面未使用的浮点格式化输出/输入，保留浮点运算与Arm-2D内核。正式分享时仅包含运行C素材、配套资源包及许可，母图、提示词和预览不进入demo。

主机回归覆盖互动、每日限制、跨图/跨午夜、编码器操作、旧档迁移、饵料装备及鱼库回退；两图各17种加载后页面检查Arm-2D临时申请，另有70帧像素渲染检查，覆盖晴雨收伞、左右朝向、泊船、抛竿/搏鱼及猫与多种船尾大鱼同屏。主机检查不能代替实际MCU的帧率、堆峰值和线程栈水位测量。本次未烧录设备、操作实体SD或推送仓库。

## English

An orange stray cat visits the city convenience-store area between noon and midnight. Each hourly visit has a35% chance and lasts30 game minutes, with at most three successful interactions: pet +1 friendship, canned fish +5. Cans cost25G. At50 friendship the cat joins the stern and travels to both maps. Two feeds per game day are shared across cans and newly caught fish, persist across saves and reset at midnight. Owned cats accept fish up to500g, excluding turtles, crabs and puffers. Feeding removes only the new catch while preserving collection records.

Boat idle animations are seated and curled sleeping poses, with floating "zzz" while asleep; pacing is shore-only. Petting/eating reactions show hearts. Eligible rare variants rise from the confirmed1% baseline to2.5% with the cat aboard.

Stern fish now scale by their stored body length, normalized to each portrait's visible alpha bounds. The display is limited to20–42px, compared with the64px boat, so a96px source image no longer produces a fish twice the size of a48px image. Real catch measurements and collection portraits keep their existing rules.

The boat cat now uses a compact24×24 canvas behind the near gunwale and keeps the moored vessel's orientation when the player turns ashore. Stern fish are tied at the head outside the hull, with the submerged body blended into the water. The owned umbrella opens automatically in rain and stows in dry weather; its boat pole sits between the player and cat. The shaft is one native pixel wide, rendered at2x, on the unchanged24x28 canvas.

The cargo fix uses an empty stern slot when a >10kg catch exceeds the upgraded container's remaining space. Bamboo rods reject heavy/PE rigs. The depth menu exposes bottom/top/middle without breaking legacy IDs. The lifelike lure is reusable and costs600G. V15 adds eight save bytes in separate A/B files and reads all older saves. Existing2026.10.19.5 SD packs are unchanged. This is a local build pending device testing.
