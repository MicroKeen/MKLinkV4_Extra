# 运行素材与移植契约

## 最新 v29：路线与城市 / Latest v29: route and city

右岸车站现可短按进入路线图。“湿地湖泊”与“城市江河”之间以道路表示远距离，局部场景图随选中变清晰；旋转选择，短按乘车，长按600 ms返回。**每程180G、5个游戏小时，往返相同**，加载失败不扣费、不推进时间。城市采用选定的现代天际线、远处轮渡、气垫筏、独立窗灯/霓虹/倒影层和动态射灯，原有鱼种、装备与鱼库规则保留。

The east-bank station opens a schematic road route. Scene-backed anchors become clearer when selected. Rotate to choose, tap to travel, hold 600 ms to return. **Every trip costs 180G and five game hours in either direction**; failed loads do not charge or advance time. The city uses the selected skyline/ferry, raft, aligned emission layer and sweeping spotlights, with existing fish/tackle/cargo rules.

背景、世界与鱼类贴图改为SD资源包，切图时写入独立460 KiB Flash缓存；应用限制在0x80023000–0x80080000，缓存0x80080000–0x800F3000。先完整校验源文件，最后提交缓存头；路线页和加载人物留在固件。首次运行请先复制SD资源目录，原存档目录保留。v8独立A/B存档新增所在地，保留v1–v7与已购装备、声音、图鉴和鱼获。

World/fish art is loaded from SD packs into a dedicated 460 KiB Flash cache. Source preflight and a last-written commit header protect interrupted loading; route/loading art remains in firmware. Copy the SD resource directory before first boot and keep the original saves. Independent v8 A/B files add location while preserving v1–v7, gear, sound and catches.

完整部署、分区、协议与验证限制见 [地图与SD资源 / Maps and SD resources](maps-and-sd-resources.md)。下方带版本的章节为历史记录，以本节v29为准。


公开 demo 只包含实际运行使用的 C 资源，位于 `src/fish/assets/`：湖景、云/芦苇/船、岸边码头、小路、11 种鱼、装备与行囊、商店外屋/柜台、四帧老人、三档容器、九种柜台商品和中文位图字形。RGB565、A8 和 A1 数据由既有 Arm-2D tile 描述读取。字体许可见同目录 OFL.txt。

原图、未采用版本、PNG、图集、TTF 源字库、提示词和截图不随仓库提供。常规固件编译直接链接 C 资源，无需任何图片或字体生成工具。`scripts/verify_public_assets.py` 检查非运行素材缺席以及位图数组确实链接。

原湖景为 480×240，远景相机偏移为正相机坐标除 3；`fish_sky_horizon.c` 每 2 个源像素保存一个原山林边界，共 240 B。夜空、星点和月亮只画到边界之上，不能覆盖远山。太阳/月亮仍沿既有时间轨迹移动。小路和外屋以相同 2 像素网格显示，小路与西岸重叠 8 像素，现有码头在上层绘制；岸上相机经过世界 x=0 时连续移动。

`data/species.json`、`equipment.json`、`economy.json` 和 `shop_layout.json` 保存稳定 ID、整数 mm/g/coins 和布局。移植时以这些数据及同名 C 规则为准；不要重排存档的鱼种、渔具、饵料或容器索引。商品浏览仍是单编码器 ROTATE/PRESS/RELEASE。

游戏核心不依赖 SDL 或 Arm-2D；移植层传入单调毫秒和输入边沿，绘制消费状态快照。鱼种、体型、咬钩等待与搏鱼消耗同一 xorshift 状态，必须保留相同抽样顺序。夜光漂的夜间加成作用于真实的咬钩等待概率，不修改鱼种权重。存档 v3 为 236 B，兼容读取 v1/v2；UUID 和密钥来源由平台提供。尚未创建 Android/微信运行项目。

时间活跃系数见species.json/game_distribution.activity_percent，顺序白天/晨昏峰值/夜间，按game-design.md线性插值，再乘环境权重。斑鳜白天与夜间均100%，无夜间偏好。
