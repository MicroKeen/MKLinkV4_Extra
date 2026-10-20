# D7 鳙鱼与宽河道 / Bighead carp and deep urban river

2026-10-19，硬件平台MKLink V4。本版只在本地构建，不推Git；未操作或烧录用户设备。

## 当前鱼种 / Current fish

| 鱼种 | 稀有度 | 钓层 | 长度上限 | 重量上限 |
|---|---|---|---:|---:|
| 白鲢 | 铜 | 表层 | 100 cm | 40 kg |
| 鳙鱼 | 白银 | 中下层 | 145 cm | 60 kg |

按作者最后确认值执行。白鲢只参与表层权重，深处不再出鱼；鳙鱼参与中层、下层，水面和贴底均不参与。鳙鱼使用独立饵料、时段、区域及挣扎配置，不继承红尾的连续跃水机制。原12种城市鱼中移除红尾，补入鳙鱼，城市仍为12种可钓鱼，当前图鉴24项。

Silver carp is bronze, surface-only, capped at 100 cm / 40 kg. Bighead carp is silver, middle/lower-water, capped at 145 cm / 60 kg. Catchable redtail is replaced; the city retains twelve fish and the active guide retains twenty-four entries. The wetland's existing tuning is preserved.

鳙鱼使用内置ImageGen新建立绘，突出大头比例、低位小眼与深色不规则斑块；白鲢保持银白色，二者有独立素材。形态参考 [FAO鳙鱼资料](https://www.fao.org/fishery/docs/CDrom/aquaculture/I1129m/file/en/en_bigheadcarp.htm)，体型上限为作者指定的游戏数值。鳙鱼没有沿用红尾贴图。


## 宽河道 / River depth

城市使用独立地形：水边实体岸坎外约3米，西侧采样点x96为3.6米，东侧x864为4.2米，江心x480为16米，避免湿地式浅岸。水深指示图、人物所在水深、抛竿落点、钓底与鱼种权重统一读取城市剖面。

城市钓深选项为表面、0.5、2、4、6.5、9、12米和钓底；钓底按实际落点水深计算。图鉴仍只显示定性钓层，不加入数值钓深描述。湿地保留原剖面、选项与6米最大深度。游戏距离和水深是虚拟地形参数。

The city uses steep banks and a broad deep channel, with 3.6/4.2 m near-bank samples and a 16 m centre. HUD, cast depth, bottom fishing and species layers share that profile. Wetland terrain/options remain unchanged, including its 6 m maximum. The guide retains qualitative layers.

## 文本与存档 / Text and saves

作者本次修改的评语按鱼名匹配回稳定ID，保留原句，不因调整行序串鱼。编辑入口仍为 [catch-comments.md](catch-comments.md)，每句最多13字符，中文保持12像素。

鳙鱼追加内部ID24，原红尾ID21保留用于旧档历史、鱼库和出售，正常图鉴跳过它且不会再遇到。活动图鉴顺序为白鲢→鳙鱼→赤眼鳟→鳡；评语表的编号是稳定标识，可不连续。

存档升级为V10，共1485 B，独立A/B，读取V1–V9且不覆盖旧文件；原密钥、设备UUID、装备初始化世代与存档目录保留。V9原1438 B载荷前缀不移动，追加鳙鱼14 B记录和1 B纪录来源。

旧版白鲢可达130cm，旧纪录与已有鱼获保留D6计算曲线，来源标记2只表示旧参数版本，不是第三张地图；保留原重量与售价。新白鲢使用100cm/40kg。旧红尾不会被当作新鳙鱼，也不会凭空解锁鳙鱼图鉴。

V10 appends bighead data without shifting V9 fields, writes separate A/B slots and reads V1–V9. D6 silver carp retains its historical size/weight/price profile. Retired redtail records and cargo remain sellable and never unlock bighead. All older save files and acquired gear remain intact.

## 资源与验证 / Resources and checks

本版必须配套 **2026.10.19.2** 资源，两张地图都更新。解压后把完整 `FishingGame_Resources_DO_NOT_DELETE` 目录放在原SD根目录，保留下载器文件与全部存档。导入流程沿用已修复的条件式USB；不格式化、不改分区。

鳙鱼使用96×56像素；兼容专用红尾缩为48×28，仍保留轮廓。载荷454,008 B，缓存剩12,936 B。加载缓冲4,096 B，昼夜/店内复用960 B行缓冲，不把整包搬入SRAM。内存细节见 [D7内存报告](city-d7-memory.md)。

Require matching 2026.10.19.2 packs for both maps. Preserve all existing downloader files and saves; do not format or repartition. Full fish art is 96×56; compatibility-only redtail uses 48×28, retaining 12,936 B cache headroom. Physical-device heap/stack peaks remain unmeasured.

验证包含城市12鱼种真实抛竿可遇、白鲢/鳙鱼层权重边界、深度剖面和HUD、24项图鉴顺序、V8/V9迁移、130cm旧白鲢的40kg重量/售价保留、旧红尾不解锁鳙鱼、60kg鳙鱼存档往返与91张PFB画面。最新构建证据见 `city-revision-d7-build-validation.json`。
