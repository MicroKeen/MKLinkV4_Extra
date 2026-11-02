# MKLink V4 D27 构建核验 / Build validation

D27：全鱼种搏鱼平衡复核，修正大鱼收线过快、强线双重减压，以及深水末段零进度／疲劳误判松线。详见[fight-d27.md](fight-d27.md)。D26R2：背包常显游戏时间，休息显示本次已过去的小时／分钟，松开保留结果。详见[时间显示](cat-rest-d26-r2.md)。D26R1：猫每小时出现概率65%，三图跟随；船上选中休息，长按600ms后20倍推进时间，松开恢复。存档和资源不变。详见[猫与休息](cat-rest-d26-r1.md)。本版：海图1680px、右端125m，连续旋转灯塔，批量赶海与两种金色生物，偏低铜／银基价20G／50G。V21存档2147B迁移旧档，完整资源2026.10.31.4。详见[本版设定](rewards-d26.md)、[回归核验](rewards-d26-build-validation.json)、[内存报告](rewards-d26-memory.md)。本地未上传。 / Extended deep sea, continuous lighthouse rotation, batch shore rewards, gold species and price increases. V21 migrates older saves; matching2026.10.31.4 resources. Local only.

以下为历史版本摘要；以本版设定及本文件数值表为准。 / Earlier revision notes follow; current settings and measured region sizes take precedence.

D24：三图按线组和真实线长钓底；细线<5m、通用<10m，强韧及以上只受线长限制。强韧20m、八编25m，无法钓底依次转中层／表层。合入作者MD价格；沙滩与棱角礁石32px渐接。完整资源2026.10.31.1，V19旧档保留。本地未上传。 / Shared depth-aware rig resolution, longer freshwater tackle, edited prices and a seamless sand/reef join; compatible V19 saves and matching resources.

历史D19：海滩现代鱼档、公路堤岸、礁石衔接，以及缓坡大陆架到右四分之一陡降的深水曲线；船只最右水深114米。配套完整2026.10.29.3格式3资源，V17存档不变；海鱼待加入，本地未推送。 / D19 adds a modern coastal vendor, seawall road, reef transition and a continental slope reaching114m at the boat limit. Matching resources required; marine species deferred.

历史D18：28张鱼图无损压缩75.2%，独立公共鱼图包/缓存，换地图只擦写场景分区。原海滩D17：新增右下海滩锚点、1440像素长地图、快艇、单岛灯塔及强暴雨；V17独立存档，配套三图资源2026.10.29.1。海鱼尚未加入，本版未上传。 D16历史：轻量猫选项框、岸边往返与屏内状态锁、八阶段月相及店面遮雨；继承D15：流浪猫好感/每日喂食/同伴与2.5%变异加成；修复升级容器误放流，限制竹竿配线，三档钓层及600G永久拟真假饵。V16存档1511B，保留所有V1–V15旧档。 / D16 adds a small cat menu, stable visible visits, eight moon phases and roof shelter; V16 saves retain all older files.

继承更新：商店列表反转滚轮方向；岸上增加32/64像素每秒两档步行，反向先停、交互点自动停车。旧存档与SD资源包保持兼容。 / Local revision: reversed shop-wheel navigation and32/64px/s shore walking, retaining reverse-to-stop and interaction-point stops. Existing saves and packs remain compatible.

断线后仅10%概率损坏线组，其余保留；河豚中钩后逃脱额外独立20%概率咬断，最多消耗一副；泡面4游戏小时只提升收放线速度10%，不增加提竿窗口。热拿铁保持6游戏小时、收放线和提竿窗口各+10%。采用情境化提示，旧存档和配套SD资源保持兼容。以下为继承的D13功能。
Each line break makes one10% rig-damage roll. Hooked puffer escapes add an independent20% bite-through chance, consuming at most one rig. Noodles grant10% handling only for four game hours; latte keeps both10% bonuses for six hours. Existing saves and packs remain compatible. Inherited D13 features follow.

D13新增都市3500G高碳长竿（7.2米、125%收线、大鱼张力缓冲）、160G永久雨伞及28G泡面（4游戏小时、收放线速度+10%）。雨伞岸上手持、船上固定，阻止新潮湿。泡面和35G拿铁共用状态，后买替换不叠加。V14仍1503B并独立保留V13等旧档；配套资源2026.10.19.5不变。D13新增内容待实机验证，D12稳定反馈不等同新功能已验收。
D13 adds a3500G city long rod (7.2m,125% retrieval, reduced detent tension above6.5kg), a160G permanent umbrella and28G noodles (four game hours,+10% handling only). The umbrella is held on shore and mounted on the boat. Meal/drink buffs replace rather than stack. V14 remains1503bytes and retains V13 files. The same SD packs work. New D13 features await on-device testing.

GNU RISC-V GCC 13.2、HPM SDK 1.11.0。2026-11-02。本次仅重建 GNU flash_uf2，SEGGER 工程输入已检查，未重新编译 SEGGER。

旧的容量错误测试版保持撤回。当前版本恢复原下载器的 SD 扇区映射，开放前检查原文件系统、容量和 MBR/BPB 边界；CPU 阻塞 SPI 路径不做 DMA 地址转换或接收后缓存失效。
The original faulty-capacity build remains withdrawn. The current local importer restores stock SD LBA mapping with existing-volume/geometry checks, exclusive ownership and CPU-blocking SPI pointers. Model checks do not establish integrity of user media or hardware acceptance. No user-device writes or flashing were performed. Test results are recorded separately.

Resource version: 2026.10.31.4; format 3. Valid resources keep USB hidden. Missing packages can expose the existing SD only after its filesystem/geometry pass preflight; complete CRC checks and command drain precede detach. The author approved publishing D12 after reporting basic stability on2026-10-20.

地图测试覆盖两个真实 SD 包、CRC/文件长度/版本/地图/布局错误、预检失败不擦写、缓存复用、部分写入中断与重试、Flash 写失败、双向扣费与时间、金币不足、长按返回和随机状态不变。
Map tests cover real packs, corrupt identities/CRC/size, no erases on failed preflight, cache reuse, interrupted writes and retry, flash failures, both-way fare/time, insufficient funds, hold-to-return and unchanged fishing RNG.

应用在 0x80023000–0x80080000；460 KiB 地图缓存在 0x80080000–0x800F3000。下载器和末尾 52 KiB 不写入。NOR 初始化和擦写函数均在 ILM，ROM 调用期间关闭中断，调用前后刷新看门狗。原两块 240×24 PFB 保留，另有 4 KiB 加载缓冲和 960 B 夜景行缓冲。
The application ends before 0x80080000. The 460 KiB map cache ends before 0x800F3000; bootloader and final 52 KiB remain outside every write. The flash writer is in ILM, with IRQs masked during ROM operations. Original 240×24 PFB blocks remain; the loader adds a 4 KiB staging buffer and the night decoder uses 960 bytes.

| 工具链/区域 / Toolchain/region | 占用 / Used | 容量 / Capacity | 余量 / Free |
|---|---:|---:|---:|
| GNU / APP_FLASH | 372,736 B | 380,928 B | 8,192 B |
| GNU / ILM | 2,440 B | 131,072 B | 128,632 B |
| GNU / DLM | 128,528 B | 130,304 B | 1,776 B |
| GNU / AHB_SRAM | 32,288 B | 32,768 B | 480 B |

SD payload: 249,508 B; cache header: 4,096 B; unused: 101,084 B.

历史V16存档为1511字节，原8字节猫状态的空余位保存出现/走动状态，独立A/B，保留原密钥、UUID、目录和所有V1–V15文件。装备、声音、金币、图鉴和单鱼售价继续保留。
Historical: the 1511-byte V16 save packs cat visibility/walk state into the existing8cat bytes and uses separate A/B files, preserving keys, UUID, directory and all V1–V15 files. Bighead is appended; old redtail and D6 silver carp records remain valid. Gear, sound, coins, collection and exact fish prices survive.

城市12种鱼采用独立数值与饵料权重，超级体型不为旧装备削弱。三图共用53张公共鱼图、49项可见图鉴，旧红尾保留兼容。路线图和坐船加载图标留在固件，缓存损坏时仍可显示错误与重试。
The twelve city fish use independent profiles and bait weights; giants are not weakened for old gear. Three maps share53 portraits and49 visible collection entries, retaining retired redtail compatibility. Route UI and the seated fisherman/boat loading icon stay in firmware for failure/retry screens.

未执行实板烧录、拔卡/掉电、电气、帧率、栈水位或蜂鸣器试听；中断测试使用主机 Flash 模型。保留第三方库已有宏/未使用变量警告。
No hardware flashing, card-removal/power-loss, electrical, FPS, stack-watermark or audio-audition tests were performed. Interruption tests use a host flash model. Existing third-party warnings remain.

GNU UF2 SHA-256: `4617a764987cbc1849d150d0c4aa0543c11250a3e70b84aa82a3a030696d7487`.
