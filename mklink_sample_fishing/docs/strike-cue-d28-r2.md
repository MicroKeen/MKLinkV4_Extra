# D28R2 仅鱼讯提醒 / Strike-window-only sound mode

MKLink V4，2026-11-02。声音“仅鱼讯”现在只在进入实际可提竿窗口时发出一次短提醒，不播放上钩成功声、挣扎脉冲或收银音。提醒沿用160Hz／60ms／20%占空比的短促低频。此前该档保留上钩／挣扎，且没有提前的窗口提醒，已按新规则替换。

“全部开启”包含窗口提醒，并保留上钩成功声、不规则挣扎反馈和购买／出售收银音。“关闭”保持完全静音，切换时停止当前音符。设置的数字值、确认存档行为和V22格式保持不变，旧存档中的“仅鱼讯”直接采用新语义。

提醒复用游戏的实际可提竿判断，与漂的加成、拿铁、线组和鱼种延迟一致。试饵和过早提竿不响，已过有效窗口不补播；帧刷新与输入回调不重复提醒，每一口新鱼可再次提醒。不读取鱼种名称给玩家提示，也不推进游戏、尺寸或运动随机数。

主机检查覆盖40个可钓编号、三种声音档位和普通／回绕时间戳共240组窗口边界，重复回调、新一口鱼、窗口过期不补播、漂／拿铁扩展窗口，以及真实上钩、挣扎、购买和出售入口。既有不规则脉冲、卡顿、随机数隔离和退出停止继续检查。实板听感待验。

Fish-only now emits one160Hz/60ms/20%-duty cue when the real strike window becomes available. Hook-success, struggle and cashier sounds are disabled in that mode. All sounds includes the new reminder and retains existing fight/trade feedback; Off stays silent. V22 saves, setting values and2026.10.31.4 resource packs are unchanged.

The cue follows actual species/tackle/float/latte timing, never repeats on duplicate callbacks and is not replayed after an expired window. Tests cover40 catchable IDs, three modes and clock wrap, plus real hook/trade paths and unchanged game state. Hardware sound acceptance remains pending.

## 构建核验 / Build evidence

131项主机检查通过（93＋38），GNU MCU固件和公共素材检查通过。应用Flash余9392B，比D28R1增加128B；DLM余1776B，AHB余480B。静态RAM总占用不变，无新位图。实板听感待验。

All131 host checks pass. MCU build/runtime-asset checks pass. Application Flash retains9392B and static RAM totals are unchanged. Hardware sound acceptance remains pending.

UF2 SHA256: `e17049f9dd7a04f3dc58819738b8bea5b9a336192ccde6d22272f6e80f425aed`
