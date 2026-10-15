# MKLinkV4_Extra
基于 MKLink 的 Vibe Coding 整活实验室。一起用 AI 写小游戏、做互动装置，把脑洞变成能跑、能玩、能分享的开源作品

## 获取项目

首次克隆时一并下载子模块：

```sh
git clone --recurse-submodules https://github.com/MicroKeen/MKLinkV4_Extra.git
cd MKLinkV4_Extra
```

如果已经克隆了主仓库，在仓库根目录执行：

```sh
git submodule update --init --recursive
```

拉取主仓库更新后，使用同一条命令将依赖同步到仓库记录的版本。

## 目录

- `mklink_sample_fire/`：Arm-2D 火焰示例工程。
- `mklink_sample_fishing/`：240×240 RGB565 单编码器钓鱼游戏，含图鉴、装备与 SD 手动/自动存档。
- `common/`：公共驱动、适配代码及显示组件。
- `hpm5301evklite/`：开发板配置与支持代码。
- `external/`：通过 Git 子模块管理的 notGUIyet、perf_counter 和 Arm-2D。

示例工程使用 HPM SDK，构建前需要配置 `HPM_SDK_BASE`。

钓鱼示例的构建、操作、旧存档兼容性与验证记录见 [mklink_sample_fishing](mklink_sample_fishing/README.md)。其 SD 适配保留 PA26 为屏幕背光；运行期间不启用 USB 磁盘，以避免与游戏同时写 SD 卡。

## 火焰示例与 USB 日志

`mklink_sample_fire` 默认使用无传感器模式。接入提供 `DATA_GY_ACC` 的加速度传感器驱动后，可通过 CMake 的 `-DFIRE_AXIS_INPUT=ON` 启用姿态输入。

`printf` 输出重定向到 USB CDC：GCC 使用应用提供的 `_write` 覆盖 SDK 的弱实现；生成的 SEGGER 工程通过预处理宏重命名 SDK 的 `__SEGGER_RTL_X_file_write`，由应用提供原名实现，无需修改 SDK 文件。修改 CMake 配置后需要重新生成 SEGGER 工程。

串口终端需要打开 CDC 端口并置位 DTR 才会发送。日志使用 1024 字节 Cherry Ringbuffer，单次 USB 发送最多 512 字节；未连接或主机接收过慢时，队列满后的新增日志会被丢弃，不阻塞应用。此重定向针对标准库 `printf`，不改变 RT-Thread 的 `rt_kprintf` 输出路径。

SEGGER 工程在构建完成后自动调用 `scripts/uf2conv.py`，在项目根目录生成 `mklink_sample_fire/MicroLink.uf2`（Family ID `0x0A4D5048`，起始地址 `0x80023000`）。需要 `python.exe` 在 SEGGER 的 PATH 中可用。该步骤由 CMake 生成，Debug 和 Release 均生效，并覆盖同一个 UF2 文件，无需手动复制脚本到输出目录。此地址配置用于 SEGGER 固件；GCC 的链接布局不同，未添加同样的转换步骤。
