# 第三方来源与许可

- `config/`、`linkers/`、`scripts/uf2conv.py` 及输入驱动基于同仓库的 `mklink_sample_fire` / `common` 模板，保留原文件的 HPMicro、Arm、Gorgon Meducer 和 EmbedIoT 版权/许可说明。
- Arm-2D、notGUIyet、perf_counter 通过原仓库子模块引用，不复制或修改其官方接口；适用各子模块自己的许可。
- `src/fish/crypto/` 是 HPM SDK 1.11.0 的 TinyCrypt 0.2.8 AES/CMAC/utils 子集，BSD-3-Clause，完整许可证见其 `LICENSE`；文件原版权说明保留。
- `lcd/fatfs/` 来自 HPM SDK 1.11.0 FatFs R0.14b 与 SDK diskio/SPI-SD 适配，ChaN/HPMicro 原许可和免责声明保留在源码中。只定制应用本地 `ffconf.h`，不修改 SDK 安装目录。
- `lcd/adapt/spi_sd_adapt.c/.h` 基于 HPMicro SDK SPI-SD 示例，BSD-3-Clause；恢复 MKLink 原工程的空 GPIO 片选回调，避免操作 LCD 背光 PA26。
- `src/fish/assets/remarks_font.c` 的字形数据由 Noto Sans SC 导出，SIL Open Font License 1.1，见 `src/fish/assets/OFL.txt`；字体生成/Arm-2D glue 的原许可说明保留。
- 船、湖景、鱼种、装备与行囊像素素材来自本钓鱼项目的生成/加工资产，不包含新增下载的第三方图包。发布者为此项目作者。
- 商店木屋、柜台内饰、老人四帧与岸边小路来自内置 ImageGen；老人采用 merchant-v6，仓库只提供实际使用的 C 导出资源，原图、变体和提示词不公开。没有引用《暗黑地牢 2》的现成角色或素材。
