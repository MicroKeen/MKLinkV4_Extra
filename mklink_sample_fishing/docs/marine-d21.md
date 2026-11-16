# D21 海图鱼种与鱼舱 / Marine fish and boat hold

MKLink V4（HPM5301），240×240 RGB565，单颗双向旋转／按下编码器。本版为本地开发版，未推送仓库，未烧录设备。

## 鱼种与资源 / Species and assets

追加21个稳定编号：13种普通垂钓物种、皇带鱼特殊种、雀尾螳螂虾稀有分支，以及6种赶海占位。赶海生物在图鉴显示立绘和“玩法待开放”，不会误混入垂钓鱼池。旧25个编号完整保留，退役红尾占位不展示，总图鉴45项，仍能跨三图查看。详见[物种与体型复核](coast-species-review.md)。

普通遭遇基础权重铜36、银16、金4；地点、钓层、饵料和时间继续乘权重，所以实际概率随选择变化。常见鱿鱼为铜级中下层鱼。海图近岸、内陆架、外陆架、大陆坡、深海各有不同分布；仍只操作顶层／中层／钓底三档，不增加菜单层数。已有淡水鱼不会混入海图。

皇带鱼：世界X≥1200、雨量与风暴强度均≥200/255、中层或钓底、使用适配饵料且有普通候选鱼时，每次抽选独立1%替换为皇带鱼；带猫为2.5%。这是条件内概率，并非所有天气下都有1%。彩色外框，独立基础售价，不额外乘十。

雀尾螳螂虾：先正常抽到口虾蛄，再独立1%替换；带猫为2.5%。它有独立长度、重量和贴图，彩色外框，按口虾蛄同尺寸价格×10。其全局概率还需乘上口虾蛄本身的遭遇率。两种特殊鱼进入海边鱼档时有专属台词、惊讶表情；可编辑[海边员工台词](coast-merchant-dialogue.md)。

7张大型鱼图使用96×56，其余48×28；新增图鉴、收获卡、鱼库均使用公共鱼图包。海鱼卡面限定在180×56的可见区内，保留12像素中文字，避免巨物压住名称或标题。皇带鱼挂船尾另按64px船身约2.5m计算：2m约51px（显示下限64px），5m约128px；头部系绳，仍带入水遮罩。其他船尾鱼沿用20～42px范围。

## 鱼舱 / Boat fish hold

海边鱼档增加“鱼舱”，售价暂定2500G，200kg，永久拥有。它是海滩快艇的设备，不取代原有小木桶／大木桶／储鱼箱。海鱼优先进入鱼舱，剩余空间不足时依次尝试随身容器、空船尾位，最后才放流；单尾超过200kg也可使用空船尾位。

鱼舱留在海滩：离开地图后其中鱼获保留，但其他地图的鱼库和出售菜单不会取出或卖掉它。返回海滩可继续查看／出售。海滩“出售全部鱼获”同时处理本地图鱼舱、随身容器和船尾。仍保存单鱼售价；复用256组售价记录中的末32组给鱼舱，未再申请一整组售价缓冲。

## 装备与挣扎 / Tackle and fight balance

现有城市高碳长竿＋PE高级线组维持100kg附近的可战性；只在体重超过100kg后增加额外拉力。200kg、300kg巨物需要更谨慎收放，现阶段没有另塞海图顶级装备，也没有为旧装备削弱鱼的真实游戏上限。金枪鱼持续快速跑线、石斑重拖与爆发、鮟鱇和皇带鱼较被动。

固定策略、相同16组随机种子测试：100kg金枪鱼钓起14/16，200kg为1/16，300kg为1/16。这只是可复现的控制策略回归，不是玩家胜率预测。另检查安全收线仍有最小进展，耗竭巨物到船边可以直接拉起；没有最后0.2m停止移动、强制等时或体力清零才能上鱼的条件。

## 存档与安装 / Saves and installation

V18存档2037B，独立A/B文件；V17的1511B前缀兼容迁移，海鱼记录与鱼舱状态追加。旧密钥、板卡UUID绑定、存档目录、装备、金币、图鉴、位置、时间、天气与猫状态保留，所有旧版本文件留存。老版本“空船尾”的编号25转换为当前46，避免误当口虾蛄。旧档售价列表末32组的明细迁移时清空，但鱼获数量和总价保留，旧鱼不会丢失价值。

HPM存档回读使用512B分块，逐字节比较完整2037B加密文件（最后一块501B），不重复申请整份回读缓冲。已测写入失败重试、最后一字节回读不一致以及原下载器文件／卷引导扇区保留；没有操作实体SD或格式化。

必须配套完整 **2026.10.30.2／格式3** 资源：common/fish.fgpk及maps下三张图。将压缩包中的FishingGame_Resources_DO_NOT_DELETE整个目录覆盖复制到SD根目录，保留下载器原文件和FishingGame_Saves_DO_NOT_DELETE。不要格式化，勿混用D20固件或旧资源。正常启动资源有效时不弹USB盘；缺失资源仍沿用既有受保护导入流程。

公共鱼图无损压缩后105448B。背包原120×88背景也移入SD场景资源，像素不变，应用Flash腾出21120B；启动／错误图标仍在固件。原应用与460KiB缓存边界不变。两组固定鱼图解码槽仍32256B，无新增堆和整屏缓冲；本版静态SRAM有所增加，以[ELF内存报告](marine-d21-memory.md)为准，实板堆峰值／栈水位未测。

本地原图与完整提示记录：assets/source/marine/generation-d21.json；原生像素预览assets/previews/marine-d21-native-art.png。全部新图由内置image_gen生成，转换脚本仅作裁切、最近/面积缩放、调色和RGB565／Alpha编码。公共工程只保留实际引用的C素材、配套资源和许可证。

This local D21 build adds21 stable marine records, including six future foraging placeholders, common squid, storm-only rainbow oarfish and a peacock mantis rare branch. A2500G200kg coastal boat hold retains cargo when travelling away. Global collection assets remain shared. V18 saves append marine/cabin data and preserve older files and progress. Install the entire matching2026.10.30.2 format3 resource folder without formatting or removing downloader files. Existing gear remains viable around100kg; larger marine fish add physical pressure without a forced final-distance or timer gate. Firmware partitions and fixed decode buffers are unchanged. Host validation does not replace on-device acceptance.
