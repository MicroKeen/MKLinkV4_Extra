# D22 海钓装备、深度与诱鱼灯 / Coastal gear, depth and fishing light

平台MKLink V4（HPM5301），240×240 RGB565，单颗双向旋转／按下编码器。本地开发版，不自动推送或烧录。

## 鱼种与昼夜 / Species and time

大黄鱼改为金稀有。鱿鱼、带鱼白天在中下层，夜间18:00～次日06:00进入表层和中层；习性页同步显示。真鲷、鲻鱼、金枪鱼和大黄鱼也有各自的日间／晨昏／夜间权重，具体在data/marine_fish.json；这些是游戏规则，保留同一鱼种在不同时段的差异。

仅新增**青花鱼**（Scomber japonicus，亦称鲐鱼），没有加入竹荚鱼。铜稀有，20～50cm、80g～1.5kg，价格基数20G，主要在表／中层；青绿色背部深色波纹、银白腹部、小离鳍与其他海鱼区分。素材从原始大图导出为96×56，使用城市大鱼同规格解码；卡面显示大小不变，提高背纹与轮廓清晰度。总计47个稳定编号，总图鉴46项，6种赶海生物仍仅占位。

## 海边货架 / Coastal shop

| 物品 | 售价 | 描述（可改） | 效果 |
|---|---:|---|---|
| 重型海竿 | 6000G | 深水省力，近抛重装 | 以高碳长竿为基准：抛距−30%，收线速度+20%；对应126px／5.04m、基础收线150%。深水空钩回收也享受加成。 |
| 海钓线组 | 1200G | 放到底，也得慢慢收回来 | 线长200m，可到达海图最深处；强度240，需要玻纤竿或更好的竿，小鱼不易保持张力。 |
| 诱鱼灯 | 2800G | 夜里招来趋光的鱼 | 永久安装在快艇船头，在背包侧面灯具格短按切换；夜间对目标表层鱼增加概率并缩短等待。 |

海边仍卖八编PE线组和200kg鱼舱，保留出售鱼获／查看容器／返回。旧鱼竿、旧浮漂、旧饵料、随身容器不再摆上海图货架，已拥有的装备可携带使用。原免费细线应急补给在符合救济条件时出现，属于免费服务。

以上描述的正式可编辑输入是[鱼竿、鱼漂与新物品描述表](tackle-descriptions.md)。修改后运行tools/prepare_tackle_descriptions.py、tools/prepare_remarks.py并重新构建，继续使用12px字号。鱼类评语仍读取[catch-comments.md](catch-comments.md)，用户修改的评语已合入。

## 用线量、下沉与回收 / Line geometry and depth

海图把水平抛距和钓深合成为用线量：线距≈√（水平距离²＋钓深²）。搏鱼时显示“线距”和当前钓深；原两张地图继续使用原来的距离规则。

- 其他旧线组只能钓表层。
- 八编PE线组允许钓中层；近岸水深严格小于15m时，也允许尝试钓底。八编线长仍10.5m，**仍需合成距离不超线长**；允许浅底不代表15m内所有位置都一定够长。
- 海钓线组允许中层和钓底，200m线长足够当前约125m的最深投点。
- 目标深度或合成距离不满足线组条件，抛出后显示“线组不够长”，钓层切回表层，不在不可达深度等待。

海图深水钓组以2m/s逐步下沉，进度条和钓深同时变化，到位后再开始等待咬钩；下沉中短按可收回。空钩回收基础3m/s，再乘鱼竿收线效率和当前潮湿／饮食效果，深水自然需要更多时间。重型海竿约4.5m/s，高碳长竿约3.75m/s（无其他状态时）。搏鱼收线与鱼的跑线也换算成实际距离；没有固定收尾等待或最后距离门槛。

## 诱鱼灯 / Fishing light

目标仅为**鱿鱼、带鱼、青花鱼**。开启且处于夜间、实际钓层为表层时，适配饵料和区域内的目标鱼抽选权重×4；选中目标鱼后的等待时间再×0.75。加成保留饵料、线组隐蔽性和体型规则，不保证上钩或成功拉起，不增加变异／皇带鱼概率。灯能把鱿鱼、带鱼引到近岸陆架的表层；中层、钓底、白天不获此加成。开关选择跨地图、重启保留。

船头灯具在场景中缩为12×16px，灯光亮区由64×20扩大为96×28px；使用生成的像素素材与独立半透明海面椭圆贴图，叠加在夜色之后；朝左、朝右时同步，靠岸时裁掉陆地部分，不把亮区盖到岸上。灯具按现有2倍像素接口正确显示，不悬在船外。

沙滩重新生成接近实际比例的像素贴图，远端上沿改为平缓岸线并与海平线对齐，近处保留细浪花；原岛屿、灯塔、建筑、礁石和码头保留。新原图与提示记录在assets/source/beach/generation-d22.json，本地保留。

固定近岸、细线、普通假饵的5000次夜间抛竿回归：目标鱼被选中3420→4643次，目标等待均值2585→1947ms。该数值是固定场景回归，不代表所有钓点的总体胜率。

灯诱依据：[SEAFDEC渔业技术资料](https://repository.seafdec.or.th/bitstream/handle/20.500.12067/496/TD-TRB-01.PDF?isAllowed=y&sequence=1)将鲭类列入灯诱捕捞对象；[ICES研究报告](https://orbit.dtu.dk/files/152261391/Publishers_version.pdf)记录剑尖枪乌贼的灯光诱集研究。带鱼夜间上浮有种群与年龄差异，本版按用户要求作夜间表／中层的游戏设定；倍数和等待减免均为游戏参数。

## 数值、存档与资源 / Values, saves and resources

[鱼类数值表](coast-species-review.md)已增加“价格基数G”和计算方式，22条海洋记录都可调整。运行tools/prepare_marine_prices_d22.py可把本表价格同步到JSON和C价格表；雀尾螳螂虾始终继承口虾蛄同尺寸售价×10。已有鱼获沿用钓到时保存的售价，后续降低基数也不会因此拒绝读档或减少原售价。

V19存档为2062B，新增青花鱼记录较V18增加25B。固定保留V18的2037B布局，兼容较短的V19开发档和V17等旧版本；旧档、密钥、UUID绑定、装备、金币、鱼获、图鉴、位置、时间、天气及猫状态保留。灯拥有／开关复用船具标志位，不增加整套缓冲；回读仍512B分块，最后14B也完整验证。各版本存档文件留存，不格式化SD。

配套资源必须完整更新为**2026.10.30.6／格式3**，包含公共鱼图common/fish.fgpk和三张地图。使用本版压缩包中的FishingGame_Resources_DO_NOT_DELETE完整目录，保留下载器原文件和FishingGame_Saves_DO_NOT_DELETE。正常启动资源有效时仍不弹USB盘。

运行素材以外的原图和提示留在本地：assets/source/coast-tools/generation-d22.json、assets/source/marine/generation-d22.json。公共工程只包含实际使用的C素材、资源包和许可证。固定两组鱼图解码槽32256B、原应用／460KiB缓存边界及PFB不变，内存占用以[本版ELF报告](coast-tools-d22-memory.md)为准；实机堆峰值、栈水位和帧率待测。

D22 adds a6000G heavy marine rod,1200G200m sea rig and2800G bow light. Relative to the city long rod, the heavy rod casts30% less and retrieves20% faster, including empty-rig recovery. Marine line range includes depth; rigs visibly sink and retrieve, and insufficient line falls back to the surface with a clear message. Squid and hairtail move upward at night; the added common chub mackerel is named青花鱼. The night light increases the three target species' surface-selection weights and shortens their waits. Editable price bases and item/catch descriptions feed generated C data. V19 appends25bytes while retaining V18 and older progress. Install the entire matching2026.10.30.6 resource folder. Local build; hardware acceptance pending.
