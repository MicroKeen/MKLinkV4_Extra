# MKLink V4 蜂鸣器反馈 v24 / Buzzer feedback v24

复用工程的 `common/beep_control` 驱动，使用 PA10 / GPTMR0 通道2。音效只在游戏线程的状态更新和输入处理之后触发，不在PFB绘图回调中操作硬件；驱动的单次RT-Thread定时器自动停声，无延时等待、独立音频线程或音频素材。

The existing `common/beep_control` driver uses PA10 / GPTMR0 channel 2. Cues run after game updates and input handling in the game thread, outside PFB drawing. A one-shot RT-Thread timer stops each tone automatically, without delay loops, a dedicated audio thread or audio assets.

| 情境 / Event | 频率 / Frequency | 时长 / Duration | 重复 / Repeat |
|---|---|---|---|
| 成功提竿上钩 / Successful hook | 160 Hz | 60 ms | 一次 / Once |
| 鱼实际挣扎 / Active fish burst | 140 Hz | 30 ms | 挣扎开始立即播放，持续时每240 ms一次 / Immediately on burst entry, then every 240 ms |
| 成功购买或出售 / Successful purchase or sale | 660 → 1100 Hz | 40 ms，间隔10 ms，尾音80 ms / 40 ms, 10 ms gap, 80 ms tail | 每笔成功交易一次 / Once per successful trade |

所有音符占空比暂定20%。咬钩试饵阶段安静，只有成功提竿才播放上钩声；体力恢复、平静期和被动鱼种不伪造挣扎脉冲。退出搏鱼或商店立即停声，重新初始化也会停声。重复帧不会重播上钩声，重复交易会重新触发收银声；金币实际变化配合交易成功状态判断，钱不足、已有装备、饵料库存满、鱼库空或金币达上限时不播放。

All notes initially use 20% duty. Probing bites stay silent; only a successful strike emits the hook cue. Recovery, lulls and passive species do not fabricate burst pulses. Leaving a fight/shop or reinitializing stops playback. Repeated frames cannot replay the hook cue; repeated successful trades restart the cashier cue. Coin changes must accompany a successful trade result, so insufficient funds, owned gear, full bait stock, empty cargo and a full coin purse remain silent.

参数集中在 `src/fish/fish_hpm_feedback.c`。蜂鸣器接口的时长单位为10 ms，传入0频率/占空比表示停止；所有可听音符都传入有限时长。音符切换依赖游戏线程后续更新，不补播错过的挣扎脉冲，避免卡顿后集中响声。

Parameters are grouped in `src/fish/fish_hpm_feedback.c`. Driver durations use 10 ms units; zero frequency/duty stops output, and every audible note has a finite duration. Note transitions follow subsequent game-thread updates; missed burst pulses are not queued or replayed after stalls.

主机测试覆盖真实提竿和交易入口、重复帧、挣扎间隔、体力恢复、被动种、失败交易、重复交易、退出停止及毫秒计数回绕。PWM频率、真实声音和机壳震感尚未在实板试听；这版作为短促低频的初始参数。

Host checks cover real strike/trade entry points, repeated frames, burst cadence, recovery, passive species, failed/repeated trades, exit cancellation and millisecond wrap. Physical PWM output, sound and enclosure vibration have not been measured or auditioned on hardware; these are initial settings for short low-frequency feedback.

## 声音档位 v25 / Saved sound modes v25

行囊“声音”提供关闭、仅鱼讯、全部开启，默认全部开启。短按编辑、旋转选择、短按确认，选择立即生效并在确认时请求保存。仅鱼讯保留上钩/挣扎，关闭收银音；关闭档位停止全部声音，包括正在播放的尾音。档位写入293字节v6主存档，不另建配置文件；旧v1–v5迁移默认全部开启。

The bag sound control offers off, fish-only and all (default). Tap to edit, rotate to select, and tap to confirm/save. Changes apply immediately. Fish-only retains hook/burst cues and disables cashier notes; off cancels all output, including pending tails. The mode is stored in the authenticated 293-byte v6 save, with no separate settings file; v1–v5 migration defaults to all sounds.
