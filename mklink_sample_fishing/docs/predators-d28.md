# D28 储鱼箱与鱼种挣扎 / Cargo and predator behavior

MKLink V4，2026-11-02。江河鱼摊不再售卖20kg大木桶，保留35kg储鱼箱750G，新增80kg大型储鱼箱1800G。已有容器保留；大型储鱼箱沿用随身容器规则，可带往三图，海滩鱼舱仍独立为200kg。复用现有储鱼箱图标，不增加素材包。

容量升级保留鱼获的数量、重量和历史售价；船尾鱼能放入时移入箱内，不能放入时继续挂船尾。相同容器不重复扣款，不允许买小容器降级。20kg桶仍可在湿地购买；湿地和海滩不出售新增80kg箱。

沿用固定224组随身售价明细，满后继续保留鱼种数量、重量和总售价，出售未记录的部分按合计反馈；不因此拒收鱼获或扣减收益。80kg箱的明细满载／保存／出售已有回归检查，不增加RAM。

## 挣扎特征 / Fight traits

黑鱼提竿成功时立即给予按体型和线组响应计算的第一下重压：25＋基础拉力×线组响应／100，上限为当前线强度90%，保留700ms过载宽限。小鱼、强线与大鱼、弱线的感受不同；其余鱼种仍从25张力起步。

鳡强化快速外冲；斑鳜提高短冲力度、缩短周期，保持1000ms平静间隙；大梭鱼及金枪鱼在发力时保留超过共用拉力上限的额外空间。巨型石斑保留沉重拖拽，避免所有掠食鱼使用同一节奏。参数是游戏设定，不是生物测量数据。

| 鱼种 | 发力／周期ms | 突冲加成 | 发力拉力上限额外值 | 冲刺距离倍率% | 首口系数% |
|---|---:|---:|---:|---:|---:|
| 黑鱼（乌鳢） | 1800/2800 | 30 | 10 | 135 | 100 |
| 斑鳜 | 1400/2400 | 55 | 15 | 175 | 0 |
| 翘嘴鲌 | 600/1600 | 35 | 12 | 185 | 0 |
| 鳡 | 1600/2600 | 65 | 25 | 235 | 0 |
| 大梭鱼 | 1400/2400 | 58 | 20 | 210 | 0 |
| 鞍带石斑鱼 | 3900/5600 | 48 | 15 | 210 | 0 |
| 黄鳍金枪鱼 | 1300/2300 | 65 | 20 | 230 | 0 |
| 蓝鳍金枪鱼 | 2200/3200 | 75 | 20 | 250 | 0 |

额外上限只作用于发力段，平静及疲劳仍有回收窗口；600ms预警、70%向远离玩家一侧突冲、偶尔向内游动、两秒竿体弯曲救线保留。安全收线在所有距离都有正进度，没有最后一段距离锁或强制搏鱼时长。尺寸、稀有度、出现权重和鱼价未改。青鱼保留上一版沉重持续拉扯。

## 存档和资源 / Saves and resources

新容器索引使用V22独立A/B文件，文件仍2147B，原密钥／UUID／目录不变。读取V1～V21；只写V22，旧文件保留，避免旧固件将新箱误读成小木桶。原容器不会自动变为新箱。历史路径按版本和A/B生成，复用已在文件系统锁内使用的512B缓冲，不增加静态RAM。

2026.10.31.4四个资源包保持逐字节不变，已安装该版本时只需更新UF2。未烧录设备、未操作实体SD；实际手感和掉电表现仍待实板验收。

The city tackle stall replaces its20kg bucket offer with an80kg large cooler priced at1800G, retaining the35kg cooler. Existing ownership, cargo and historical individual prices survive upgrades; the portable cooler travels across regions. No new artwork is needed.

Snakehead gains immediate but bounded hook-set pressure. Yellowcheek, mandarin, barracuda and tuna retain distinct burst cadence, force and run multipliers; grouper retains heavy sustained drag. Calm/fatigue phases and warning/rescue windows remain. Safe reeling always progresses, with no artificial landing timer or final-distance gate. The audit uses deterministic input policies, not predicted human win rates.

V22 keeps the2147-byte layout while using its own A/B names, reads V1–V21 and leaves older files untouched. Matching2026.10.31.4 resource packs are unchanged. Hardware acceptance is pending.
