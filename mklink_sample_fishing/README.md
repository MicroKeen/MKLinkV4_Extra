# mklink_sample_fishing / 北方湿地钓鱼

基于同仓库 `mklink_sample_fire` 标准工程的独立钓鱼 demo。目标是 MKLink V4 / HPM5301、240×240 RGB565 ST7789 和一颗双向旋转、可按下的编码器。


包含横向小船/岸边移动、蓄力抛竿、漂相提竿、张力与体力搏鱼、11 种鱼图鉴、鱼竿/线钩/浮漂/鱼饵/钓深选择，以及 小木桶/可购买扩容容器/船尾单条巨鱼、左岸渔具店/coins、装备解锁/特殊饵料消耗、系统毫秒驱动的昼夜和 SD 手动/五分钟自动存档。

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

- 旋转控制横向移动；继续旋转可改变速度和方向。
- 短按打开行囊，旋转选择装备/图鉴/存档/返回；短按选择或确认。
- 长按蓄力，松开抛竿；出现可提竿漂相时按下。
- 搏鱼时顺转收线、逆转放线，观察张力与体力。

画面使用编码器用语，存档按钮没有额外操作教学。若实物编码器方向相反，可在 CMake 加 `-DFISH_ENCODER_DIRECTION=-1`（默认 `1`）。

## SD 存档与兼容性

SD 卡文件目录为：

```text
FishingGame_Saves_DO_NOT_DELETE/
  NorthWetland_Save_A_DO_NOT_DELETE.sav
  NorthWetland_Save_B_DO_NOT_DELETE.sav
  README_DO_NOT_DELETE.txt
```

保存图鉴捕获次数/最大尺寸、最近捕获、总捕获数、当前装备/钓深，以及金币、解锁、消耗品库存、鱼库、船尾和游戏分钟。每份 v4 存档 236 字节（兼容读取 92 字节 v1 和 236 字节 v2/v3），AES-SIV 认证加密，绑定芯片 UUID；写非活动槽并同步、关闭、读回校验后才确认成功。右上显示“存档中/已存档/失败/恢复/异常”。开机选择最新的有效槽，异常旧文件不会被自动覆盖。

**按作者要求，demo 保留并公开原项目密钥和原目录，读取并迁移同一板卡旧游戏的 v1 存档。** 当前写 v4，原 60 字节核心字段和索引不重排。首次升级保留图鉴、金币、饵料库存、鱼获、容器和时间，竿/线组/漂及所有权恢复初始配置，当前饵料恢复普通假饵；写入一次标记后，后续购买正常保留。旧固件无法读 v4，切换旧版本前备份目录，切回本版本可恢复。 更新时不要重新生成 `src/fish/fish_save_key.c` 中的密钥、重排鱼种/装备索引或改写格式版本。切换其他固件前先手动存档并等待“已存档”；保留 SD 上整个目录，可在切回兼容固件后读取。

密钥已公开，了解源码的玩家可以生成合法认证的修改存档。本 demo 的加密主要避免明文编辑、识别损坏文件及限制跨板使用，不提供强防作弊或防旧存档回退。SD 卡被格式化、删除或物理损坏无法靠游戏避免；建议备份整个目录。默认不启用 USB MSC/CDC，让存档线程独占 SD 文件系统，避免电脑和游戏同时修改 FAT 元数据。

SD 使用 SPI1，LCD 使用 SPI0。**PA26 是 LCD 背光，不能当软件 SD 片选切换。** 通用 board.h 将 `BOARD_SPI_CS_PIN` 定义为 PA26，但 MKLink 原 SD 适配器的片选回调为空；此 demo 沿用该行为，不配置或切换 PA26，修复了存档时背光频闪。

## 验证与维护

游戏逻辑测试可在桌面 C11 编译器上执行，无需 HPM SDK 或 SDL：

```sh
cmake -S mklink_sample_fishing/tests -B mklink_sample_fishing/build-tests
cmake --build mklink_sample_fishing/build-tests
ctest --test-dir mklink_sample_fishing/build-tests --output-on-failure
```

10 项测试覆盖玩法、搏鱼平衡、地图装备、图鉴、尺寸采样、存档认证/v1/v2/v3迁移/v4一次装备初始化与后续购买恢复/容器/鱼获/库存恢复/自动周期/计时回绕、经济系统/时钟、实际输入适配及 SD 读写时的背光电平隔离。测试中的 HAL/GUI 桩不代表实板运行。链接占用、GNU/SEGGER 构建及 UF2 校验记录见 [验证报告](docs/validation.md)；本次移植未执行实板烧录或断电测试。

`src/fish/` 包含游戏逻辑与已生成的 RGB565/A8/A1 素材，常规构建不需要素材转换工具。`data/` 和 `docs/` 保存数值/说明；运行使用已生成的 `fish_*_data.c`、`fish_rules.h` 和 `fish_map_config.h`，修改 JSON 后需同步这些 C 数据。新增文字需要重新导出中文字体子集。

第三方代码与字形的许可说明见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。

## 鱼库、商店与昼夜

初始装备为竹竿、通用线钩、红白漂和无限普通假饵（“塑料味”）。普通鱼获收入初始“小木桶”（10 kg），可购买大木桶（20 kg / 300 coins）和储鱼箱（35 kg / 750 coins）。单条超过当前容量才挂船尾，限一条；装不下时放流，仍记图鉴。行囊显示当前容器名称和实物，打开后以容器背景查看鱼获。升级保留全部鱼，并将能装下的船尾鱼收入容器。

在西码头下船后向左沿小径走，靠近木屋短按进入商店。旋转浏览五行商品列表，底部详情显示选中商品的名称、价格和容量；短按交易，出售全部鱼获换取 coins，购买渔具永久解锁，特殊饵料每包五份/每次抛竿消耗一份；钱不足、重复装备、库存满均不扣钱。“返回小径”退出，再回码头登船。具体价格和规则见 [管理文档](docs/economy-time-and-shop.md)。

游戏时间由系统单调毫秒驱动，每秒折算一分钟，一昼夜 24 分钟；日夜渐变、太阳/月亮移动和 12 条水面反光不增加整屏缓冲。读取存档恢复游戏分钟，退出期间不快进，尚未实测硬件帧率。

公开仓库只提供 `src/fish/assets/` 中实际链接的 C 位图/蒙版/字形和必需的 OFL 许可。生成原图、未采用版本、PNG/图集、字体源文件、提示词和截图不随仓库提供。正常构建无需素材转换工具，执行 `python scripts/verify_public_assets.py` 可校验发布范围。

## 柜台、夜景和操作 v17

店内采用柜台近景和放大的老人上半身，旋转浏览商品，左侧为五行滚动列表，底部显示价格和详细信息；仍为 16 个稳定商品索引。位置见 `data/shop_layout.json`，`python tools/prepare_shop_layout.py` 可导出 C 坐标。夜景只替换原背景天空，月亮/星星按原山林轮廓遮挡，水面沿用原图；夜光漂入夜呈明亮蓝绿色，完全入夜时每轮咬钩概率相对提高 15%，白天和普通漂无额外加成。基础等待轮次由 2600 ms 改为 2860 ms，最多 22 轮，避免溢出 16 位计时。

背包按鱼竿、线钩、浮漂、鱼饵、钓深、容器，然后图鉴、存档、返回切换；金币图标及余额位于标题旁。容器背景压暗，鱼名、数量、重量、价值显示在清晰底板上。船尾大鱼斜向悬挂，并用绳索连接船尾固定点。

最新价格：玻纤竿 550、碳素竿 1800、鹅毛漂 90、夜光漂 120、大木桶 300、储鱼箱 750 coins；其他商品价格见 `data/economy.json`。旧存档和原解锁保留。详见 [界面与夜钓说明](docs/v17-ui-night-fishing.md)。

## 显示方向与 Embedded Studio

钓鱼工程默认 `FISH_LCD_ROTATION=0`，对应 `ST7789_ROTATION=0`，相对原 `2` 转向 180°，240×240 窗口使用驱动既有的 `Y_SHIFT=80`。公共头文件只增加可覆盖配置，未指定的其他 demo 保持默认 `2`。可在 CMake 用 `-DFISH_LCD_ROTATION=0` 设置；编码器方向独立配置。

常用工程由构建脚本生成：`hpm5301evklite_flash_uf2_debug/segger_embedded_studio/mklink_sample_fishing.emProject`，选择 Debug。另有 `hpm5301evklite_flash_xip_debug/segger_embedded_studio/mklink_sample_fishing.emProject`。构建方式、占用及 UF2 检查见 [验证报告](docs/validation.md)。显示方向尚需实板最终确认。

完整设定、最新价格与11种鱼的活跃时段统一见 [整体设定](docs/game-design.md)。斑鳜无夜间加成，白天与夜间权重相同；晨昏渐变，其他鱼种按各自表值变化。

营业时间07:00 AM–06:00 PM，外屋正面亮灯/熄灯随时间切换，尝试进入关门商店时主角上方显示“打烊了”1.6秒；打烊无人且无法进入交易。店主四种表情与9类情境各两句台词见 [merchant-dialogue.md](docs/merchant-dialogue.md)。可编辑装备描述稿见 [tackle-descriptions.md](docs/tackle-descriptions.md)，完整当前设定见 [game-design.md](docs/game-design.md)。

当前左岸延伸到x=-284，商店入口在x=-260，地面连续铺到HUD底部。转动一格持续以32px/s慢走，反向转动换向，短按停止并交互；门口、码头或边界自动停。码头停稳后再次向岸转动上岸、向水转动登船，已取消单独上下船按钮。鹅毛漂延长15%提竿窗口，夜光漂夜间咬钩加成15%。用户编辑的六条装备描述、18句店主台词已接入。

最新界面：主界面持续显示金币余额，收鱼卡显示单鱼SELL售价、已解锁图鉴显示BEST纪录价，鱼库只分页显示当前存有鱼的鱼种并追加实际存在的船尾鱼。重绘低干扰左岸地面和有壁灯/挂具的粗像素商店背景，去掉深斜桌面及独立商品摆拍素材。当前共35项Windows模拟器检查，独立demo10项检查。

商品列表保留名称与选中高亮，选中装备的大图标放在详情左下角（14像素内容整数放大2倍），右侧分名称/售价、描述、实际效果三行。岸上渔夫与船上角色同帽同装，三帧14×24像素包含站立及两种按移动距离交替的步态；停止即站立，左右镜像。
