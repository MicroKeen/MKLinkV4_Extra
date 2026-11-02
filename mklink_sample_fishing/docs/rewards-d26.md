# D26 赶海回报、金色生物与深海延伸 / Shore rewards and deep water

文蛤、牡蛎每处2～4只；沙蚕每处3～5条；其他生物每处1只。每次实际采收只消耗1点耐久，失败仍扣1点。容量不足时明确显示收下与放走的数量；沙蚕只装入剩余饵盒空间，库存不会超过999。

沙滩新增金色竖琴螺，礁石新增金色青蟹，价格基数均为80G。每个痕迹有3%概率替换成所属区域的金色生物，总刷新量仍各3～6处。竖琴螺直接采收，青蟹保留60%逃跑QTE、1500ms往返周期和52px判定区。两者使用新像素贴图，跨地图共用图鉴；旧编号0～47保持，新编号48、49。

偏低铜级鱼获基价抬到20G，偏低银级抬到50G；已有更高价格、金级和尺寸平方倍率保留，变异仍为母种十倍。旧容器、鱼舱、船尾鱼获的存档售价保留，不重新估价。完整当前表见[价格基数](fish-price-bases-d26.md)，评语仍可在[catch-comments.md](catch-comments.md)编辑。

海滩总长1440→1680px，快艇最右位置1368→1608。大陆架1080和既有有效钓点的深度不变，新增右侧一屏延续125m深海缓台。灯塔保留12秒连续旋转、朝向玩家时展开增亮、背向时变细变暗，雾中柔和散射；前景建筑、岸面、角色正确遮挡光束。

V21存档2147B，独立A/B；V20的2097B文件、赶海状态、饵料、装备、金币、图鉴、当前位置和天气均可迁移，V1～V19继续支持，旧文件不删除。必须配套完整2026.10.31.4格式3资源。覆盖FishingGame_Resources_DO_NOT_DELETE目录，保留下载器及FishingGame_Saves_DO_NOT_DELETE，不格式化。

D26 adds2–4 clams/oysters and3–5 ragworms per successful node, with one tool use per attempt and explicit partial storage feedback. Gold harp snails and green mud crabs each have an80G base and a3% replacement chance. Low bronze/silver bases rise to20G/50G; higher bases and historical cargo values remain. The coast extends by240px to1680px, preserving the shelf and adding a125m deep-water plateau. Continuous projected lighthouse rotation is retained. V21 saves migrate V20 and earlier progress. Install complete matching2026.10.31.4 resources; local build, hardware acceptance pending.

## 本轮核验 / Validation

127项主机检查通过；96日全区域采收模拟（空容器起步，按每处工具成本6G／9G摊销），逃跑QTE全失败时平均净收益273.55G／日，75%成功时316.36G／日，另有沙蚕收入。此为固定种子的经济审计，不是实际玩家收益保证；未计交通票价，未出售沙蚕。

53张鱼图解码与透明边界通过；44组鱼类配置、2112次末段拉起、680次深水安全收线通过。检查了批量只装下一只、饵盒998→999、旧档迁移和最右端125m抛竿／存档。灯塔12秒旋转连续性通过，预览按125ms步进覆盖一周。实板帧率、堆栈水位与掉电仍待验证。

Flash通过共享存档路径前缀减少重复字符串，完整路径保持原样，36个版本槽位逐一比对，实际FatFs内存盘写入／回读及下载器文件保留检查通过；没有格式化或操作实体SD。空间详见[内存报告](rewards-d26-memory.md)。
