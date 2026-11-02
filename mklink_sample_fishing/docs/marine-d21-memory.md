# D21 内存与缓存 / Memory and cache

鱼图使用两组固定解码槽，共32,256B，ELF确认全部位于0xF0400000～0xF0408000的AHB SRAM。支持两张96×56的RGB565＋Alpha同时显示；不使用堆，不挪用PFB或DLM。AHB余量480B，不应再放入大数组。

| 区域 / Region | 已占 / Used | 余量 / Free |
|---|---:|---:|
| APP_FLASH | 358,520B | 22,408B |
| ILM | 2,440B | 128,632B |
| DLM | 128,528B | 1,776B |
| AHB_SRAM | 32,288B | 480B |

D21新增海鱼进度、鱼舱和较长存档，静态DLM占用有所增加；512B回读缓冲复用逐块校验。DLM包含已预留的24KiB C库堆、16KiB RT-Thread池、主栈及两个线程栈，不应重复计算。加载缓冲仍4KiB，共用行缓冲960B，未增加整屏缓冲。
公共鱼图105,448B，场景232,716B；扣除两个提交头，总缓存余124,684B（公共余21,528B、场景余103,156B）。

三图冷/缓存启动及加载后页面的主机Arm-2D scratch申请均为0。静态解码槽已明确计入AHB占用；scratch为0不代表没有增加静态SRAM。实际设备堆峰值、栈水位和帧率未实测。

Two fixed decode slots occupy32,256B in AHB SRAM, verified from ELF symbols. They support two concurrent96×56 RGB565/alpha portraits without heap allocation or a full-screen buffer. AHB retains480B; DLM retains its reserved heaps/stacks. Host Arm-2D scratch remains zero across three maps; hardware peaks and FPS remain unmeasured.
