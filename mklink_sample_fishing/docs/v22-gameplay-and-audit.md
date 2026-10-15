# v22 玩法与核实 / Gameplay and audit

硬件平台为 **MKLink V4**（HPM5301、240×240 RGB565 ST7789、单颗按压旋转编码器）。所有检查使用同一份C规则与实际Arm-2D/PFB模拟画面；未进行实板烧录。

The hardware platform is **MKLink V4** (HPM5301, 240×240 RGB565 ST7789, one push-button rotary encoder). Checks use the shared C rules and actual Arm-2D/PFB simulator output. No hardware flashing was performed.

## 鱼线与鱼竿 / Line length and rod flex

| 线组 / Line set | 最大线长 / Maximum length |
|---|---:|
| 细线小钩 / Fine line, small hook | 4.5 m |
| 通用线钩 / General-purpose line/hook | 6.5 m |
| 强韧大钩 / Heavy line, large hook | 8.5 m |

线长在背包和商店属性显示。实际抛距与蓄力落点预览同时受鱼竿抛距和线长限制；相机按同一距离移动，松开不会突然跳变。最大出线后继续放线无效，也不能继续获得放线降张力的收益。

Lengths appear in backpack/shop properties. Both actual casting and the charged-cast preview are limited by the rod and line length; the camera uses the same limit. Payout stops at nominal maximum length and provides no extra tension relief once the spool is exhausted.

鱼可以在冲刺时拉出少量弹性超距（上限为标称线长的10%，仅表示竿身弯曲缓冲，不能主动放出）。竿身采用8段二次曲线逐渐弯曲，线连接到弯曲后的竿尖。连续超距2000 ms后断线；收回标称线长范围后立即清除超距计时、恢复竿形。缓冲期间允许减速收线，不能靠放线消除超距；正常范围内仍保留原张力/松线风险。恰好达到线长不会触发弯竿计时。

A surge may use a small elastic excess of up to 10% of nominal length, representing rod flex rather than additional manually released line. An eight-segment quadratic curve bends the rod and keeps the line attached to its tip. Continuous excess for 2,000 ms breaks the line; retrieving into nominal range immediately clears the timer and restores the rod. Slower retrieval remains possible during this rescue window. Ordinary tension/slack risks still apply within nominal range. Exactly reaching the limit does not start the flex timer.

## 双向挣扎 / Bidirectional surges

每次爆发开始只抽取一次方向：约70%远离玩家，约30%靠近玩家，同次爆发方向保持，不按帧反复抽取。使用独立动作随机流，不改变鱼种、尺寸或1%变异的抽样。向玩家移动不会自动收鱼，仍需收线完成起鱼。整体爆发移动略增，平静期不变；鲤鱼、马口、斑鳜进一步使用135%、140%、130%的爆发位移系数，其他基础鱼100%，变异继承基础鱼。

Each burst chooses its direction once: about 70% away from the player and 30% toward the player. The direction remains fixed throughout that burst. An independent motion RNG leaves species, size and rare rolls unchanged. A fish moving closer does not land itself; the player must still retrieve it. Burst movement is modestly increased while lulls remain unchanged. Carp, hooksnout minnow and mandarin fish use movement multipliers of 135%, 140% and 130%; other base species use 100%, and variants inherit their parents.

4000次方向抽样为2827次远离、1173次靠近。4455组鱼种/尺寸/鱼竿/鱼线/反应延迟搭配中，4182组起鱼、273组跑鱼，2030组使用放线，最长起鱼56.1秒。它们是脚本策略下的检查记录，不代表真人胜率。

Across 4,000 direction samples, 2,827 moved away and 1,173 moved closer. Of 4,455 scripted species/size/rod/line/reaction-delay matchups, 4,182 landed, 273 were lost, 2,030 used payout, and the longest landing took 56.1 seconds. These are scripted audit outcomes, not human success rates.

## 全鱼种与店主核实 / Fish and merchant audit

对11种基础鱼分别搜索实际可抛出的合适落点、水层和饵料，再各运行20万次完整抛竿选择，共220万次。11种基础鱼与4种变异均实际选中，并通过起鱼入口验证身份、图鉴次数、最佳尺寸和售价。变异条件概率另有四组各20万次独立检查。没有为本次核实提高趴地虎权重。

For each base species, the audit finds a reachable cast position/layer/bait profile and runs 200,000 actual cast selections: 2.2 million total. All 11 base species and four variants are selected and passed through landing to verify identity, guide counts, best size and sale value. Four additional 200,000-sample checks validate conditional rare probabilities. Goby weights were not increased for this audit.

| 基础鱼 / Base species | 20万次中抽中基础鱼 / Base selections | 其中变异 / Rare selections |
|---|---:|---:|
| 鲫鱼 / Crucian carp | 165704 | 0 |
| 鲤鱼 / Carp | 200000 | 2059 |
| 马口 / Hooksnout minnow | 121167 | 1278 |
| 土鲶 / Catfish | 118855 | 1212 |
| 趴地虎 / Goby | 136363 | 0 |
| 黑鱼 / Snakehead | 190102 | 0 |
| 黄颡鱼 / Yellow catfish | 58111 | 0 |
| 白条 / Sharpbelly | 197614 | 0 |
| 斑鳜 / Mandarin fish | 126555 | 0 |
| 中华鳖 / Softshell | 927 | 10 |
| 河南华溪蟹 / Freshwater crab | 45705 | 0 |

每行使用各自专门匹配的条件，数值不表示一般抛竿概率。趴地虎仅近岸底层、5–250 cm虚拟钓深；右岸浅水、虾饵、钓底可正常遇到。收鱼卡现显示“子陵吻虾虎鱼（趴地虎）”，便于识别，图鉴索引未变。

Each row uses its own targeted profile, so these are not general-cast probabilities. Goby requires the nearshore bottom layer and 5–250 cm virtual hook depth. Right-bank shallows with shrimp bait on the bottom are reachable. Catch cards now include the Chinese goby alias; collection indices are unchanged.

店主14类情境、28句台词和四种立绘均核实。空手/提示使用招呼，普通鱼获/出售使用赞许，购买/红色长尾鲤使用交付，巨鱼/其他稀有鱼/已有装备/库存满使用惊讶；钱不足和空鱼库沿用用户设定的招呼表情。交易反馈优先于进店招呼，同时带多种稀有鱼时按总价值最高者选择；打烊无法进入。实际PFB像素对照确认同一表情跨情境相同，四种表情各自不同；原用户台词未修改。

All 14 merchant contexts, 28 dialogue lines and four portraits are verified. Greetings/hints use the greeting portrait; ordinary cargo/sales use approval; purchases/red longtail carp use the handing-over portrait; giant fish, other rare fish, duplicate gear and full stock use surprise. Insufficient funds and empty storage retain the user-configured greeting expression. Transaction feedback takes priority, and the highest-value rare cargo determines its greeting. Closed shops cannot be entered. Actual PFB crops match within each expression and differ across all four. User-authored dialogue is unchanged.

## 汉化与岸上视差 / Localization and shore parallax

收鱼卡和鱼库使用中文鱼名；尺寸类型、长度/重量、售价/纪录、状态、水深/钓深、张力/距离、商品价格/已拥有和抛距/收线说明补齐中文。复用12像素主字库，并新增仅含所需文字的8像素A1小字库，用于紧凑数值栏；构建不依赖字体源文件。

Catch/storage screens now use Chinese fish names. Size metrics, length/weight, sale/record values, states, depths, tension/distance, shop prices/ownership and casting/retrieval descriptions are localized. The existing 12-pixel font is supplemented by a small 8-pixel A1 subset for compact numeric rows. Normal builds need no source font.

左岸镜头使用有符号远景视差，按原湖景边缘镜像延展；不新增背景位图。原山林轮廓同步映射，日月、星星、云仍在山后。云采用较浅视差和每900 ms一像素的缓慢漂移。50幅昼夜画面核对240000个远景像素，另10幅检查缓慢云漂移；原码头接岸与人物比例检查继续通过。

Signed shore parallax extends the existing panorama by reflecting its left edge, with no additional background bitmap. Skyline mapping follows the same extension, keeping sky objects behind mountains. Clouds use shallow parallax and drift one pixel per 900 ms. Fifty day/night frames verify 240,000 panorama pixels; ten more check slow cloud drift. Existing dock joins and actor-scale checks still pass.

SD多地图位图加载仍仅为后续规划，未实装。最终构建、内存与固件哈希见[验证记录](validation.md)。

SD multi-map bitmap loading remains a recorded future plan only. See the validation record for final builds, memory use and firmware hash.
