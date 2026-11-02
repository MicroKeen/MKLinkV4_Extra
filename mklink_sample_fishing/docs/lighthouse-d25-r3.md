# D25R3 灯塔旋转 / Lighthouse rotation

D26合入说明：以下资源／存档结论仅针对独立灯塔改动；当前整合版本新增生物，须配2026.10.31.4资源，使用V21存档。 / D26 integration requires2026.10.31.4 resources and V21 saves; resource/save statements below describe the earlier lighthouse-only change.

灯塔每12秒连续旋转一周。光束沿水面的透视方向投影：侧向时较长，转向玩家时缩短、展开并变亮，背向时变细变暗；正对玩家时灯室有短暂亮晕。前后方向平滑衔接，取消原来的左右折返与俯仰跳变。

保留薄雾中的丁达尔散射。光锥边缘、远端和雾内亮度柔和衰减，采用原2px像素网格。建筑、岸面和角色遮挡光束。单帧冻结时间供所有PFB条块使用，避免条块之间的角度差异。

复用Arm-2D/CMSIS已有的正弦、余弦及透明度填色。HPM5301工程已启用RV32F硬件浮点；光束按相同透明度合并横向像素段，不新增堆分配、粒子池、位图或整屏缓冲。实际板卡帧率待测。

资源仍为2026.10.31.3、格式3，V20存档不变；已安装该资源版本时只需更新固件。本版本地构建，未上传、未烧录或操作实体SD。

The lighthouse completes a continuous12-second revolution. Its projected beam length, width and intensity change with direction: long side views, a wider/brighter front pass and a narrower/dimmer back pass. Fog scattering and a brief front-facing lens bloom remain, with smooth cycle wrap and foreground occlusion. Existing Arm-2D/CMSIS math and opacity fills are reused; no new heap allocation or full-frame buffer is added. Resources2026.10.31.3 and V20 saves stay compatible. Local build; hardware performance pending.
