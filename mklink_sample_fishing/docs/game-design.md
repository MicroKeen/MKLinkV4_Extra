# 北方湿地公园：整体设定 v27 / North Wetland: Game Design v27

2026-10-17。适用 Windows 模拟器与标准 `mklink_sample_fishing`，以下为当前实现，价格和规则覆盖旧版说明。

## 功能概述 / Feature overview

硬件平台为 **MKLink V4**：HPM5301、240×240 RGB565 ST7789屏幕、单颗可按下的双向旋转编码器。Windows模拟器用于开发和验证同一份游戏规则。

The hardware platform is **MKLink V4**: HPM5301, a 240×240 RGB565 ST7789 display and one rotary encoder with a push button. The Windows simulator develops and validates the same game rules.

1. **鱼饵与道具 / Bait and tackle**：六种鱼饵对11种基础鱼使用不同吸引权重，结合水域、钓层和时段选鱼；线钩影响等待/搏鱼，鹅毛漂延长提竿窗口，夜光漂提高夜间咬钩概率。

   Six bait types have distinct attraction weights for the 11 base species, combined with habitat, fishing layer and time. Line/hook sets affect waiting/fighting, the feather float extends the hook window, and the luminous float improves nighttime bite chance.
2. **时间与昼夜 / Time and day–night cycle**：现实1秒为游戏1分钟，完整昼夜24分钟，晨昏渐变并影响鱼种活跃度；保存游戏时间，退出期间不快进。

   One real second equals one game minute; a full day lasts 24 real minutes. Dawn/dusk transitions affect species activity. Game time is saved, with no offline advancement.
3. **经济、商店、船上鱼库 / Economy, shop and boat storage**：左侧码头上岸沿路访问渔具店，07:00–18:00营业；卖鱼换coins并购买装备、饵料和容器。初始小木桶10 kg，可升级20 kg大木桶和35 kg储鱼箱。

   Disembark at the left dock and follow the path to the tackle shop, open 07:00–18:00. Sell fish for coins and buy tackle, bait and containers. The initial 10 kg bucket upgrades to a 20 kg bucket or a 35 kg fish box.
4. **稀有变异 / Rare variants**：黄化土鲶、长鳍马口、红色长尾鲤、白化鳖，在选中对应基础鱼后独立抽取1%；彩色外框、独立收藏/库存、普通计算价格×10，并有商店专属对白。

   Xanthic catfish, longfin hooksnout minnow, red longtail carp and albino softshell each have a conditional 1% chance after their base species is selected. They have rainbow frames, separate collection/storage records, ten times the ordinary calculated price and special shop dialogue.

## 设备与操作

240×240 RGB565，单颗双向旋转、可按下的编码器。旋转移动/调整，短按打开行囊或确认；长按280 ms开始蓄力、1.4 s满蓄力，松开抛竿。漂相进入可提竿窗口后按下，搏鱼时顺转收线、逆转泄力。成功收鱼才揭晓鱼种、尺寸、估算重量、星级和评语。

背包类别顺序为鱼竿、线钩、浮漂、鱼饵、钓深、储鱼容器，随后是图鉴、存档、返回。底部菜单按屏幕左到右切换，原菜单和装备索引不重排。金币图标及余额在湖面、岸上、行囊、商店、图鉴、鱼获卡和容器页持续显示，最大余额1,000,000,000。只选择已拥有的装备与有库存的特殊饵料。

## 地图、背景与场景

水域x=56–904，船中心132–828，左右码头可上下船；13节点虚拟水深0–6m，五分区为芦苇浅湾、开阔水面、泥底深槽、缓流入口、浅岸砾石。左岸延伸到x=-284，商店入口x=-260，入口±16像素短按进入。船与水域碰撞坐标不变，岸上相机可到-336，经过小路/码头世界x=0时连续移动。

原湖景的山、森林、岸、水面保留，夜空在原山林轮廓上方转为接近黑的深蓝，少量星星和圆润月牙随原视差移动；日月被远山和树冠遮挡。生成小路使用同样2像素颗粒，和西岸重叠24像素，码头在上层盖接；外屋也用2倍粗像素。渔具店为柜台近景与放大的老人上半身，保留四种表情和14种情境各两句台词，老人位于柜台后方，原柜台前沿遮挡人物下缘；左侧显示五行滚动商品列表，旋转浏览16个稳定条目，列表保留文字与选中高亮，底部左下角以2倍整数缩放显示选中装备图标，右侧按名称/售价、描述、实际效果分三行显示详情。重绘粗像素背景，壁灯、线轴和浮漂挂在墙上，柜台只有窄台沿与正面木板，没有深斜桌面。

## 鱼种、钓层与活跃时间

保留11种鱼。按落点、水深、钓层、分区、饵料和游戏时间筛选权重；趴地虎、溪蟹仅近岸底层，土鲶仅底层。默认钓到底，行囊可选表层、0.5/1/2/3/4.5/6m。铜/银/金70/25/5为基础比例，实际局部比例随条件变化；卡面稀有度不变，体型星级按物种独立评级。

活跃度是相对出现权重的百分比，100为基础值；这是虚构湿地的游戏数值，不是自然调查结论。5–7点与18–20点按小时线性插值，6点/19点达到晨昏系数，7–18点使用白天系数，20–5点使用夜间系数。午夜连续，非活跃时段仍有机会；不突破水域/钓层/饵料限制。**斑鳜没有夜间偏好：白天和夜间都是100，只在晨昏轻微增加。**

| 鱼种 | 白天 | 晨昏峰值 | 夜间 |
|---|---:|---:|---:|
| 鲫鱼 | 100 | 125 | 90 |
| 鲤鱼 | 100 | 130 | 85 |
| 马口鱼 | 130 | 115 | 60 |
| 土鲶 | 65 | 125 | 155 |
| 子陵吻虾虎鱼（趴地虎） | 115 | 110 | 80 |
| 黑鱼（乌鳢） | 125 | 130 | 65 |
| 黄颡鱼 | 65 | 130 | 155 |
| 䱗（餐条／白条） | 135 | 110 | 55 |
| 斑鳜 | 100 | 115 | 100 |
| 中华鳖 | 85 | 110 | 130 |
| 溪蟹 | 70 | 125 | 145 |

先用环境基础权重乘时间系数，再进行原来的物种/体型抽样。切换时间不改变星级阈值、体型范围、重量公式或售价公式；当前一竿的候选在抛竿时确定，等待期间经过时段边界不重新选鱼。活动表保存在 `data/species.json` 的 `game_distribution.activity_percent`。

## 等待、夜光漂与搏鱼

基础咬钩轮次由2600改为2860 ms，最多22轮，最久62,920 ms。按线钩隐蔽性和候选鱼重量抽取实际等待，保留原900 ms试饵阶段；一般等待比旧版略长约10%。

夜光漂夜间发明亮蓝绿色光，环境压暗后绘制，附轻微光晕，只显示露出水面的部分；白天维持原色。完全入夜时每轮真实咬钩概率相对提高15%，例如70%→80%（整数向下取整），上限100%，晨昏随暗度渐变。这个加成缩短同等条件下的等待，不改变鱼种权重或提竿窗口。鹅毛漂独立把线组修正后的可提竿窗口延长15%，试饵延迟不变，白天和夜间都有效；红白漂没有额外加成。

搏鱼仍以20 ms整数步进更新张力、松线、距离、爆发与体力；竿影响抛距/收线，线钩影响强度/距离/窗口/隐蔽性。v22线长为细线4.5米、通用6.5米、强韧8.5米，抛竿/预览同步受线长限制；最大出线后不能继续放线。鱼冲刺可产生10%以内的竿身弹性超距，竿形随之弯曲；持续超距2000 ms断线，收回标称范围立即解除计时。每次爆发约70%远离玩家、30%靠近玩家；鲤鱼、马口、斑鳜位移系数135%、140%、130%，其他基础鱼100%，变异继承基础鱼。尺寸、售价、星级和变异概率不变。

Line lengths are 4.5/6.5/8.5 m for fine/general/heavy tackle, limiting both actual casts and previews. Manual payout stops at nominal length. Fish surges can use up to 10% elastic rod excess; continuous excess for 2,000 ms breaks the line, while retrieving into nominal range clears the timer. Each burst is about 70% away from the player and 30% toward the player, with movement factors of 135%/140%/130% for carp/hooksnout minnow/mandarin fish and 100% for other base species. Variants inherit their parents. Size, prices, stars and rare probabilities are unchanged.

## 鱼获容器与船尾

初始小木桶10 kg，可升级大木桶20 kg和储鱼箱35 kg。按鱼种汇总数量、重量和估价，容量内恰好装满有效；单条超过当前总容量才挂船尾，限一条。普通鱼没有剩余空间时放流，仍计图鉴；第二条超大鱼放流。升级保留所有鱼获，船尾鱼若可放入新增剩余空间则自动收纳。

鱼获页用压暗容器作背景，鱼图在前景，鱼名/数量/重量/价值放在独立暗色底板，避免纹理干扰。仅显示当前存有鱼获的鱼种，按稳定鱼种索引紧凑排序；船尾有鱼时追加最后一页，没有鱼获时显示统一空库提示，页码随实际页数变化。大鱼按朝向约30°斜挂，绳索从船尾固定点连接鱼头/鳃部，左右朝向对称。

## 货币与商店

营业时间07:00 AM–06:00 PM（07:00–18:00），外屋正面亮灯/熄灯两种状态表示营业，不再挂牌。尝试进入关门商店时，主角上方显示“打烊了”1.6秒。打烊无人且无法进入或交易，18点退出店内，查看容器后也不能返回已打烊的店；不增加打烊对话。店主台词与表情可在 `docs/merchant-dialogue.md` 修改，装备描述草稿在 `docs/tackle-descriptions.md`。

币重分开：coins为货币，g/kg为重量。单鱼售价为 `max(1, floor((base * size_mm² + typical_mm²/2) / typical_mm²))`，`typical_mm=mean_x2_mm/2`。鱼种基础价仍为白条6、鲫鱼8、鲤鱼12、土鲶12、溪蟹10、马口16、趴地虎16、黑鱼28、黄颡鱼30、中华鳖60、斑鳜72。出售一次结算容器及船尾全部鱼，不清图鉴；超过金币上限整笔拒绝。

| 商品 | coins | 交付 |
|---|---:|---|
| 玻纤竿 | 550 | 永久解锁并装备 |
| 碳素竿 | 1800 | 永久解锁并装备 |
| 细线小钩 / 强韧大钩 | 35 / 100 | 永久解锁并装备 |
| 鹅毛漂 / 夜光漂 | 90 / 120 | 永久解锁并装备 |
| 蚯蚓 / 米饭 / 面饵 / 小鱼 / 虾饵 | 8 / 6 / 10 / 18 / 20 | 每包五份 |
| 大木桶 /20 kg | 300 | 替换低级容器 |
| 储鱼箱 /35 kg | 750 | 替换低级容器 |

初始竹竿、通用线钩、红白漂和无限普通假饵；特殊饵料每次抛竿消耗一份，即使收回也消耗，最后一份用完后下一竿自动用普通假饵。重复永久装备、容器降级、余额不足、库存超过999均整笔拒绝。价格唯一契约见 `data/economy.json`，C表应同步维护。

## 时间、存档、工程与发布

系统单调毫秒驱动，每1秒现实时间为1分钟游戏时间，一昼夜24分钟，05–07渐亮、18–20渐暗。新游戏起始06:00加运行秒数，读档恢复游戏分钟，退出期间不快进。毫秒余量与uint32回绕安全。

存档v7为1317B，读取v1–v6；声音字段保持偏移260，新增1024B单鱼售价分组记录，保持原基础字段、密钥、UUID派生与目录。V4装备保留，更早版本沿用一次初始化标记。包含四种稀有图鉴和实物记录；V7使用独立A/B文件，V6、V5及原文件只读保留，避免切回旧版覆盖进度。后台写非活动槽、同步并读回确认，不操作背光PA26。完整布局和文件名见[存档说明](save-system.md)。

标准工程默认 `FISH_LCD_ROTATION=0`，对应ST7789由原2转向180°，240×240窗口按驱动使用Y_SHIFT=80；其他工程未指定时保持默认2。常用Embedded Studio工程在 `hpm5301evklite_flash_uf2_debug/segger_embedded_studio/mklink_sample_fishing.emProject`，选择Debug。方向与帧率仍需实板确认。

公开仓库只保留实际运行链接的 C 位图/蒙版/字形及必需许可；原图、备用版本、PNG/图集、TTF、提示词与预览均留本地。没有新增整屏缓冲。Windows39项CTest、标准demo11项测试、GNU UF2/XIP及SEGGER构建，具体结果见验证报告。尚未实板烧录、掉电或栈水位验证，也未创建Android/微信运行应用。

商店素材统一正面、较平面的构图。外屋64×48逻辑像素加采样边框，整数2倍显示约128×96；原图重新生成到目标网格，门框约38像素，略高于36像素主角。营业亮窗开门，打烊暗窗关门；两个状态共用轮廓，熄灯状态只导出实际变化的门窗区域，避免整屋跳动及重复占用Flash。内景120×72、店主每帧40×50，全部2倍最近邻显示；柜台水平前沿遮挡人物下缘。商品改为五行文字列表，已移除九组独立商品摆拍位图及对应变换操作。背景保留RGB565灯光色，不量化丢失壁灯亮部。

左岸转动一次即以32像素/秒持续慢走，反向转动先停住，再转一格反向起步；短按停止并执行交互。走到商店门口、西码头上岸点或岸边界自动停止，菜单中不会继续行走。单帧补算上限250 ms，避免卡顿后的瞬移；计时支持uint32回绕。地面使用生成的48×46逻辑像素草地/土路贴图，2倍显示并连续铺满左岸下方，店铺下不再透出湖水。

六条竿/漂描述由 `docs/tackle-descriptions.md` 导出到运行界面，店主台词由 `docs/merchant-dialogue.md` 导出，新增汉字同步位图字库。价格和存档格式不变。

码头交互替代行囊上下船按钮：船抵达码头自动停止，再向岸方向转动一格上岸并开始慢走；步行回到码头自动停止，再向水方向转动一格登船，船保持停泊。码头方向操作保持，去掉顺逆方向上船提示。菜单仅保留图鉴、存档、返回；原DOCK菜单ID保留为内部空位，未序列化进存档。

收鱼卡显示当前鱼的出售价格（SELL＋金币图标），已解锁图鉴显示该鱼最佳纪录尺寸对应的价格（BEST）；未解锁项不显示售价。鱼库按每种实际存货汇总价值，商店“出售鱼获”显示整批售价。价格展示统一调用实际结算函数，不另外维护估价表。

岸上小人按船上渔夫身份重绘：棕色钓鱼帽、绿外套、深蓝裤，三帧14×20像素含采样边，内容高18像素，以2倍显示为36像素，与船上人物使用同样的2像素颗粒。站立帧0；移动每6像素在接触/经过两帧间交替，停住立即回站姿。共享裁切与脚底基线，左右镜像，步态计数不进存档。详情图标复用已批准素材缩为14×14并加1像素采样边，2倍显示为28像素内容，RGB565共7168B，无新增原图发布。

接岸修正v19：码头接岸段按原土路重绘，土路在世界x=0处延伸到码头下24像素，以同网格A8蒙版重叠过渡，避免竖直碎石断面。东岸镜像共用资源并有地面衬底。码头仅存56×47逻辑像素与1像素边框（58×49，RGB565/A8共8526B），运行2倍显示，替代原112×94展开数组，节省23058B像素载荷。脚底统一y=134，岸边与木板不再跳高；提示框移到放大人物头顶上方。左岸镜头最小值-336保证完整显示放大商店，门口对齐人物。43幅新PFB捕获覆盖两岸、边界移动、昼夜、上下船及商店营业/打烊比例。

## 稀有变异、云层与停步 v20

11种基础鱼外另有黄化土鲶、长鳍马口、红色长尾鲤、白化鳖，选中对应基础鱼后独立抽取1%；成功起鱼后记录为四个新索引11–14，单独图鉴/库存，彩色外框，出售价格为同尺寸普通鱼计算并取整后的10倍。携带时有四类专属招呼，普通进店5%提示收购线索。共14类28句台词，原稿保留；详见[稀有变异设定](rare-variants.md)。

行走中反向一格先停止，第二格再反向走；停在码头后另转一格上下船，不显示顺向上船之类提示。云重绘为舒展的薄云，90×10逻辑像素加边框（3312B）整数2倍显示，以原山线裁切，始终位于山后，不占用额外整屏缓存。

存档升级为v5（260B载荷、292B文件），前204B基础字段不重排，追加四种稀有进度与库存。读取v1/v2/v3/v4，V4装备保持；继续使用原密钥/UUID/目录。新V5_A/B槽与原A/B文件分开，原文件只读保留，有效V5优先加载，防止切回旧版覆盖稀有鱼记录。

## 大鱼细节 v21 / Large-fish detail v21

黑鱼、鲤鱼、土鲶各有48×28普通贴图与96×56细节贴图。收鱼卡和已解锁图鉴按实际显示比例选择：超过原96×56参考画布的1倍比例时使用细节版，较小时仍用原版，体型与屏幕显示尺寸不变。细节从已批准的原始母图重新采样，保留16色RGB565像素风、二值透明蒙版、原朝向和外形比例；不对低清贴图插值造细节。小鱼、鱼库和船尾保持原显示颗粒，四种稀有变异及所有数值规则不变。

Snakehead, carp and catfish each have a 48×28 regular tile and a 96×56 detail tile. Catch cards and unlocked guide entries select detail art when the display scale exceeds 1× the original 96×56 reference canvas; smaller presentations retain the original pixels and all on-screen sizes stay unchanged. Detail is resampled from the approved masters, retaining the 16-color RGB565 pixel style, binary alpha, facing and proportions. Small fish, storage and stern fish retain their original pixel density; rare variants and gameplay values are unchanged.

11种基础鱼取消预先展开的2倍重复像素，统一以48×28原生贴图绘制；释放133,056 B，三种细节贴图增加48,384 B，位图净节省84,672 B（约82.7 KiB），另有少量资源表与绘制代码变化。沿用Arm-2D/PFB变换操作，每帧准备一次、分块复用，不增加整屏缓冲、运行时解码或堆分配。最终Flash/RAM占用见`validation.md`。

The 11 base sprites now store native 48×28 pixels instead of duplicated 2× pixels, recovering 133,056 bytes. Three detail tiles add 48,384 bytes, for a net bitmap saving of 84,672 bytes (about 82.7 KiB), plus small table/code changes. Existing Arm-2D/PFB transforms are prepared once per frame and reused across blocks, with no additional full-screen buffer, runtime decoder or heap allocation. Final Flash/RAM usage is recorded in `validation.md`.

## 汉化、视差与内容核实 v22 / Localization, parallax and content audit v22

收鱼卡与鱼库的鱼名、尺寸类型/长度/重量、状态/水深/钓深、售价/纪录、商店价格/已拥有和装备数值说明已补齐中文。v23统一恢复原有12像素中文字体；撤销v22新增的8像素小字库，价格、鱼获数值及状态栏通过对齐和间距调整适配，不依赖运行时字体文件。左岸使用连续有符号远景视差、原图边缘镜像延展，云每900 ms漂移一像素并保持山后遮挡，无新增背景位图。

Fish names, metrics, states, depths, prices, records, ownership and tackle properties are localized using the original 12-pixel Chinese font throughout in v23. The v22 8-pixel subset is removed; price, catch metrics and HUD rows use adjusted alignment and spacing. Signed shore parallax reflects the existing panorama edge; clouds drift one pixel per 900 ms and remain behind the skyline, with no new background bitmap.

11种基础鱼各20万次实际抛竿选择，共220万次，全部15个收藏项均能抽中并完成起鱼/图鉴/售价记录。趴地虎未被条件误排除：右岸浅水虾饵钓底的专门匹配条件下抽中136363次；这不是全局概率。店主14类情境、28句台词和四种立绘的逻辑与实际PFB画面均核对通过。完整鱼种抽样、线长/竿身弹性、双向挣扎与界面规则见[v22玩法与核实](v22-gameplay-and-audit.md)。

The audit runs 200,000 actual cast selections for each of 11 base species (2.2 million total); all 15 collection entries are selected and pass landing, guide and price checks. Goby is reachable, with 136,363 selections under its targeted right-bank shallow/shrimp/bottom profile, rather than a general probability. All 14 merchant contexts, 28 lines and four portraits pass logic and actual PFB checks. The linked v22 audit documents complete results and current rules.

## 字号与提示修正 v23 / Font and hint correction v23

全部中文统一使用原有12像素字体；移除v22新增的8像素字库，紧凑栏通过对齐和间距调整适配。移除“鱼竿弯了！快收线”提示；弯竿表现、两秒弹性窗口及搏鱼规则不变。蟹的显示名改为“溪蟹”，土鲶去掉真实水域别称，地图设定不指向真实地区；物种索引和存档不变。

All Chinese text uses the original 12-pixel font. The v22 8-pixel subset is removed, with alignment and spacing adjustments for compact rows. The misleading reel-now message during rod flex is removed; rod animation, the two-second elastic window and fight rules are unchanged. Crab uses the generic display name “溪蟹” (stream crab), catfish loses its geographic alias, and the setting is a fictional wetland. Species indices and saves are preserved.

## 蜂鸣器反馈 v24 / Buzzer feedback v24

成功提竿上钩播放160 Hz/60 ms低频短脉冲；鱼实际挣扎时播放110–170 Hz不规则低频短震，每次20–50 ms、静音间隙20–70 ms，体力恢复和被动鱼种安静。成功买卖播放660→1100 Hz的上扬收银音，40 ms首音、10 ms间隔、80 ms尾音；失败交易静音。退出搏鱼或商店立即停止，使用既有PWM及单次定时器，不阻塞绘图或输入。所有音符占空比20%，详见[蜂鸣器参数](buzzer-feedback.md)，实板声音/震感待试听。

Successful hooks play a 160 Hz/60 ms low-frequency pulse. Active bursts play irregular 110–170 Hz pulses lasting 20–50 ms with 20–70 ms silent gaps, with recovery and passive species silent. Successful trades play an ascending 660→1100 Hz cashier cue (40 ms, 10 ms gap, 80 ms tail); failed trades stay silent. Leaving the fight/shop stops playback. Existing PWM and one-shot timers avoid blocking drawing/input; all notes use 20% duty. See the linked parameters; physical sound/vibration audition is pending.

## 声音设置与两页图鉴 v25 / Sound settings and two-page guide v25

行囊底部按图鉴、声音、存档、返回排列。声音提供关闭／仅鱼讯／全部开启三档，短按进入调整、旋转选择、短按确认并请求保存；调整立即生效。声音档位写入v6认证存档末尾偏移260，文件由292增至293字节，旧v1–v5迁移默认全部开启。保留原密钥、目录、索引和所有旧文件，优先V6、其次V5，再读取更早版本。

The bag footer reads guide, sound, save and return. Tap sound to edit, rotate to choose off/fish-only/all, and tap to confirm/save; changes apply immediately. Sound is authenticated in v6 at payload offset 260, increasing files from 292 to 293 bytes. v1–v5 migration defaults to all sounds. The key, directory, indices and all old files remain; loading prioritizes v6, then v5, then earlier versions.

图鉴第一页保留鱼图、纪录和评语；第二页用四行12像素文字显示时段、水域、钓层和推荐鱼饵，不显示多少米的范围。内容从实际地图/鱼饵/活跃配置生成，稀有变异继承基础鱼；未收集只显示简短线索。旋转切鱼，短按翻页，长按600 ms返回来源页面；进入时默认鱼卡页。斑鳜提示全天、晨昏稍好，不偏夜间。

The first guide page retains portraits, records and remarks. The second shows four 12-pixel rows: activity, area, layer and recommended bait, without numeric depth ranges, derived from actual game rules. Variants inherit their base rules; locked entries show a short hint. Rotate to select fish, tap to change pages, and hold 600 ms to return. Entry starts on the portrait page. Mandarin fish remain day/night-neutral with a mild dawn/dusk preference.

本次不增加背景位图；构建检查要求至少保留64 KiB应用Flash供后续地图页面，地图页本身尚未实装。SD多地图素材仍只保留规划。

No background bitmaps are added. Build checks reserve at least 64 KiB of application Flash for the future map page, which is not implemented yet. SD multi-map assets remain a recorded plan.

## 后续规划，暂不执行 / Future plan, not implemented

多地图扩展时，可将地图位图素材存放在SD卡上，进入或切换地图时花费几秒读取所需资源，再供场景绘制使用。允许显式加载等待，以减少应用Flash对地图数量的限制。后续需要核算资源缓存与RAM预算，并协调现有SD存档访问。本次只记录方案，不实现SD素材存储、加载器或加载画面。

For a future multi-map expansion, store map bitmaps on the SD card and allow a few seconds to load the required assets when entering or switching maps. An explicit loading period can reduce the limit imposed by application Flash capacity. Cache/RAM budgets and coordination with SD save access will need evaluation. This is a recorded proposal only; SD asset storage, loaders and loading screens are not implemented in this update.

## 交易金币浮字 v26 / Transaction popups v26

成功购买立即显示暖色 `-550G` 等扣款，成功出售显示浅绿色 `+11G` 等收入。出售第一条立即弹出，其余每250 ms一条；每条向上移动48像素、存续900 ms，末250 ms淡出，最多同时显示四条，放在商店柜台右侧。余额和鱼获在交易成功时一次结算并请求保存，动画不延迟结算；空鱼库、余额不足、已拥有、库存满或打烊时没有金币浮字。旋转和购买可继续操作，购买不清掉待显示的出售明细；离店清除剩余视觉队列。

新鱼获按真实单条售价记录，等价售价合并计数以节省RAM与SD空间，出售时仍按条弹出，包含稀有鱼和船尾大鱼。最多256种不同售价；枚举当前全部合法尺寸，即使为每种售价选择最轻鱼，35 kg容器最多219种，不减少鱼库容量。扩容后船尾鱼入库也保留单价。售价记录保存于v7，重启后继续准确显示。旧v1–v6鱼库只有物种合计，迁移时保留全部数量/重量/金额；缺失的明细显示一次“旧鱼获合计”，不虚构单价，新鱼和已知船尾鱼仍逐条显示。

Successful purchases immediately show a warm `-550G` charge; sales show pale-green `+11G` income per fish. The first sale popup is immediate, followed by one every 250 ms. Each rises 48 pixels over 900 ms, fading over its last 250 ms, with at most four visible over the right counter. Balances and cargo settle atomically and request a save before the animation finishes. Failed transactions produce no popup. Inputs remain active, purchases preserve the pending sale sequence, and leaving the shop clears only cosmetic feedback.

New catches retain exact individual prices, with equal prices sharing a count. Selling expands these counts into individual popups, including rare and stern catches. All valid sizes give a conservative maximum of 219 different prices in the 35 kg hold, below the 256-group capacity. The v7 save persists these records. v1–v6 cargo has only species totals: migration preserves all cargo and money, and clearly labels its unrecoverable details as one legacy total. New catches and the known stern fish still show exact individual amounts. No bitmap, heap allocation or framebuffer is added.

## 不规则挣扎反馈 v27 / Irregular struggle feedback v27

挣扎音效改为私有随机短脉冲，起音间距40–120 ms（受线程更新延迟影响），每一下同时改变频率、时长和间隙，避免机械节拍。音效随机数与鱼种、尺寸、搏鱼和运动随机数独立，存档格式仍为v7/1317 B。实际冲刺以外安静、卡顿不补播、收银与上钩提示保留；细节见[蜂鸣器反馈](buzzer-feedback.md)。当前重量理论上限为1米鲤鱼／红色长尾鲤16,461 g；1.2米土鲶／黄化土鲶15,170 g。

Struggle feedback now varies pitch, duration and silence using a private random state. Pulse onsets are 40–120 ms apart before thread delays. Fishing/size/fight/motion randomness and the v7/1317-byte save remain unchanged. Actual lulls stay silent and stalls do not queue audio. The current theoretical weight maximum is 16,461 g for a 1 m carp/red longtail carp, followed by 15,170 g for a 1.2 m catfish/xanthic catfish.
