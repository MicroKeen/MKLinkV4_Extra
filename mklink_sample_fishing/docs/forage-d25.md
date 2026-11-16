# D25 海滩赶海 / Beachcombing

MKLink V4，2026-10-31，D25R2。沙滩和礁石每天各随机3～6处痕迹，04:00刷新。靠近48px内显示脚边气泡，走远后隐藏，经过痕迹不吸附、不停步，到附近短按采收。岸上右上角以工具图标和数字显示耐久，不带背景底板。铲子120G、撬棍180G，各20耐久、最多持有一把；每次实际采收消耗1耐久，失败也消耗并清除该处，耗尽后可重买。缺工具或饵盒已满不扣耐久。

沙滩有文蛤、猫眼螺、沙蟹、沙蚕，礁石有牡蛎、短蛸、明对虾。贝螺直接采收；其他物种60%触发逃跑，按下完成一次滑块QTE，进入绿色区成功，6.5秒未操作失败。沙蟹较快且滑块较窄，沙蚕较慢且较宽。沙蚕直接收入饵盒，每次抛竿消耗1条；海图等待时间乘65%，不吸引原两图鱼种。其他收获进入现有容器／鱼舱／船尾，满载沿用原放流规则。

当日数量、物种、位置由游戏日期确定，稀有度较高的种类权重较低。位置从各区可行走范围内随机抽取，不按等间隔排列，也不固定两端点；允许随机点紧挨出现，气泡也允许重叠；只排除同坐标，采用2px网格。沙滩范围X124～260、礁石X288～464，避开公路和上船点；交互范围重叠时选最近一处。已发现／已采收标记、工具耐久和沙蚕数量纳入V20存档，读档或往返地图不额外刷新。QTE中断后该点仍已消耗，回到岸边。工具和生物采用生成式像素素材，12px字体、原单编码器操作保留。

配套完整资源**2026.10.31.3，格式3**；新增沙蚕公共鱼图及工具、饵料图标。V20为2097字节，独立A/B文件，保留V1～V19文件、原密钥与UUID。请覆盖完整FishingGame_Resources_DO_NOT_DELETE目录，保留下载器文件与FishingGame_Saves_DO_NOT_DELETE；不格式化。

D25R2 places3–6 daily spots randomly across each walkable beach/reef area, with no fixed endpoints or equal spacing. Passing a trace never snaps or stops the player. Tool durability appears at the upper right without a panel. Refresh remains04:00;20-use tools,60% escape QTE, bait and V20/legacy saves remain supported. Matching2026.10.31.3 resources are required. Local firmware, hardware acceptance pending.

## 可调整初值 / Tuning

| 项目 | 当前值 |
|---|---:|
| 每区域每日数量 | 随机3～6 |
| 刷新 | 04:00 |
| 发现距离／交互距离 | 48px／12px |
| 工具耐久 | 20 |
| 非贝螺逃跑率 | 60% |
| QTE时限 | 6500ms |
| 短蛸滑块宽／周期 | 16px／1400ms |
| 沙蟹滑块宽／周期 | 16px／1200ms |
| 明对虾滑块宽／周期 | 18px／1500ms |
| 沙蚕滑块宽／周期 | 24px／1700ms |
| 绿色判定区 | 滑块中心56～107px |
| 沙蚕库存上限 | 999 |
| 沙蚕海图等待乘数 | 65% |

沙滩权重：文蛤36、猫眼螺14、沙蟹22、沙蚕28；礁石：牡蛎48、短蛸16、明对虾36。均为各区的相对权重。工具描述在[tackle-descriptions.md](tackle-descriptions.md)，鱼类评语在[catch-comments.md](catch-comments.md)，价格基数在[coast-species-review.md](coast-species-review.md)。

## 同版海滩修正 / Coast revision

D25R2：工具耐久在岸上右上角，无背景；赶海点按区域随机分布，经过不停步。海岛蓝色岩体的天空遮罩与雾层视差已修正；灯塔光束在夜色之后显示，随转向形成柔和光锥，雨雾中散射增强，并由前景建筑遮挡。快艇二档由20提高到60px/s，一档10px/s；高速时增加向船尾扩散的泡沫尾浪。票价为湿地↔江河180G、江河↔海滩280G、湿地↔海滩460G；加载成功扣费，失败不扣、重复回调不重复扣，乘车时间保持原设定。完整配套资源2026.10.31.3；V20存档保持兼容。

D25R2 adds freely walkable random traces, a transparent upper-right tool HUD, corrected island/sky masking and parallax, fog-scattered rotating lighthouse light, and60px/s fast motorboat wakes. Fares are180G wetland–river,280G river–coast and460G wetland–coast. Successful arrival charges once. Matching2026.10.31.3 resources are required; V20 saves remain compatible.

痕迹采用尖端向下的水滴气泡，移除地面黑点；随机点允许紧挨出现，只排除同坐标，交互优先最近点。QTE最快档往返周期1200ms，滑块16px；绿色区统一52px，其他档保留差异。短蛸1400ms／16px、明对虾1500ms／18px、沙蚕1700ms／24px；连续有效按压窗口均不短于200ms。

气泡尖端贴近脚边，距角色48px内才显示，走远隐藏，再靠近重新出现；已采收点不再显示，发现标记仍保留在存档。气泡位于角色后方，避免盖住腿部。 / Ground-level pins appear within48px, hide outside that radius and reappear on return; harvested spots stay hidden. The actor occludes overlapping pins.
