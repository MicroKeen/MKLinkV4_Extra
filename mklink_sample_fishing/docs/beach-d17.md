# D17 海滩场景 / Beach scene

2026-10-29，MKLink V4，240×240 RGB565。本轮先实现地图、快艇与天气，海鱼和海滩商店尚未加入。当前为本地开发版，未上传仓库，未烧录设备或操作实体SD。

## 场景与位置

车站纸质路线图的海滩锚点在城市江河右下方，虚拟坐标(484,284)，以折线路轨连接；旋转依次切换三个钓点，选中后平滑移动纸张。D17-R1在纸张右下补上海岸轮廓、窄沙岸、低饱和海洋底色、水波符号与一座小灯塔岛，保持网格/折痕可见，随纸张一起移动。保持每次180G、5游戏小时，加载成功后才扣费/推进时间；失败不扣费。

海滩世界长度1440像素，前两图仍960。左侧0～480为干净浅色沙滩，少量礁石点缀；480～1440为开放海面。只有左侧码头，上船位置540，右侧最远航行至1368，不在海上虚构另一岸或码头。车站位于沙滩240，步行范围32～480。抵达码头先停，再向对应方向旋转上下船，反向先停与两档行走规则保留。

水深由沙滩边缘0米逐渐增加：540为1.6米、720为5.5米、960为12米、1200为21米、1400为28.5米，最外缘30米。位置条和水深剖面按本地图长度缩放；不会继续套用原960像素边界。

蓝天白云、单座离岸岛屿与更突出的灯塔，远景以较浅的1/8镜头位移保持距离感。夜晚与阴雨时灯塔有12秒周期的柔和扫光，位于岸边和主角后方，不覆盖HUD。摩托快艇为独立生成的白色/蓝绿色船体，带舷外机，继续使用原主角像素和统一2倍网格；猫、雨伞、鱼竿和水面接触沿用既有系统。

天气使用独立的海滩种子，不共享湿地/城市雨期。原2～48游戏小时雨期、过渡、潮湿和雨伞规则保留；海滩有更强阴天压暗、60条远雨以及最多42条中近景长雨，近雨速度和长度更明显，不新增整屏图层缓冲。

## 资源与存档

三图配套资源版本为 **2026.10.29.1**，包含0_wetland、1_city_river、2_beach三个`.fgpk`。必须使用此固件对应的完整目录，不能只补第三个文件。解压后将`FishingGame_Resources_DO_NOT_DELETE`整个目录复制到SD根目录；原下载器文件和`FishingGame_Saves_DO_NOT_DELETE`保留，不格式化。

仍只使用原0x80080000～0x800F3000缓存，每次只缓存当前地图；每个资源载荷449,916B，不因增加第三图而占用三份SRAM或扩大Flash写入区。固定加载缓冲4096B，场景共用960B行缓冲。三张路线缩略图均保留64×32尺寸，以4bpp共3168B常驻ROM，在缓存无效时也能显示。

V17存档仍1511B，独立A/B文件，保留原密钥、UUID、目录及V1～V16旧档；保存海滩地图编号、沙滩/远海位置、时间、天气、潮湿和同伴状态。鱼获来源编号2仍保留历史都市白鲢含义，不挪作新的海鱼来源；本轮不在海滩刷出淡水鱼，后续海鱼需单独追加来源和鱼种。

素材由内置image_gen生成，原图与提示记录位于本地`assets/source/beach/`；生成式原图不随公共demo导出。运行素材进入C资源和配套SD包。当前核验与内存报告见`beach-d17-build-validation.json`、`beach-d17-memory.md`。

## English

The beach sits southeast of the city on the station's scrolling paper route. Travel retains the180G/five-hour rule and charges only after successful loading. This1440-pixel world is50% longer than the original two: clean sand fills the left third, while the water deepens continuously eastward to30m. A single western dock and beach station provide boarding and return travel; the eastern boundary remains open sea.

A motor speedboat uses the original fisherman pixels. The generated sunny panorama has one offshore island and a prominent lighthouse with a soft12-second rotating beam in darkness/rain. Independent regional weather adds stronger overcast dimming and longer, denser storm rain. Existing wetness, umbrella and companion mechanics remain.

Use the complete2026.10.29.1 resource folder, preserving downloader files and saves. The original460KiB Flash cache,4KiB loader buffer and960B shared row buffer remain. Three64×32 indexed route previews take3168B ROM. V17 remains1511bytes with independent A/B files and all older files retained. Marine fish and a beach shop are deferred by the owner's choice. Local build; no device flashing, physical SD writes or repository push.
