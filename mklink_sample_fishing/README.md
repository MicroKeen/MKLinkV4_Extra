# MKLink V4 钓鱼 / North Wetland Fishing

运行于 **MKLink V4（HPM5301）** 的240×240 RGB565像素钓鱼游戏，使用ST7789屏幕和一颗可双向旋转、按下的编码器。当前版本 **D27 / 2026-11-02**，包括湿地湖泊、城市江河与海滩三张地图。

A 240×240 RGB565 pixel fishing game for **MKLink V4 (HPM5301)** with an ST7789 screen and one rotary encoder. **D27 / 2026-11-02** includes wetland, city-river and coastal regions.

## D27 搏鱼平衡 / Fight balance

按全鱼种、体型、装备与操作策略复核。大鱼不再能用高效鱼竿在首次挣扎前快速硬收；保留强线承重优势，增加可判断的持续发力段、质量阻力和超配压力。修复大鱼疲劳时持续收线仍误判松线及深水整数零进度。不加末段距离或强制时间门槛，存档／资源不变。[设定](docs/fight-d27.md) · [逐鱼种数值与对比](docs/fight-d27-audit.md)。

Whole-roster tuning addresses oversized per-detent gains and excessive strong-rig suppression. It preserves gear roles while adding meaningful exertion windows, mass drag and mismatch pressure. Safe retrieval always progresses; tired large fish no longer suffer unavoidable slack losses while reeling. No artificial landing-distance/time gate. Save/resource layouts remain unchanged.

## D26R2 时间显示 / Time display

背包上方常显当前游戏时间；休息详情显示本次已休息的小时和分钟，松开后保留，下一次按住重新计时。跨午夜和超过一天仍正确累计。实际时间继续存档，临时计数不存档。资源和V21格式不变。[说明](docs/cat-rest-d26-r2.md)。

The backpack shows the current clock. Rest displays elapsed game hours/minutes, retained on release and reset on the next press, including multi-day sessions. Only world time persists in saves. Resources and V21 remain unchanged.

## D26R1 猫与休息 / Cats and rest

都市流浪猫每小时候选概率35%→65%；收养后随玩家到湿地、城市、海滩，次数和好感全局保存。船上背包选中“休息”后长按600ms，以20倍推进时间；松开或旋转离开即停，岸上不提供。资源与V21存档不变。[设定与核验](docs/cat-rest-d26-r1.md)。

City-cat hourly visit candidates rise to65%. Adopted cats follow all three maps and survive travel/saves. Select Rest in the boat backpack and hold600ms for20x world time; release or rotate away to stop. Matching2026.10.31.4 resources and V21 saves remain unchanged.

## 固件与完整资源 / Firmware and complete resources

- 固件：[MicroLink.uf2](MicroLink.uf2)。配套资源 **2026.10.31.4，格式3**。
- 复制完整的 [FishingGame_Resources_DO_NOT_DELETE](sdcard/FishingGame_Resources_DO_NOT_DELETE) 目录到SD根目录，保留`common/`、`maps/`、版本文件及清单。共四个`.fgpk`：公共鱼图与三张地图。
- **从D16或其他旧版升级时，需完整更新资源目录。** 不要只换UF2、单个鱼图包或部分地图包，也不要将ZIP直接当成资源目录。
- 保留原下载器文件及`FishingGame_Saves_DO_NOT_DELETE`，不要格式化。缺少或不匹配资源时，设备先检查原文件系统和容量，再独占开放SD供电脑复制；校验完成后隐藏磁盘并进入游戏。

Download the UF2 and copy the **entire matching2026.10.31.4 resource directory** to the SD root, keeping its `common/` and `maps/` subdirectories and metadata. All four packs are required. Older resources must be replaced together. Preserve downloader files and the save directory; do not format the disk. Missing or mismatched resources use guarded, exclusive SD import, followed by verification and USB detach. See [resource import](docs/resource-import.md).

## 三张地图与玩法 / Regions and gameplay

- **湿地湖泊**：木船、山湖、渔具店；保留原装备、鱼种与昼夜习性。
- **城市江河**：现代天际线、夜景灯光、气垫筏、公园码头、便利店及露天鱼摊；独立鱼种数值与天气，支持100kg级巨物。
- **海滩**：1680px长地图，左侧公路停车位、鱼档、沙滩和棱角礁石，快艇停靠礁石边缘。大陆架之后水深陡增，最右侧延续125m深海；远处单岛灯塔每12秒连续旋转，雨雾中有柔和散射。
- **旅行**：24小时车站与纸质路线图；湿地↔城市180G、城市↔海滩280G、湿地↔海滩460G，行程推进5游戏小时。成功抵达扣费，加载失败不扣。
- **钓鱼**：鱼饵、钓深、游戏时间和天气决定鱼池权重；钓深分为顶层、中层、钓底，实际用线量包括深度。粗线大钩及以上只受实际线长限制；海钓线组缓慢下沉和回收，短线按可达层回退。
- **海钓装备**：1200G海钓线组、6000G重型海竿、2800G诱鱼灯、2500G／200kg海图鱼舱；夜间灯光吸引鱿鱼、带鱼、青花鱼并缩短等待。
- **赶海**：每天04:00沙滩与礁石各随机3～6处；走近出现脚边气泡，附近短按采收，经过不吸附。铲子120G、撬棍180G，各20耐久；非贝螺生物60%触发一次滑块QTE。每处文蛤／牡蛎2～4只、沙蚕3～5条，其他单只；每次采收耗1耐久，失败仍扣。沙滩金色竖琴螺、礁石金色青蟹各80G基价，每处3%替换概率。
- **经济与图鉴**：49项可见图鉴跨地图共用，53张公共鱼图独立缓存；偏低铜级基价20G、银级50G，已有高价保留。符合条件的变异鱼为母种10倍售价，猫同伴将原1%变异概率提高至2.5%。包含容器、鱼舱、船尾挂鱼、逐条出售反馈、声音设置和旧档迁移。
- **天气与同伴**：三地天气独立，雨持续2～48游戏小时，海图暴雨更强；月相、薄雾、分层雨滴、涟漪、雨伞和店面遮雨。都市流浪猫可抚摸、投喂、收养，船上坐／睡；热拿铁和泡面提供限时加成。

Three distinct regions connect through a24-hour railway route. The city adds modern lighting, large freshwater fish, an inflatable raft and separate vendors. The1680px coast adds a speedboat, angular tidal rocks, parking, a modern fish stall, a continental slope to125m and a continuously rotating fog-lit lighthouse. Fares are180G wetland–city,280G city–coast and460G wetland–coast, charged once after successful arrival.

Bait, depth, time and weather affect fish attraction. Depth-aware marine rigs sink and recover progressively; the night fishing light attracts squid, hairtail and chub mackerel. Daily beachcombing offers3–6 spots per area,20-use tools, a one-press escape QTE and batch shellfish/ragworms. Gold harp snails and green mud crabs have80G bases and a3% per-spot chance. The global49-entry collection shares53 packed portraits. Shops, cargo upgrades, rare variants, regional rain, moon phases, an adoptable cat, refreshments and sound settings remain available. Details: [D26 settings](docs/rewards-d26.md), [marine tackle](docs/coast-tools-d22.md), [depth rules](docs/depth-shore-d24.md).

## 操作 / Controls

旋转用于两档移动、浏览和收放线，短按用于交互和确认；钓鱼时按住蓄力、松开抛竿。反向旋转先停止，再转开始反向移动；抵达码头停住，再次向岸／水方向旋转上下船。图鉴短按翻页、长按返回。赶海QTE在滑块进入绿色区时按下。

Rotate to move, browse or reel/ease line; press to interact. Hold/release to cast. Reverse once to stop and again to move the other way. At a dock, rotate toward land/water again to disembark/board. In the guide, short press changes pages and long press returns. Press during the green slider zone in an escape QTE. All controls use the single encoder.

## 存档与核验 / Saves and validation

**V21存档2147B**，独立A/B，保留原密钥、UUID、目录及所有V1～V20文件。保存装备、金币、图鉴、单鱼历史售价、位置、时间、天气、声音、猫状态，以及工具耐久、沙蚕库存和每日赶海点。旧鱼获不因本轮提价重新估价。

131项主机检查通过（主工程93、独立demo38），含真实FatFs内存盘的A/B写入、尾部回读、I/O失败重试、旧档迁移、下载器文件与卷引导扇区保留。53张鱼图解码无截边，三图已加载页面的Arm-2D临时申请为0。Flash余8192B、DLM余1776B、AHB余480B，均已计固定缓冲和预留堆栈；公共／地图缓存合计余101084B。没有新增整屏缓冲或堆分配。实板帧率、掉电及堆栈峰值仍待验。

V21 uses2147-byte A/B saves and preserves original keys, UUID, paths and all V1–V20 files. Equipment, coins, collection, historical catch prices, position, time, weather, sound, cats, tools and daily shore state persist. All131 host checks pass, including actual FatFs on a RAM disk, final-chunk comparison, retry and migration, with downloader files and the volume boot sector preserved. All53 portraits decode without clipped bounds. Application Flash retains8192B, DLM1776B and AHB480B; common/scene cache retains101084B. Hardware FPS, power-loss behavior and stack/heap peaks remain unmeasured.

[核验报告 / validation](docs/fight-d27-build-validation.json) · [内存 / memory](docs/fight-d27-memory.md) · [鱼类审计 / fish audit](docs/fish-audit-d26.md)

## 构建与工程 / Building

需要HPM SDK1.11.0、RISC-V GCC13.2和CMake/Ninja，先按主仓库说明初始化子模块。设置`HPM_SDK_BASE`、`GNURISCV_TOOLCHAIN_PATH`并把工具加入PATH。在本目录执行：

```sh
cmake -S . -B build-fishing -G Ninja -DBOARD=hpm5301evklite -DBOARD_SEARCH_PATH=.. -DHPM_BUILD_TYPE=flash_uf2 -DPYTHON_EXECUTABLE=python -DFISH_PYTHON_EXECUTABLE=python
cmake --build build-fishing
python scripts/verify_build.py --build-dir build-fishing --gnu-only --write-report
python scripts/verify_public_assets.py
```

默认`FISH_LCD_ROTATION=0`修正屏幕方向，编码器方向可用`-DFISH_ENCODER_DIRECTION=-1`反转。Embedded Studio工程生成于`build-fishing/segger_embedded_studio/mklink_sample_fishing.emProject`。应用区`0x80023000–0x80080000`，公共／地图缓存位于`0x80080000–0x800F3000`；下载器和末尾52KiB不在游戏写入范围。

Use the SDK/toolchain above and initialise repository submodules. CMake generates the GNU build and Embedded Studio project. Pre-generated runtime artwork is included; normal builds need no artwork conversion tools. Host tests use native GCC/CMake:

```sh
cmake -S tests -B build-tests -G Ninja
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

可用`-DFISH_CHERRYUSB_DIR=/path/to/hpm_sdk/middleware/cherryusb`指定协议测试头文件。The optional SDK protocol tests use that configurable header path.

## 可编辑文案与素材 / Editable text and artwork

[整体设定](docs/game-design.md) · [鱼类价格](docs/fish-price-bases-d26.md) · [海图数值](docs/coast-species-review.md) · [鱼类评语](docs/catch-comments.md) · [装备描述](docs/tackle-descriptions.md) · [湿地店主](docs/merchant-dialogue.md) · [都市摊主](docs/city-fishmonger-dialogue.md) · [便利店员](docs/city-merchant-dialogue.md) · [海边员工](docs/coast-merchant-dialogue.md)

仓库只提供C代码使用的位图、蒙版、字形、tile描述、运行所需`.fgpk`及必要许可；不提供原画、未采用素材、PNG图集、字体源文件、生成提示词或预览截图。`data/`和Markdown供编辑参考；运行读取已生成C表，改文字需同步生成表和字形。价格总表是当前汇总，海图数值导入工具为`tools/prepare_marine_prices_d22.py`。

Only used runtime C artwork, required packs and licenses are published. Source masters, unused alternatives, image exports, font sources, generation prompts and previews are excluded. Editable data/text accompanies generated C tables; text changes require matching tables and glyphs. The full price table is a snapshot; the marine price importer reads the coastal species review. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). Earlier revision records remain under`docs/`.
