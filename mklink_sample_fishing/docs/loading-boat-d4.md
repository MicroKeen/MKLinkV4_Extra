# D4：加载页坐船图标 / D4: boat icon on destination loading

2026-10-19，作者确认 D3 已成功加载进入游戏。D4 按反馈将进度条上方的行走小人及独立鱼竿，换回此前的坐船钓鱼图标。

复用固件内已有的48×40索引图，以整数2倍显示为96×80，固定居中于 `(72,74)`，下方仍为原进度条、百分比与目的地提示。加载页不再使用人物变换操作或额外线段，保留加载/路线页面的提前返回。没有新增素材或修改SD资源包，原存档继续兼容。

5项相关检查通过：实际导出的HPM场景两图首次/有效缓存启动、电脑版完整启动、切图画面和资源导入画面。GNU固件编译成功；应用Flash为371,992 B，余量8,936 B，比D3少96 B，DLM不变。检查了60%城市加载画面，图标与标题、进度条均无遮挡。仍仅本地提供，不推Git。

The author confirmed that D3 successfully loaded the game. D4 replaces the walking sprite and separately drawn rod with the existing seated-fisherman/boat icon. The resident 48×40 indexed asset is drawn at integer 2× scale, centred above the existing progress bar; no extra artwork, map-pack update or transform is required. Saves remain compatible. Five relevant checks pass, including both exported HPM scenes with cold/valid caches, simulator boot, travel rendering and resource-page rendering. The GNU app uses 371,992 Flash bytes with 8,936 free, 96 fewer than D3; DLM is unchanged. Distribution remains local without Git push.

## D5：加载提示 / Loading hints

加载页新增“正在整理钓具”“正在前往码头”，与原目的地描述每7秒轮换，两图昼夜通用。保持坐船图标和原字号，补齐“整”字；资源包无需更新。5项相关启动与渲染检查通过。应用Flash 372,084 B，余量8,844 B；DLM不变。

D5 adds the shared tackle-preparation and dock-departure hints to the existing seven-second rotation on both maps, including at night. The boat icon and font size remain. Five relevant checks pass; no pack update is required.
