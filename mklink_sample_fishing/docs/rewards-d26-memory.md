# D26 内存与缓存 / Memory and cache

鱼图使用两组固定解码槽，共32,256B，ELF确认全部位于0xF0400000～0xF0408000的AHB SRAM。支持两张96×56的RGB565＋Alpha同时显示；不使用堆，不挪用PFB或DLM。AHB余量480B，不应再放入大数组。

| 区域 / Region | 已占 / Used | 余量 / Free |
|---|---:|---:|
| APP_FLASH | 371,916B | 9,012B |
| ILM | 2,440B | 128,632B |
| DLM | 128,528B | 1,776B |
| AHB_SRAM | 32,288B | 480B |

D26追加两种金色赶海生物，V21存档2147B且保留V20的2097B迁移布局，当前静态DLM总占用见上表；512B回读缓冲复用逐块校验。DLM包含已预留的24KiB C库堆、16KiB RT-Thread池、主栈及两个线程栈，不应重复计算。加载缓冲仍4KiB，共用行缓冲960B，未增加整屏缓冲。
公共鱼图112,256B，场景249,508B；扣除两个提交头，总缓存余101,084B（公共余14,720B、场景余86,364B）。

三图冷/缓存启动及17／17／35个加载后页面（含赶海）的主机Arm-2D scratch申请均为0。静态解码槽已明确计入AHB占用；scratch为0不代表没有增加静态SRAM。实际设备堆峰值、栈水位和帧率未实测。

Two fixed decode slots occupy32,256B in AHB SRAM, verified from ELF symbols. They support two concurrent96×56 RGB565/alpha portraits without heap allocation or a full-screen buffer. AHB retains480B; DLM retains its reserved heaps/stacks. Host Arm-2D scratch remains zero across three maps; hardware peaks and FPS remain unmeasured.
