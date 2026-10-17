# MKLink V4 v29 验证 / Validation

GNU RISC-V GCC 13.2、HPM SDK 1.11.0、SEGGER Embedded Studio 8.24。2026-10-17。

48 项 Windows 检查和 14 项独立 demo 主机测试通过；GNU flash_uf2、flash_xip 与 SEGGER flash_uf2 构建通过。
All 48 simulator checks and 14 standalone host tests pass. GNU flash_uf2/flash_xip and SEGGER flash_uf2 builds pass.

地图测试覆盖两个真实 SD 包、CRC/文件长度/版本/地图/布局错误、预检失败不擦写、缓存复用、部分写入中断与重试、Flash 写失败、双向扣费与时间、金币不足、长按返回和随机状态不变。
Map tests cover real packs, corrupt identities/CRC/size, no erases on failed preflight, cache reuse, interrupted writes and retry, flash failures, both-way fare/time, insufficient funds, hold-to-return and unchanged fishing RNG.

应用在 0x80023000–0x80080000；460 KiB 地图缓存在 0x80080000–0x800F3000。下载器和末尾 52 KiB 不写入。擦写函数在 ILM，ROM 调用期间关闭中断。原两块 240×24 PFB 保留，另有 4 KiB 加载缓冲和 960 B 夜景行缓冲。
The application ends before 0x80080000. The 460 KiB map cache ends before 0x800F3000; bootloader and final 52 KiB remain outside every write. The flash writer is in ILM, with IRQs masked during ROM operations. Original 240×24 PFB blocks remain; the loader adds a 4 KiB staging buffer and the night decoder uses 960 bytes.

| 工具链/区域 / Toolchain/region | 占用 / Used | 容量 / Capacity | 余量 / Free |
|---|---:|---:|---:|
| GNU / APP_FLASH | 370,528 B | 380,928 B | 10,400 B |
| GNU / ILM | 2,240 B | 131,072 B | 128,832 B |
| GNU / DLM | 107,632 B | 130,304 B | 22,672 B |
| GNU / AHB_SRAM | 0 B | 32,768 B | 32,768 B |
| SES / APP_FLASH | 363,696 B | 380,928 B | 17,232 B |
| SES / ILM | 27,728 B | 131,072 B | 103,344 B |
| SES / DLM | 81,672 B | 130,304 B | 48,632 B |
| SES / AHB_SRAM | 0 B | 32,768 B | 32,768 B |

SD payload: 455,404 B; cache header: 4,096 B; unused: 11,540 B.

v8 存档 1318 字节，独立 A/B，保留原密钥、UUID、目录和所有 v1–v7 文件；新增当前地图，旧档默认为湿地。装备、声音、金币、图鉴和单鱼售价继续保留。
The 1318-byte v8 save uses separate A/B files and preserves keys, UUID, directory and v1–v7 files. It adds the current map; older saves default to the wetland. Gear, sound, coins, collection and exact fish prices survive.

城市沿用现有鱼种、鱼饵权重与装备规则；资源包包含现代都市/远处轮渡、气垫筏、灯光层和全部鱼类贴图。路线图和行走小人留在固件，缓存损坏时仍可显示错误与重试。
The city currently shares existing species, bait weights and tackle rules. Its pack contains the modern panorama/ferry, raft, emission layer and fish art. Route UI and walking/loading sprites stay in firmware for safe failure/retry screens.

未执行实板烧录、拔卡/掉电、电气、帧率、栈水位或蜂鸣器试听；中断测试使用主机 Flash 模型。保留第三方库已有宏/未使用变量警告。
No hardware flashing, card-removal/power-loss, electrical, FPS, stack-watermark or audio-audition tests were performed. Interruption tests use a host flash model. Existing third-party warnings remain.

GNU UF2 SHA-256: `574d6f7e64c79718126074440843a905014b55436b24b50d24dd6ce1eb32c652`.
