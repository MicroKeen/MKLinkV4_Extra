# D12 发布说明 / Release notes

2026-10-20，硬件平台MKLink V4。作者反馈当前版本“基本稳定”，恢复上传，更新现有PR。固件与已交付D12一致，未修改游戏行为。

The author reported basic stability and approved publication on2026-10-20. This update targets MKLink V4 and publishes the same tested D12 firmware without gameplay changes.

- 两地图、纸质车站路线图、每程180G/5游戏时、都市鱼种/便利店/露天鱼摊、生成夜景灯光。
- 独立2～48小时降雨、景深雨线、水面涟漪、都市晨夜雾、潮湿及热拿铁状态。
- 删除最后收线距离锁，青鱼与鱤鱼产生更明显冲刺位移；断线损坏、线组售价与600G八编PE、免费细线补给。
- V13存档保持1503B，兼容旧档，保存位置/朝向/时间/天气；SD CRC、延迟响应、有界忙等待和A/B回读恢复。

Two regions, station travel, city fish/vendors, generated night lighting, independent regional rain, wetness/latte buffs, visible giant surges, rig damage/replacement and V13 position/weather saves are included. The SD writer adds CRC16, delayed-response polling and bounded busy waits while preserving A/B readback protection.

## 安装与核验 / Installation and validation

配套资源2026.10.19.5。已有该版本只更新`MicroLink.uf2`，否则复制完整`sdcard/FishingGame_Resources_DO_NOT_DELETE`目录到SD根目录。保留下载器和存档文件，不格式化或重分区。

Use matching2026.10.19.5 resources. Existing matching packs need no recopy; otherwise copy the entire resource directory. Preserve downloader files and saves; do not format or repartition.

99项工作区检查（主工程72、独立demo27）和60帧PFB像素检查通过。程序区372,300/380,928B，余8,628B；DLM126,480/130,304B，静态余3,824B；资源缓存447,884/466,944B，余19,060B。未新增整屏缓冲。作者反馈基本稳定与自动测试结果分别记录；代理未操作实物SD或烧录设备，未专项执行掉电/拔卡、帧率与硬件堆栈峰值测试。

All99 workspace checks and60 pixel-checked frames pass. Static partition accounting and zero Arm-2D scratch allocation traces are documented. The author's basic-stability report is separate from automated evidence. The agent did not flash or access physical media; power-loss/card-removal resilience, FPS and hardware heap/stack peaks remain unmeasured.

UF2 SHA-256: `115069750f1d80237de0c673413c40c962e94fb6f5d06c1b90f390ffc6f5d64a`.

只发布运行需要的C素材、地图资源包、许可和文档，不附原图、字体源、生成稿或预览。Only runtime C artwork, map packs, licenses and documentation are included; source artwork and previews are excluded.
