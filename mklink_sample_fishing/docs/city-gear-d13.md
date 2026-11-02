# D13 都市长竿、雨伞与泡面 / City gear and comforts

## 当前D14调整 / D14 update

断线后仅10%概率损坏线组，其余保留并可继续使用；河豚中钩后逃脱额外20%概率咬断线组；泡面改为4游戏小时、收放线速度+10%，不增加提竿窗口。提示采用钓鱼情境用语，详见[线组与泡面调整](line-damage-d14.md)。V14存档及现有SD资源保持兼容。以下D13及更早章节为历史。 / Line breaks now have a10% rig-damage chance; noodles grant10% handling only for four game hours. Existing saves and packs remain compatible. See the linked D14 guide; older sections are historical.

本地测试版，接续已发布D12；硬件MKLink V4。三件新物品使用新增稳定ID22/23/24，不重排旧ID，不改湿地商店。当前配套资源仍2026.10.19.5，无需重拷。

| 物品 / Item | 商家 / Vendor | 售价 | 效果 / Effect |
|---|---|---:|---|
| 高碳长竿 | 都市鱼摊，06:00–16:00 | 3500G | 最大抛距180世界像素=7.2米，收线效率125%；6.5kg以上鱼每次收线少增加1点张力，仍受所配线组长度及断线规则限制 |
| 雨伞 | 都市便利店，09:00–21:00 | 160G | 永久拥有并自动展开；岸上手持，船上固定。阻止新潮湿积累，已有潮湿按正常棚下速度晾干，不立即清零 |
| 泡面 | 都市便利店，09:00–21:00 | 28G | 即买即食，持续4游戏小时，收放线速度和提竿窗口各+8% |

原热拿铁保持35G、6游戏小时、两项+10%。泡面与拿铁共用一个饮食状态，后购买的替换前一个，不叠加；再次吃同类刷新完整持续时间。已有潮湿仍与饮食加成相乘：满潮湿+泡面为86%基础收放线速度。加载与离线不计时，乘车照常扣除5游戏小时，因此泡面乘车后到期。

新装备可带回湿地使用，但只能在对应都市商家购买；金币不足、重复购买永久装备和错误商家交易均不扣款。长竿不是无限线长或免断线，巨物冲刺、过载和超距规则继续生效。原三根鱼竿参数保持。长竿对500g及以下鱼的低张力松线积累提高50%，500g至1.5kg线性递减为正常；张力保持合适时不额外增加脱钩。没有随机强制跑鱼。小鱼/近岸仍适合旧竿，长竿不会全面淘汰旧装备。

V14独立A/B读取并保留V1–V13旧文件，密钥、目录、装备初始化世代不变。文件仍1503B：雨伞使用位置标记内一个保留位，泡面种类使用原饮食计时字段高位，持续时间占其余位。原V13档默认没有新增装备，已有装备、鱼获、位置、时刻、天气、潮湿、拿铁和声音保持。无需新缓冲或更大存档。

雨伞v2为内置ImageGen重绘的蓝灰布面/木柄，转换为24×28逻辑像素/4位索引图；泡面为生成素材转换到16×16 RGB565图标。与现有原生像素网格按2倍显示，复用既有行缓冲。原图与提示词保留本地，公开demo仅包含运行用C数据。长竿复用碳素竿图标并增加水中实际竿尖长度，避免为装备复制位图。台词和长竿描述可在city-merchant-dialogue.md、tackle-descriptions.md继续修改。

The city tackle stall adds a3500G long carbon rod:180px/7.2m maximum cast,125% retrieval and one less added tension per retrieval detent for fish over6.5kg. Line length, overload and break rules remain. The convenience store adds a permanent160G umbrella and28G instant noodles. The umbrella is carried on shore and mounted on the boat; it prevents further rain wetness while existing wetness dries normally. Noodles grant+8% handling/strike window for four game hours. They replace, rather than stack with, the existing35G six-hour+10% hot latte. Rebuying refreshes the selected meal/drink.

V14 remains1503bytes, uses separate A/B files and preserves all old files/keys/progress, including V13 position/weather. Item IDs are appended; old rods and the wetland shop retain their behavior. The same2026.10.19.5 SD packs remain compatible. Native pixel artwork uses the existing row decoder and no new framebuffer. Hardware verification of these additions is pending.

Long-rod specialization: up to500g, low-tension slack accumulates50% faster, tapering to normal at1.5kg. Keeping proper tension avoids the penalty; no random forced escape is added. Old rods retain their value for small fish and close-range handling.
