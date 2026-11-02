# 海边鱼档员工台词 / Coastal fish-stall dialogue

D21独立台词表。每句1～16字符；保留ID与四列。表情0招呼、1成交高兴、2犯愁、3惊讶。修改后运行tools/prepare_merchant_dialogue.py，再重建12px字库。一般情境每次选一句；皇带鱼优先于雀尾螳螂虾专属招呼，卖掉后恢复普通台词。

Edit the two lines and expression index without changing IDs. Regenerate the dialogue header and font before building.

| 情境 | 表情 | 台词一 | 台词二 |
|---|---:|---|---|
| greeting_empty | 0 | 来啦，今天打算钓什么？ | 钩线都在这，自己挑。 |
| greeting_fish | 1 | 桶放这儿，我给你过秤。 | 嚯，今天没白跑。 |
| greeting_giant | 3 | 好家伙！先别往桌上搁。 | 等我腾个地方，这条够沉。 |
| sold | 1 | 秤看清了，钱拿好。 | 行，这一桶我都收了。 |
| bought | 1 | 给你装好了，拿稳。 | 线头我给你理过了。 |
| poor | 2 | 手头不够？下回来拿。 | 先别急，家伙还能用。 |
| empty | 0 | 空桶就先搁旁边吧。 | 没鱼不要紧，聊两句。 |
| owned | 0 | 这件你有啦，别买重了。 | 手里那套还没用坏呢。 |
| full | 2 | 先理理箱子，装不下了。 | 塞太满，回头找东西费劲。 |
| rare_catfish | 3 | 这鲶鱼黄得晃眼啊。 | 留着，我认识人专收这个。 |
| rare_hooksnout | 3 | 这马口的鳍，可真够长。 | 单独养着，别把尾巴碰坏。 |
| rare_carp | 3 | 红的长尾鲤？你手气行啊。 | 这条我按稀罕货收。 |
| rare_softshell | 3 | 白鳖？我得凑近瞧瞧。 | 嘴还挺凶，手别伸过去。 |
| rare_hint | 0 | 钓着模样特别的，带来看看。 | 有人就爱稀奇鱼，舍得出钱。 |
| 场景ID | 表情 | 台词1 | 台词2 |
|---|---:|---|---|
| relief | 1 | 这副细线算我的，先用着。 | 这副细线算我的，先用着。 |
| oarfish | 3 | 嚯，这条够摆半条街了。 | 我得找个人来帮着抬。 |
| peacock_mantis | 3 | 这只单独放，我仔细瞧瞧。 | 这配色，跟彩绘似的。 |
