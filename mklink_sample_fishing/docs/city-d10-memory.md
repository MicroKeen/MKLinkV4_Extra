# D10 加载后内存核对 / Memory after resource loading

地图载荷447,884 B保存在外部Flash缓存，MCU ELF没有PC模拟器的整包SRAM缓存。资源读入使用4,096 B固定缓冲，日景/夜景/内景共用960 B两行缓冲；这些都已计入下面DLM占用，没有新增整屏缓冲。

| 区域 / Region | 已占 / Used | 容量 / Capacity | 余量 / Free |
|---|---:|---:|---:|
| APP_FLASH | 366,652 B | 380,928 B | 14,276 B |
| ILM | 2,440 B | 131,072 B | 128,632 B |
| DLM | 126,480 B | 130,304 B | 3,824 B |
| AHB_SRAM | 28 B | 32,768 B | 32,740 B |

DLM已包含24 KiB C库堆预留、16 KiB RT-Thread堆池、4 KiB主栈、8 KiB场景栈和4 KiB存档栈；以上不是每次加载再申请的内存，不能重复加算。PFB仍为两块240×24 RGB565：像素23,040 B，含描述符总计23,096 B。

导出的HPM场景在主机运行两图冷启动和缓存启动，再各进入划船、日间岸边、鱼摊、图鉴、大鱼鱼库、夜间岸边、便利店和递热拿铁，每页12帧。加载后所有页面的Arm-2D scratch申请次数/存活/峰值均为0，未出现渲染堆增长。实际MCU的C库/RT-Thread堆峰值和线程栈水位尚未实测；本次没有烧录设备。

Both cold/cache boot paths and eight loaded pages per map were exercised through the exported HPM scene. No Arm-2D scratch allocations occurred. This does not measure physical MCU heap peaks or stack watermarks. Static ELF accounting already includes the reserved heaps and stacks.

通用Map分析脚本误将32 KiB AHB区当作总RAM，并误读部分符号，因此不采用它的汇总告警；本报告按ELF32实际地址、分区和符号表重新计算。明细及逐页面记录见 `city-d10-memory.json`。
