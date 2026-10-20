# MKLink V4 钓鱼 / North Wetland Fishing

运行于 **MKLink V4（HPM5301）** 的240×240 RGB565像素钓鱼游戏，使用ST7789屏幕和一颗可双向旋转、按下的编码器。当前版本 **D12 / 2026-10-20**，作者实测反馈“基本稳定”。

A240×240 RGB565 pixel fishing game for **MKLink V4 (HPM5301)**, using an ST7789 display and one rotary encoder with a push button. **D12 / 2026-10-20** is published following the author's report of basic stability.

## 下载与资源 / Firmware and resources

- 固件：[MicroLink.uf2](MicroLink.uf2)。资源版本：**2026.10.19.5**（格式2）。
- 首次使用，复制完整的 [FishingGame_Resources_DO_NOT_DELETE](sdcard/FishingGame_Resources_DO_NOT_DELETE) 目录到SD根目录，保留其中`maps/`等子目录。不要只复制两个`.fgpk`文件或ZIP本身。
- 已有2026.10.19.5配套资源时，只更新UF2，无需重拷。旧版本资源需要更新完整目录。
- 缺少或不匹配资源时，设备检查原文件系统及容量后才开放SD供电脑复制；校验完成后自动隐藏磁盘并进入游戏。正常游戏期间不开放USB磁盘。保留原下载器文件及存档目录；不要格式化磁盘。

Download the UF2 above and copy the **entire matching resource directory** to the SD root, preserving its subdirectories. Existing2026.10.19.5 packs need no recopy. Missing/mismatched packs trigger guarded, exclusive SD import; successful validation detaches USB and starts the game. Keep downloader files and saves; do not format the disk. See [resource import](docs/resource-import.md).

## 游戏内容 / Gameplay

湿地湖泊与城市江河两处钓点，通过24小时车站和纸质路线图往返，每次180G并推进5游戏小时。都市有独立鱼种、深河道、现代天际线和夜间灯光，使用气垫筏。

鱼种依水域、游戏时间、鱼饵和天气分配出现权重；24项活动图鉴包含四种条件1%稀有变异，特殊鱼获为彩色外框并获得10倍售价。鱼竿、线组、浮漂、饵料、容器、金币、买卖、图鉴及声音设置均已实现。

两地独立天气，降雨持续2～48游戏小时，包含远景薄雾、分层雨线和水面涟漪。淋雨1游戏小时潮湿满，最多降低20%收放线速度；热拿铁持续6游戏小时，收放线和提竿窗口提高10%。都市便利店09:00–21:00，露天鱼摊06:00–16:00；湿地渔具店07:00–18:00。

大鱼压力通过张力、可见冲刺和线长变化体现，没有最后一段距离锁。真正断线会损坏当前线组；金币不足120G且没有细线小钩时，可免费补一副。都市鱼摊提供600G八编PE线组，降低小鱼上鱼率并延长等待。

Two regions connect through a24-hour station: each trip costs180G and five game hours. The city adds independent fish profiles, a deeper river, night lighting and an inflatable raft. Equipment, bait attraction, fish sales, cargo upgrades, a24-entry active collection, four conditional1% variants and sound settings are implemented. Regional rain lasts2–48game hours. Wetness and hot-latte buffs affect handling. Giant fights use visible surges and tension without an artificial final-distance gate. Broken rigs require replacement; qualifying players can obtain a free fine rig.

## 操作 / Controls

旋转用于移动、浏览或收放线，短按用于交互和确认；钓鱼时按住蓄力、松开抛竿。移动到码头后先停，再次向岸/水方向旋转即可上下船。反向旋转先停止，再次旋转反向移动。图鉴短按翻页、长按返回。只有一颗编码器，没有鼠标或额外方向键。

Rotate to move, browse or handle line; press to interact/confirm, hold and release to cast. Stop at a dock and rotate again toward shore/water to disembark/board. Reverse once to stop, then again to reverse direction. In the guide, tap to change page and hold to return.

## 存档与核验 / Saves and validation

V13存档仍为1503B，沿用原密钥、设备UUID和目录，使用独立V13_A/B，读取并保留V1–V12旧档。保存位置、朝向、地图、时间、天气、潮湿、拿铁、声音、装备、金币、图鉴及鱼获；启动加载资源后恢复位置，离线不推进时间。存档目录`FishingGame_Saves_DO_NOT_DELETE`建议整体备份。

针对S08/01加入SD写入CRC16、延迟响应与有界忙等待，保留A/B、同步和回读验证，并补充S/W错误编号。72项主工程与27项独立demo检查通过；程序区余8,628B，DLM静态余3,824B，资源缓存余19,060B。作者报告实机基本稳定；掉电、拔卡、硬件堆峰值和栈水位未专项验收，主机测试不代替这些项目。

V13 remains1503bytes, preserves original keys/UUID/path and all old files, and restores location, clock, weather and progress. SD writes now handle CRC16, delayed responses and bounded busy waits while retaining A/B and durable readback checks. All99 workspace checks pass, including27 standalone demo checks. The author reports basic on-device stability; power-loss/card-removal resilience and hardware heap/stack peaks remain untested. See [validation](docs/validation.md), [memory](docs/rain-d12-memory.md) and [D12 release notes](docs/release-d12.md).

## 构建与工程 / Building

需要HPM SDK1.11.0、RISC-V GCC13.2和CMake/Ninja，先按主仓库说明初始化子模块。设置`HPM_SDK_BASE`、`GNURISCV_TOOLCHAIN_PATH`并把工具加入PATH。

```sh
cmake -S . -B build-fishing -G Ninja -DBOARD=hpm5301evklite -DBOARD_SEARCH_PATH=.. -DHPM_BUILD_TYPE=flash_uf2 -DPYTHON_EXECUTABLE=python -DFISH_PYTHON_EXECUTABLE=python
cmake --build build-fishing
python scripts/verify_build.py --build-dir build-fishing --gnu-only --write-report
python scripts/verify_public_assets.py
```

默认`FISH_LCD_ROTATION=0`修正屏幕方向，编码器方向可用`-DFISH_ENCODER_DIRECTION=-1`反转。生成的Embedded Studio工程位于`build-fishing/segger_embedded_studio/mklink_sample_fishing.emProject`。应用区`0x80023000–0x80080000`，资源缓存`0x80080000–0x800F3000`；下载器和末尾52KiB不在游戏写入范围。

Use the SDK/toolchain above and initialise repository submodules. CMake generates the GNU build and Embedded Studio project. Pre-generated runtime artwork is included; normal builds need no artwork conversion tools. Host tests use a native GCC/CMake toolchain:

```sh
cmake -S tests -B build-tests -G Ninja
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

安装SDK在其他位置时，可通过`-DFISH_CHERRYUSB_DIR=/path/to/hpm_sdk/middleware/cherryusb`指定协议测试头文件路径。The optional SDK protocol tests use that configurable header path.

## 设定、台词与素材 / Design, dialogue and assets

[整体设定](docs/game-design.md) · [D12调整](docs/rain-depth-and-lines-d12.md) · [鱼类评语](docs/catch-comments.md) · [装备描述](docs/tackle-descriptions.md) · [湿地店主](docs/merchant-dialogue.md) · [都市摊主](docs/city-fishmonger-dialogue.md) · [便利店员](docs/city-merchant-dialogue.md)

仓库只提供C代码用到的共享位图/蒙版/字形、地图tile描述、实际运行`.fgpk`包及必要许可；不提供原画、未采用素材、PNG图集、字体源文件、生成提示词或预览截图。`data/`和Markdown用于编辑设定；运行读取已生成C表，改文字需同步生成表和字形。第三方许可见[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

Only runtime C assets, required map packs and licenses are published. Source artwork, unused alternatives, image exports, font masters, prompts and previews stay out of the repository. Editable design/dialogue files accompany the generated runtime tables; new glyphs require regeneration.
