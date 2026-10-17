# mklink_sample_fishing / 北方湿地钓鱼 / North Wetland

基于同仓库 `mklink_sample_fire` 标准工程的独立钓鱼 demo。游戏使用的硬件平台为 **MKLink V4**，主控为HPM5301，搭配240×240 RGB565 ST7789屏幕和一颗双向旋转、可按下的编码器。

An independent fishing demo based on this repository's `mklink_sample_fire` project. The hardware platform is **MKLink V4**, with an HPM5301 MCU, a 240×240 RGB565 ST7789 display and one rotary encoder with a push button.

## v29 首次运行 / First boot

将 `sdcard/FishingGame_Resources_DO_NOT_DELETE/` 整个目录复制到SD卡根目录，再安装本目录 `MicroLink.uf2`。保留原 `FishingGame_Saves_DO_NOT_DELETE/` 存档目录。背景和鱼类贴图已移入SD资源包，首次启动与切换固件后缓存失效时需读卡加载；有效缓存启动只校验、不擦写。

Copy the complete `sdcard/FishingGame_Resources_DO_NOT_DELETE/` directory to the SD root, then install `MicroLink.uf2`. Keep the existing save directory. Background/fish art now uses SD packs; first boot and invalidated caches require an SD reload. Valid caches are checked and reused without erasing.

右岸靠近车站短按打开路线图，旋转选湿地湖泊/城市江河，短按乘车，长按600 ms返回。**每程180G，耗时5个游戏小时**，加载失败不扣费。城市有现代天际线、远处轮渡、气垫筏、夜间灯光层与动态射灯；完整说明见 [地图与SD资源 / Maps and SD resources](docs/maps-and-sd-resources.md)。

Tap at the east-bank station to open the route, rotate to choose a location, tap to travel and hold 600 ms to return. **Each trip costs 180G and five in-game hours, both directions.** Failed loads are free. The city has a modern skyline/ferry, raft, emission layer and moving spotlights.

## 当前玩法 v29 / Current gameplay v29

1. **鱼饵吸引力与装备效果 / Bait attraction and tackle effects**

   蚯蚓、米饭、面饵、小鱼、虾饵和普通假饵分别具有针对11种基础鱼的相对吸引权重，并结合落点、水域、水深、钓层和活跃时段选择鱼种。线钩影响等待和搏鱼，鹅毛漂延长15%提竿窗口，夜光漂在完全入夜时使每轮咬钩概率相对提高15%；装备效果分别计算。

   Worms, rice, dough, baitfish, shrimp and the reusable plastic lure have distinct attraction weights for the 11 base species. Selection also considers cast position, habitat, depth, fishing layer and activity time. Line/hook sets affect waiting and fighting; the feather float extends the hook window by 15%, while the luminous float gives a relative 15% bite-chance bonus per attempt at full night. These tackle effects are calculated separately.

2. **游戏时间与昼夜 / Game time and day–night cycle**

   现实1秒对应游戏1分钟，一昼夜24分钟；05:00–07:00渐亮、18:00–20:00渐暗，夜晚呈深蓝天空、星星和月亮，日月与云被远山遮挡。不同鱼种随时段改变活跃权重，斑鳜白天与夜间权重相同。存档恢复游戏时间，退出期间不快进。

   One real second equals one game minute, making a full day 24 real minutes. Dawn runs from 05:00 to 07:00 and dusk from 18:00 to 20:00, with a dark blue night sky, stars and a moon. Mountains occlude the sun, moon and clouds. Species activity varies by time; mandarin fish have equal daytime and nighttime weights. Saves restore game time without offline advancement.

3. **经济、左岸商店与船上鱼库 / Economy, left-bank shop and boat storage**

   在左侧码头停稳后继续向岸旋转即可上岸，沿小路到渔具店，营业时间07:00–18:00。出售鱼获获得coins，购买鱼竿、线钩、浮漂、消耗饵料与扩容容器。船上初始“小木桶”10 kg，大木桶20 kg／300 coins，储鱼箱35 kg／750 coins；单条超过容器总容量的巨鱼可挂船尾，限一条。鱼库只显示当前实际存有的鱼。

   Stop at the left dock, then rotate toward the bank again to disembark and follow the path to the tackle shop, open from 07:00 to 18:00. Sell catches for coins and buy rods, line/hook sets, floats, consumable bait and larger containers. Boat storage starts with a 10 kg wooden bucket; upgrades are a 20 kg large bucket for 300 coins and a 35 kg fish box for 750 coins. One fish exceeding the container's total capacity can hang from the stern. Storage pages show only fish currently held.

4. **1%稀有变异 / 1% rare variants**

   黄化土鲶、长鳍马口、红色长尾鲤和白化鳖，在选中对应基础鱼后各有独立1%概率出现，并非所有抛竿的1%。稀有鱼有彩色外框、独立图鉴与鱼库记录，售价为同尺寸普通鱼计算结果的10倍；携带进店有专属台词，日常对话也有小概率给出收购线索。

   Xanthic catfish, longfin hooksnout minnow, red longtail carp and albino softshell each have a conditional 1% chance after their corresponding base species is selected, rather than 1% of all casts. Rare catches have rainbow frames, separate collection/storage records and ten times the ordinary sale price at the same size. Carrying them into the shop triggers special dialogue, with occasional buyer hints during ordinary visits.

数值与规则见 [整体设定 / Full design](docs/game-design.md)、[鱼饵权重 / Bait weights](docs/bait-design.md) 和 [稀有变异 / Rare variants](docs/rare-variants.md)。游戏还包括蓄力抛竿、漂相提竿、张力/体力搏鱼及SD手动/五分钟自动存档。

See the linked documents for detailed values and rules. The game also includes charged casting, float cues, tension/stamina fighting, and manual SD saves plus autosaves every five real minutes.

## 获取与构建

仓库依赖仍使用原来的 Arm-2D、notGUIyet、perf_counter 子模块：

```sh
git submodule update --init --recursive
```

已验证工具链为 HPM SDK 1.11.0、GNU RISC-V GCC 13.2 和 SEGGER Embedded Studio 8.24。Windows SDK 整合包可执行：

```powershell
./mklink_sample_fishing/scripts/build.ps1 -SdkRoot <SDK整合包目录>
./mklink_sample_fishing/scripts/build.ps1 -SdkRoot <SDK整合包目录> -BuildType flash_xip
```

也可按标准 SDK 方式设置 `HPM_SDK_BASE`、`GNURISCV_TOOLCHAIN_PATH` 并构建：

```sh
cmake -S mklink_sample_fishing -B mklink_sample_fishing/build -G Ninja \
  -DBOARD=hpm5301evklite -DBOARD_SEARCH_PATH=<仓库绝对路径> \
  -DHPM_BUILD_TYPE=flash_uf2 -DCMAKE_BUILD_TYPE=Debug
cmake --build mklink_sample_fishing/build
```

GNU 输出位于构建目录 `output/`。`MicroLink.app.bin` 是去除配置头和 boot header 的应用净载荷，UF2 从 **0x80023000** 开始，Family ID 为 **0x0A4D5048**；不能直接拿带头的 XIP 原始 bin 按该地址转换。

构建同时生成 `segger_embedded_studio/mklink_sample_fishing.emProject`。在 SEGGER 打开后构建 Debug/Release，或使用其 `emBuild`。GNU 和 SEGGER 均自动导出 demo 根目录的 `MicroLink.uf2`。仓库附带的 UF2 使用 GNU `flash_uf2` 构建；它只包含游戏应用区域，不包含下载器引导程序。

本工程保留标准模板的板卡初始化、RT-Thread、LCD SPI0/DMA、ref_gui 和编码器驱动结构；原 fire demo 与子模块未修改；公共 ST7789 头文件仅允许工程覆盖旋转参数。游戏有自己的输入适配和配置，按键初始化在消息池建立后进行，计时使用实际毫秒，避免漏按压/长按与 RT-Thread 定时器冲突。

## 单编码器操作

- 旋转控制横向移动；岸上转一格持续慢走，反向一格先停住，再转一格反向走，短按停止并交互。
- 短按打开行囊，旋转选择装备/图鉴/存档/返回；短按选择或确认。
- 长按蓄力，松开抛竿；出现可提竿漂相时按下。
- 搏鱼时顺转收线、逆转放线，观察张力与体力。

画面使用编码器用语，存档按钮没有额外操作教学。若实物编码器方向相反，可在 CMake 加 `-DFISH_ENCODER_DIRECTION=-1`（默认 `1`）。

## SD 存档与兼容性

SD 卡文件目录为：

```text
FishingGame_Saves_DO_NOT_DELETE/
  NorthWetland_V8_Save_A_DO_NOT_DELETE.sav
  NorthWetland_V8_Save_B_DO_NOT_DELETE.sav
  NorthWetland_V7_Save_A/B_DO_NOT_DELETE.sav  # 旧版保留 / keep older files
  NorthWetland_V6_Save_A/B_DO_NOT_DELETE.sav
  NorthWetland_V5_Save_A/B_DO_NOT_DELETE.sav
  NorthWetland_Save_A/B_DO_NOT_DELETE.sav
  README_V8_DO_NOT_DELETE.txt
```

当前写v8，每槽1318B，独立A/B，保存当前地图、图鉴、装备、金币、鱼库、单鱼售价、时间和声音。兼容v1–v7，全部旧文件只读保留，优先当前版本。旧档默认为湿地，已初始化的高级装备不再重置。**按作者要求，保留并公开原密钥、设备UUID派生与目录。** 写非活动槽并同步、关闭、读回校验后才确认成功；切换固件前手动保存并等待“已存档”。

Version 8 uses separate 1318-byte A/B files and saves location, collection, gear, coins, cargo/prices, time and sound. Older v1–v7 files remain read-only. The existing key, UUID derivation and directory are preserved and public as requested. An inactive slot is synced, closed and read back before success is shown. Save before switching firmware.

默认不启用USB MSC/CDC；存档线程和资源加载器通过递归互斥锁共享FatFS。密钥公开后不能提供强防作弊；建议定期备份整个存档目录。细节见[存档说明](docs/save-system.md)。

SD 使用 SPI1，LCD 使用 SPI0。**PA26 是 LCD 背光，不能当软件 SD 片选切换。** 通用 board.h 将 `BOARD_SPI_CS_PIN` 定义为 PA26，但 MKLink 原 SD 适配器的片选回调为空；此 demo 沿用该行为，不配置或切换 PA26，修复了存档时背光频闪。

## 验证与维护

游戏逻辑测试可在桌面 C11 编译器上执行，无需 HPM SDK 或 SDL：

```sh
cmake -S mklink_sample_fishing/tests -B mklink_sample_fishing/build-tests
cmake --build mklink_sample_fishing/build-tests
ctest --test-dir mklink_sample_fishing/build-tests --output-on-failure
```

14项独立主机测试与48项模拟器检查通过，覆盖原玩法、声音/图鉴/交易/存档，以及SD资源CRC/版本/布局/长度、预检失败不擦写、中断与重试、双向车费/5小时时钟和输入。GNU与SEGGER构建、Flash分区和UF2校验见[验证报告](docs/validation.md)。未执行实板烧录、拔卡或掉电测试。

`src/fish/` 包含游戏逻辑与已生成的 RGB565/A8/A1 素材，常规构建不需要素材转换工具。`data/` 和 `docs/` 保存数值/说明；运行使用已生成的 `fish_*_data.c`、`fish_rules.h` 和 `fish_map_config.h`，修改 JSON 后需同步这些 C 数据。新增文字需要重新导出中文字体子集。

第三方代码与字形的许可说明见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

## 鱼库、商店与昼夜

初始装备为竹竿、通用线钩、红白漂和无限普通假饵（“塑料味”）。普通鱼获收入初始“小木桶”（10 kg），可购买大木桶（20 kg / 300 coins）和储鱼箱（35 kg / 750 coins）。单条超过当前容量才挂船尾，限一条；装不下时放流，仍记图鉴。行囊显示当前容器名称和实物，打开后以容器背景查看鱼获。升级保留全部鱼，并将能装下的船尾鱼收入容器。

在西码头下船后向左沿小径走，靠近木屋短按进入商店。旋转浏览五行商品列表，底部详情显示选中商品的名称、价格和容量；短按交易，出售全部鱼获换取 coins，购买渔具永久解锁，特殊饵料每包五份/每次抛竿消耗一份；钱不足、重复装备、库存满均不扣钱。“返回小径”退出，再回码头登船。具体价格和规则见 [整体设定](docs/game-design.md)。

游戏时间由系统单调毫秒驱动，每秒折算一分钟，一昼夜 24 分钟；日夜渐变、太阳/月亮移动和 12 条水面反光不增加整屏缓冲。读取存档恢复游戏分钟，退出期间不快进，尚未实测硬件帧率。

公开仓库只提供 `src/fish/assets/` 中使用的共享C位图/蒙版/字形、地图tile描述、`sdcard/`中的实际运行资源包和必需的OFL许可。生成原图、未采用版本、PNG/图集、字体源文件、提示词和截图不随仓库提供。正常构建无需素材转换工具，执行 `python scripts/verify_public_assets.py` 可校验发布范围。

## 柜台、夜景和操作 v17

店内采用柜台近景和放大的老人上半身，旋转浏览商品，左侧为五行滚动列表，底部显示价格和详细信息；仍为 16 个稳定商品索引。位置见 `data/shop_layout.json`，`python tools/prepare_shop_layout.py` 可导出 C 坐标。夜景只替换原背景天空，月亮/星星按原山林轮廓遮挡，水面沿用原图；夜光漂入夜呈明亮蓝绿色，完全入夜时每轮咬钩概率相对提高 15%，白天和普通漂无额外加成。基础等待轮次由 2600 ms 改为 2860 ms，最多 22 轮，避免溢出 16 位计时。

背包按鱼竿、线钩、浮漂、鱼饵、钓深、容器，然后图鉴、存档、返回切换；金币图标及余额位于标题旁。容器背景压暗，鱼名、数量、重量、价值显示在清晰底板上。船尾大鱼斜向悬挂，并用绳索连接船尾固定点。

最新价格：玻纤竿 550、碳素竿 1800、鹅毛漂 90、夜光漂 120、大木桶 300、储鱼箱 750 coins；其他商品价格见 `data/economy.json`。旧存档和原解锁保留。详见 [界面与夜钓说明](docs/v17-ui-night-fishing.md)。

## 显示方向与 Embedded Studio

钓鱼工程默认 `FISH_LCD_ROTATION=0`，对应 `ST7789_ROTATION=0`，相对原 `2` 转向 180°，240×240 窗口使用驱动既有的 `Y_SHIFT=80`。公共头文件只增加可覆盖配置，未指定的其他 demo 保持默认 `2`。可在 CMake 用 `-DFISH_LCD_ROTATION=0` 设置；编码器方向独立配置。

常用工程由构建脚本生成：`hpm5301evklite_flash_uf2_debug/segger_embedded_studio/mklink_sample_fishing.emProject`，选择 Debug。另有 `hpm5301evklite_flash_xip_debug/segger_embedded_studio/mklink_sample_fishing.emProject`。构建方式、占用及 UF2 检查见 [验证报告](docs/validation.md)。显示方向尚需实板最终确认。

完整设定、最新价格、11种基础鱼与4种稀有变异及活跃时段统一见 [整体设定](docs/game-design.md)。斑鳜无夜间加成，白天与夜间权重相同；晨昏渐变，其他鱼种按各自表值变化。

营业时间07:00 AM–06:00 PM，外屋正面亮灯/熄灯随时间切换，尝试进入关门商店时主角上方显示“打烊了”1.6秒；打烊无人且无法进入交易。店主四种表情与14类情境各两句台词见 [merchant-dialogue.md](docs/merchant-dialogue.md)。可编辑装备描述稿见 [tackle-descriptions.md](docs/tackle-descriptions.md)，完整当前设定见 [game-design.md](docs/game-design.md)。

当前左岸延伸到x=-284，商店入口在x=-260，地面连续铺到HUD底部。转动一格持续以32px/s慢走，反向转动先停住，再转一格反向走，短按停止并交互；门口、码头或边界自动停。码头停稳后再次向岸转动上岸、向水转动登船，已取消单独上下船按钮。鹅毛漂延长15%提竿窗口，夜光漂夜间咬钩加成15%。用户编辑的六条装备描述、18句店主台词已接入。

最新界面：主界面持续显示金币余额，收鱼卡显示单鱼售价、已解锁图鉴显示纪录价，鱼库只分页显示当前存有鱼的鱼种并追加实际存在的船尾鱼。重绘低干扰左岸地面和有壁灯/挂具的粗像素商店背景，去掉深斜桌面及独立商品摆拍素材。当前共46项Windows主机检查，独立demo13项检查。

商品列表保留名称与选中高亮，选中装备的大图标放在详情左下角（14像素内容整数放大2倍），右侧分名称/售价、描述、实际效果三行。岸上渔夫与船上角色同帽同装，三帧14×20像素含采样边，2倍显示36像素高，包含站立及两种按移动距离交替的步态；停止即站立，左右镜像。

接岸v19：重绘接岸草地/土路，24像素地面重叠与同网格蒙版消除x=0处硬接缝，两岸共用56×47逻辑像素码头资源整数2倍显示。人物同为2像素颗粒，脚底统一高度；店屋由原始母图按64×48逻辑像素重新生成，2倍显示约128×96，门框略高于36像素人物，完整屋顶/地基留在画面内。打烊仍只切换门窗区域。

v20新增四种1%稀有变异（黄化土鲶、长鳍马口、红色长尾鲤、白化鳖），彩色边框、独立收藏/鱼库和同尺寸普通鱼10倍售价，携带时有专属招呼，日常5%给收购线索。云层轮廓重绘并按原山线遮挡；步行反转一格先停、再转反向走，移除方向上船提示。存档v5为292B，独立V5_A/B槽保留旧A/B文件，读取v1–v4，V4装备不会再初始化。[完整设定](docs/rare-variants.md)。

v21：黑鱼、鲤鱼、土鲶在大体型收鱼卡/图鉴中自动使用96×56细节贴图，小尺寸保留48×28原图。11种基础鱼改存原生像素并在绘制时放大，三种细节图加入后位图仍净节省84,672 B；无需新增整屏缓冲。硬件平台继续为MKLink V4，存档与玩法数值保持不变。

v21: Large snakehead, carp and catfish automatically use 96×56 detail tiles in catch cards and guide entries, while small presentations retain the 48×28 originals. Base sprites now store native pixels and scale at draw time; even with three detail tiles, bitmap storage drops by 84,672 bytes without adding a full-screen buffer. The hardware platform remains MKLink V4; saves and gameplay values are unchanged.

v29已实装SD地图缓存，应用与缓存分区独立；旧64 KiB地图页预留已用于路线UI与加载器。当前容量见[验证报告](docs/validation.md)，资源协议见[地图说明](docs/maps-and-sd-resources.md)。

After removing unused forced floating scanf support, the GNU app uses 777,608 / 851,968 bytes, leaving 74,360 bytes (about 72.6 KiB). Floating drawing and diagnostic output remain available. The future SD multi-map bitmap loading proposal is recorded only and is not implemented in this version.


v22：鱼名与数值说明补齐中文，三种线长改为4.5／6.5／8.5米；最大出线后无法继续放线，鱼冲刺造成的弹性超距用弯竿显示，持续两秒后断线，收回范围即解除。冲刺两个方向都有、约70%远离玩家，鲤鱼/马口/斑鳜更积极。左岸连续视差与缓慢云漂移已修正。全部15个收藏项和店主四种表情通过实际规则/画面核实，完整记录见[当前玩法与核实 / Current gameplay audit](docs/v22-gameplay-and-audit.md)。

v22 localizes fish names and numeric UI, sets line lengths to 4.5/6.5/8.5 m, blocks further payout at the limit, and adds visible rod flex with a two-second excess-distance rescue window. Bursts can move both ways, with about 70% away from the player; carp, hooksnout minnow and mandarin fish are more active. Signed shore parallax and slow clouds are fixed. All 15 collection entries and four merchant expressions pass actual rule/render checks. See the current audit above.

v23：全部中文恢复原有12像素字号，撤销v22新增的8像素小字库；商店售价和鱼获数值重新对齐。移除“鱼竿弯了！快收线”提示，弯竿和两秒弹性窗口保持不变。

v23 restores the original 12-pixel Chinese font throughout and removes the v22 8-pixel subset, realigning shop prices and catch metrics. The misleading reel-now message during rod flex is removed; rod animation and the two-second elastic window are unchanged.

游戏名使用通用称呼“溪蟹”“土鲶”，移除真实地区指向；地图为虚构湿地，原物种索引和旧存档兼容。

Player-facing crab/catfish names use generic wording without geographic labels. The map is a fictional wetland; original species indices and save compatibility are preserved.

v24：启用板载蜂鸣器，上钩160 Hz/60 ms，鱼挣扎140 Hz/30 ms、每240 ms一次；成功购买和出售播放660→1100 Hz的上扬收银音。全部使用有限时长PWM和非阻塞更新，退出即停、失败交易静音；未实板试听。参数及调节入口见[蜂鸣器说明](docs/buzzer-feedback.md)。

v24 enables the board buzzer: a 160 Hz/60 ms hook pulse, 140 Hz/30 ms burst pulses every 240 ms, and an ascending 660→1100 Hz cashier cue on successful purchases/sales. Finite PWM notes and nonblocking updates stop on exit and stay silent on failed trades. Hardware audition is pending; see the linked parameters.

v25：行囊加入关闭／仅鱼讯／全部开启三档声音设置，确认后写入293字节V6主存档；旧V1–V5默认全部开启并保留原文件。图鉴分鱼卡／习性两页，保持12像素字号：旋转换鱼、短按翻页、长按600 ms返回。习性资料直接来自当前游戏规则，未知鱼仅给简短线索。本次不新增背景图，构建强制预留至少64 KiB Flash供后续地图页。

v25 adds off/fish-only/all sound modes saved inside authenticated 293-byte v6 files. v1–v5 migration defaults to all sounds and preserves all older files. The 12-pixel guide has portrait/habitat pages: rotate to select fish, tap to change pages, hold 600 ms to return. Details derive from current game rules; locked entries show short hints. No backgrounds are added, and builds reserve at least 64 KiB Flash for the future map page.

## 交易反馈 v26 / Transaction feedback v26

购买成功立即显示暖色 `-550G`，出售鱼获按真实单鱼售价连续显示浅绿色 `+11G`，每250 ms一条，900 ms内上浮淡出，最多四条同时可见。金币与鱼获立即结算，旋转和购买保持可用。稀有鱼与船尾鱼逐条结算反馈，失败交易不冒金币浮字。新鱼的准确售价随1317字节v7存档保存；v1–v6文件只读保留，原鱼获中缺少单鱼明细的部分明确显示“旧鱼获合计”，所有金额保留。没有新增位图；仍为后续地图页预留至少64 KiB Flash。

Successful purchases immediately show a warm `-550G` charge. Sales emit pale-green exact per-fish `+11G` income every 250 ms, rising/fading over 900 ms, with up to four visible at once. Settlement is immediate and inputs remain active. Rare and stern catches are included; failed trades produce no popup. Exact prices persist in the 1317-byte v7 save. All v1–v6 files remain read-only, and aggregate-only legacy cargo is clearly labeled without losing money. No bitmap is added; at least 64 KiB of Flash remains reserved for the future map page.

## 不规则挣扎短震 v27 / Irregular struggle pulses v27

鱼实际挣扎时改为110–170 Hz低频短震，每下20–50 ms、随机静音20–70 ms，起音间距40–120 ms（受游戏线程更新延迟影响），比原固定240 ms更密集且不规律。独立声音随机数不影响鱼种、体型或搏鱼；重复回调不连发、卡顿不补播，平静/体力恢复/离开搏鱼立即安静。上钩提示、收银音和声音三档沿用。实板声音和机壳震感待试听。

Active struggles use 110–170 Hz pulses lasting 20–50 ms, followed by random 20–70 ms silent gaps. Onsets are 40–120 ms apart before game-thread delays, replacing the regular 240 ms cadence. Private sound randomness cannot affect catches, size or fighting. Repeated callbacks cannot double-play, stalls never queue sounds, and lulls/recovery/exits stop playback. Hook/cashier cues and all sound settings remain. Hardware audition is pending.

## 右岸小火车站 v28 / East-bank railway station v28

右码头上岸后向右慢走，可沿连续草地步道抵达小型火车站并自动停下。车站使用与渔具店相同的正立面像素风格：瓦顶、暖砖木梁、候车棚、长椅、行李箱、站钟和前方轨道；人物仍为36像素高。新素材为110×54原生像素、整数2倍显示，仅新增运行时C贴图；原图和生成提示词留在本地。远景向右延伸并保持昼夜/云层的山脊遮挡，码头/地面衔接经过逐像素核对。

v28历史记录：当时只实现站外场景和步道；v29已加入乘车、目的地菜单和地图切换。原湖面、鱼种权重和v7存档格式保留。

Disembark at the east dock and walk right along the continuous meadow path to stop at a small railway station. The frontal pixel-art building matches the shop: tiled roof, aged brick/timber, waiting shelter, bench, suitcase and clock, with foreground rails. Actor height stays at 36 pixels. The 110×54 native tile displays at exactly 2×, and only runtime C art is published. Source art/prompts stay local. A reflected panorama extends the skyline while retaining day/night mountain occlusion; dock/ground joins are checked against actual PFB pixels.

This is the first step toward new-map access: the exterior and path are implemented; train travel, destination selection and map switching remain for later. Lake geometry, fish weights and save v7 are preserved.
