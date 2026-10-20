# D8 都市存档排查与气垫船 / City saves and seated raft

2026-10-19，MKLink V4，本地测试版，不推送仓库，不操作用户设备。

本次实机反馈：都市手动存档显示“存档失败”或“存档异常”，未测试返回湿地。尚无实机错误码，因此不能确认本次故障的唯一原因。

## 存档改动 / Save changes

- 修复启动发生一次文件系统I/O错误后，本次运行永久禁用保存的状态。手动保存会重新检查存储可用性；不重新初始化正在玩的游戏，不清空本次鱼获、金币或装备。
- 重试前先读取两个当前存档。若发现本次启动尚未恢复的进度，拒绝覆盖，显示`S33/00`，应重启以读取原进度。任何无法完整读取的存档继续受到保护；两个存档都异常时仍不自动覆盖。
- 写入前验证当前数据，避免把不可恢复的数据写入A/B文件。写盘失败保留原有效槽，下一次手动保存可重试。
- 错误提示下显示`S阶段/返回值`。返回值沿用FatFs：`01`磁盘I/O错误，`03`设备未就绪，`07`写入被拒绝。没有依据时不执行格式化、修盘或删除。

| 阶段 | 含义 / Meaning |
|---|---|
| 01 | 获取SD访问权或挂载失败 / SD ownership or mount |
| 02–03 | UUID或存档目录初始化 / UUID or directory initialization |
| 04–06 | 打开、读取、关闭旧存档 / Open, read, close existing save |
| 07–12 | 打开、写入、截断、同步、关闭、回读比较 / Open, write, truncate, sync, close, verify |
| 13–16 | 存档线程、信号量或请求状态 / Save worker or queue |
| 17 | 文件长度或读回长度异常 / File length mismatch |
| 32 | 启动存档解密或内容校验异常；后半为槽编号 / Invalid authenticated save or contents; suffix is slot |
| 33 | 有未读取的进度，需要正常重启恢复 / Existing unread progress needs a restart |
| 34 | 当前数据未通过写入前校验 / Live snapshot rejected before writing |

V10文件格式仍为1485字节，保持原密钥、设备UUID、文件名、目录及V1–V9迁移。不会为了绕过错误新建一套空进度，也不会删除旧存档。

Manual saves now retry transient boot storage failures without resetting live progress. Both current slots are checked before any overwrite. Unread restored progress is protected and requires a normal restart. Snapshots are validated before writing; failures show stage/FatFs details. V10 stays 1485 bytes with all existing keys, paths and older-file migration preserved. The user's specific physical-device failure remains unconfirmed until tested with this build.

## 气垫船 / Raft

原处理脚本截断人物所在木船，再把空艇压到下面12行，人物与艇身之间出现空隙。改用ImageGen重绘的完整坐姿图，统一导出64×36 RGB565和透明蒙版；下身落在座板处，前侧气囊遮住下部。鱼竿握点同步调整，核对左右朝向和四个浮动相位。上岸后只显示底部艇身，不残留人物。

The old pipeline cut the wood boat and pasted a compressed empty raft beneath it. The new generated sprite joins the seated character, bench and inflatable tubes; rod attachment follows the hand. Both directions and all bob phases are rendered for review. The wetland boat and loading icon are unchanged.

配套资源升级为 **2026.10.19.3**，需将完整`FishingGame_Resources_DO_NOT_DELETE`目录更新到原SD根目录；保留存档目录和下载器原有文件。资源载荷大小不变：454,008 B，外部Flash缓存余12,936 B。不新增整屏缓冲。

Use matching **2026.10.19.3** resource packs. Preserve the existing downloader files and save folder. Resource payload is unchanged at 454,008 B; cache headroom remains 12,936 B. No full-frame SRAM buffer is added.

## 验证 / Validation

新增测试使用实际HPM存档worker、实际FatFs及内存中的合成FAT16盘，不连接物理设备：湿地到都市、每个有效鱼种实际入库路径、反复A/B写入、重启读取、注入真实磁盘写入失败、验证原有效槽仍可读取、清除故障后再次保存。测试前后启动扇区一致。

另外覆盖启动I/O失败后的手动恢复，以及存储恢复后出现尚未读取的存档时拒绝覆盖。64项主工程、23项独立demo测试通过；103张实际PFB截图含左右朝向、浮动帧与错误码页面。ELF静态内存和加载后Arm-2D scratch见`city-d8-memory.md`。物理设备栈水位、断电与本次都市故障仍待实机验证。

The added test uses the actual HPM save worker and FatFs against a synthetic in-memory FAT16 volume: city travel, landed fish, A/B cycles, reboot restore, injected disk write errors, surviving active-slot recovery and successful retry. The volume boot sector is preserved. Root tests: 64; standalone demo tests: 23; actual PFB frames: 103. This does not replace physical-device validation.
