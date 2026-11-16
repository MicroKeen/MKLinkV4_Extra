# 海图物种与体型复核 / Marine roster review

D25新增沙蚕47号，海洋记录23条、可见图鉴47项；7种赶海生物已可采收，评语沿用可编辑MD。以下体型和价格表保持作者修改值。

2026-10-30，D22，平台MKLink V4。原表标题写25种，实际有19行（赶海6、近岸8、远海5）；未补造缺失行。加入本轮要求的鱿鱼和雀尾螳螂虾后，再加青花鱼，共22条海洋记录。原编号0～24不变，新增25～46；退役红尾保留存档位置，总图鉴46项。

下表是**游戏可收获区间**。现实的最大长度和最大重量通常不是同一尾标本，不能直接当作实测长度—重量回归。本版用两个游戏端点拟合幂函数，固件采用Q12定点运算，不引入浮点幂函数。贝螺牡蛎按含壳重量，鱼和虾按全长；短蛸、鱿鱼按胴长，沙蟹按甲宽。没有把触腕总长当作胴长。

| ID | 游戏名称 | 长度cm | 重量 | 稀有度 | 价格基数G | 状态 |
|---:|---|---:|---:|---|---:|---|
| 25 | 口虾蛄 | 8～20 | 4～65g | 铜 | 20 | 近岸钓底 |
| 26 | 真鲷 | 25～80 | 330g～8kg | 银 | 50 | 近岸钓底 |
| 27 | 大黄鱼 | 25～60 | 220g～2.5kg | 金 | 180 | 中下层 |
| 28 | 大梭鱼 | 40～200 | 400g～50kg | 铜 | 50 | 表层，海狼 |
| 29 | 鲻鱼 | 20～70 | 140g～4kg | 铜 | 20 | 表层 |
| 30 | 牙鲆 | 30～90 | 340g～7kg | 铜 | 20 | 钓底 |
| 31 | 六斑刺鲀 | 15～35 | 133g～1.5kg | 银 | 40 | 近岸钓底 |
| 32 | 鞍带石斑鱼 | 80～250 | 13.34～300kg | 金 | 120 | 礁石与陆架钓底 |
| 33 | 鮟鱇 | 30～80 | 410g～8kg | 银 | 65 | 外海钓底 |
| 34 | 带鱼 | 60～150 | 230g～2.5kg | 铜 | 20 | 远海中下层 |
| 35 | 皇带鱼 | 200～500 | 11.48～100.93kg | 彩色特殊 | 1600 | 暴雨远海 |
| 36 | 黄鳍金枪鱼 | 80～210 | 8～160kg | 银 | 55 | 远海表层 |
| 37 | 蓝鳍金枪鱼 | 120～280 | 38.24～350kg | 金 | 105 | 远海表／中层 |
| 38 | 文蛤 | 壳长3～8.5 | 6～120g | 铜 | 20 | 赶海采收 |
| 39 | 牡蛎 | 壳长5～15 | 17～450g | 铜 | 20 | 赶海采收 |
| 40 | 猫眼螺 | 壳长3～9 | 15～200g | 银 | 50 | 赶海采收 |
| 41 | 短蛸 | 胴长5～8 | 40～100g | 银 | 50 | 赶海采收 |
| 42 | 沙蟹 | 甲宽2.5～3.8 | 6～21g | 铜 | 20 | 赶海采收 |
| 43 | 明对虾 | 12～20 | 29～120g | 铜 | 20 | 赶海采收 |
| 44 | 雀尾螳螂虾 | 8～18 | 17～130g | 彩色稀有 | 200 | 口虾蛄抽选后的稀有分支 |
| 45 | 鱿鱼 | 胴长15～45 | 100g～1.5kg | 铜 | 20 | 常见，中下层 |
| 46 | 青花鱼 | 20～50 | 80g～1.5kg | 铜 | 20 | 表中层，夜间灯诱 |

| 47 | 沙蚕 | 6～18 | 1～12g | 铜 | 1 | 沙滩赶海，收入饵盒 |

## 主要修订与依据 / Corrections and references

- **梭鱼分开辨认**：用户确认是海狼Sphyraena barracuda，不能沿用原表20～45cm的小型鲻科鱼区间。游戏改为40～200cm、0.4～50kg；贴图保留尖长吻、下颌、齿列和分开的两片背鳍。鲻鱼另保留钝头、较短吻和不同体形。[FishBase大梭鱼](https://www.fishbase.se/summary/Sphyraena-barracuda)、[澳洲鱼类馆鲻鱼](https://fishesofaustralia.net.au/home/species/3880)。
- **口虾蛄减重**：原16cm／105g偏重。研究样本平均体长15.5cm、重量26.2g，本版8～20cm／4～65g是围绕这一量级的游戏估算，20cm不是该研究的样本上限。[原始研究，材料方法](https://www.mdpi.com/1420-3049/27/7/2320)。
- 真鲷、牙鲆的游戏上限分别80cm／8kg、90cm／7kg，低于资料中列出的物种最大记录；这些端点是玩法选择。[真鲷](https://www.fishbase.se/summary/Pagrus-major.html)、[牙鲆](https://www.fishbase.se/summary/Paralichthys-olivaceus)、[大黄鱼](https://www.fishbase.se/summary/Larimichthys-crocea)。
- 鞍带石斑鱼采用250cm／300kg，保留巨物定位；不把它压低到河流装备的舒适范围。[FAO物种图鉴](https://www.fao.org/4/i0254e/i0254e.pdf)。
- 皇带鱼保留原表2～5m／11.48～100.93kg的游戏区间，红色背鳍、冠状头鳍、银色带状体与普通带鱼区分。暴雨出现是**游戏条件**，不是对现实生态规律的断言。[皇带鱼资料](https://www.fishbase.se/summary/Regalecus-glesne)、[普通带鱼](https://www.fishbase.se/summary/Trichiurus-lepturus)。
- 黄鳍金枪鱼的资料上限约2.1m／200kg，本版重量收为160kg；以黄色长镰形鳍区分。蓝鳍选择太平洋种Thunnus orientalis，采用280cm／350kg的游戏端点，贴图更厚重、鳍较短。[澳大利亚渔业管理局黄鳍](https://www.afma.gov.au/species/yellowfin-tuna)、[太平洋蓝鳍](https://www.fishbase.se/summary/Thunnus-orientalis)。
- 鮟鱇明确为Lophius litulon，避免用另一种较小鮟鱇的上限；六斑刺鲀明确为Diodon holocanthus。[鮟鱇](https://www.fishbase.se/summary/Lophius-litulon)、[IOTC物种识别图鉴](https://iotc.org/sites/default/files/documents/2023/08/IOTC-2023-WPEB19-INF01_-_Species_ID_guide_IRD-Ob7_v2.1_eng.pdf)。
- 猫眼螺缩为3～9cm，重量不再沿用原表636g；重量仍属游戏估算。短蛸用胴长5～8cm、40～100g，不能与连腕全长混算。[FAO头足类卷一](https://www.fao.org/4/i3489e/i3489e.pdf)。文蛤和牡蛎保留合理的含壳游戏区间。[文蛤测量研究](https://li01.tci-thaijo.org/index.php/JFE/article/download/246754/171936/885671)、[牡蛎学名资料](https://www.agriculture.gov.au/abares/research-topics/biosecurity/biosecurity-sciences/potential-distribution-magallana-ariakensis)。
- 鱿鱼采用剑尖枪乌贼Uroteuthis edulis，胴长上限45cm；不是把触腕拉长后的总长。[FAO头足类卷二](https://www.fao.org/4/i1920e/i1920e03.pdf)。
- 雀尾螳螂虾Odontodactylus scyllarus实际上是独立物种，本游戏按用户要求放入口虾蛄的彩色稀有分支，不在生物学意义上称其为口虾蛄变异。采用8～18cm；棒状捕肢、绿色主体和彩色尾部独立绘制。[澳大利亚博物馆野外图鉴](https://lifg.australian.museum/Group.html?groupId=a6HkPlCX&hierarchyId=PVWrQCLG)。

原始表保留于data/marine_species_roster_source.md。游戏数值生成源为tools/prepare_marine_roster_d21.py，输出data/species.json和data/marine_fish.json；评语继续只修改[catch-comments.md](catch-comments.md)。

The supplied table contains19 actual entries despite its25-species heading. Squid and the requested peacock mantis branch bring the addition to21 records. Length definitions are separated, legacy IDs are preserved, and revised endpoints are game limits rather than paired biological records. Scientific references inform identity, anatomy and plausible ranges; the deterministic endpoint power curve is a game model.

## 价格基数（可改） / Editable price bases

本表ID为从0计数的内部编号；评语表的序号为ID+1，可按鱼名对照。

“价格基数G”就是一尾典型体长个体的参考售价。最终售价按体长平方变化：基数 ×（实际体长 ÷ 该鱼种典型体长）²，四舍五入，最低1G；典型体长取当前物种分布均值，稀有度不另外乘价。

雀尾螳螂虾例外：始终按同尺寸口虾蛄售价×10，表内260为26×10的参考值。修改口虾蛄基数即可，导出工具会同步这一格。皇带鱼使用独立基数1600，不额外乘十。

直接修改本表“价格基数G”的整数，然后运行tools/prepare_marine_prices_d22.py。该工具同步C价格表和JSON；已有鱼获按钓到时的已保存售价出售，不按新基数重新估价。长度、重量、分布等列目前是设定说明，调整这些参数时仍需同步物种生成源。

青花鱼为Scomber japonicus。游戏区间20～50cm、80g～1.5kg，铜稀有；体型端点为游戏拟合，不声称同一现实标本同时达到两端极值。[FAO青花鱼图鉴](https://www.fao.org/docrep/pdf/008/t0438e/t0438e23.pdf)。夜间灯诱依据见[SEAFDEC渔业技术资料](https://repository.seafdec.or.th/bitstream/handle/20.500.12067/496/TD-TRB-01.PDF?isAllowed=y&sequence=1)。新增青花鱼。

| 48 | 竖琴螺 | 壳长4～10 | 含壳20～150g | 金 | 80 | 沙滩赶海，单只 |
| 49 | 青蟹 | 甲宽8～20 | 150g～1.5kg | 金 | 80 | 礁石赶海，单只 |
