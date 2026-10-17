# MKLink V4 蜂鸣器反馈 v27 / Buzzer feedback v27

复用工程的 `common/beep_control` 驱动，使用 PA10 / GPTMR0 通道2。音效只在游戏线程的状态更新和输入处理之后触发，不在PFB绘图回调中操作硬件；驱动的单次RT-Thread定时器自动停声，无延时等待、独立音频线程或音频素材。

The existing `common/beep_control` driver uses PA10 / GPTMR0 channel 2. Cues run after game updates and input handling in the game thread, outside PFB drawing. A one-shot RT-Thread timer stops each tone automatically, without delay loops, a dedicated audio thread or audio assets.

| 情境 / Event | 频率 / Frequency | 时长 / Duration | 重复 / Repeat |
|---|---|---|---|
| 成功提竿上钩 / Successful hook | 160 Hz | 60 ms | 一次 / Once |
| 鱼实际挣扎 / Active fish burst | 随机110–170 Hz / Random 110–170 Hz | 随机20/30/40/50 ms / Random 20/30/40/50 ms | 挣扎开始立即播放，随机静音20–70 ms后再播；起音间距40–120 ms / Immediate on burst entry, then a random 20–70 ms silence; onset interval 40–120 ms |
| 成功购买或出售 / Successful purchase or sale | 660 → 1100 Hz | 40 ms，间隔10 ms，尾音80 ms / 40 ms, 10 ms gap, 80 ms tail | 每笔成功交易一次 / Once per successful trade |

所有音符占空比暂定20%。咬钩试饵阶段安静，只有成功提竿才播放上钩声；体力恢复、平静期和被动鱼种不伪造挣扎脉冲。退出搏鱼或商店立即停声，重新初始化也会停声。重复帧不会重播上钩声，重复交易会重新触发收银声；金币实际变化配合交易成功状态判断，钱不足、已有装备、饵料库存满、鱼库空或金币达上限时不播放。

All notes initially use 20% duty. Probing bites stay silent; only a successful strike emits the hook cue. Recovery, lulls and passive species do not fabricate burst pulses. Leaving a fight/shop or reinitializing stops playback. Repeated frames cannot replay the hook cue; repeated successful trades restart the cashier cue. Coin changes must accompany a successful trade result, so insufficient funds, owned gear, full bait stock, empty cargo and a full coin purse remain silent.

参数集中在 `src/fish/fish_hpm_feedback.c`。蜂鸣器接口的时长单位为10 ms，传入0频率/占空比表示停止；所有可听音符都传入有限时长。音符切换依赖游戏线程后续更新，不补播错过的挣扎脉冲，避免卡顿后集中响声。

Parameters are grouped in `src/fish/fish_hpm_feedback.c`. Driver durations use 10 ms units; zero frequency/duty stops output, and every audible note has a finite duration. Note transitions follow subsequent game-thread updates; missed burst pulses are not queued or replayed after stalls.

主机测试覆盖真实提竿和交易入口、重复帧、挣扎间隔、体力恢复、被动种、失败交易、重复交易、退出停止及毫秒计数回绕。PWM频率、真实声音和机壳震感尚未在实板试听；这版作为短促低频的初始参数。

Host checks cover real strike/trade entry points, repeated frames, burst cadence, recovery, passive species, failed/repeated trades, exit cancellation and millisecond wrap. Physical PWM output, sound and enclosure vibration have not been measured or auditioned on hardware; these are initial settings for short low-frequency feedback.

## 声音档位 v25 / Saved sound modes v25

行囊“声音”提供关闭、仅鱼讯、全部开启，默认全部开启。短按编辑、旋转选择、短按确认，选择立即生效并在确认时请求保存。仅鱼讯保留上钩/挣扎，关闭收银音；关闭档位停止全部声音，包括正在播放的尾音。档位写入1317字节v7主存档（声音仍在偏移260），不另建配置文件；旧v1–v5迁移默认全部开启。

The bag sound control offers off, fish-only and all (default). Tap to edit, rotate to select, and tap to confirm/save. Changes apply immediately. Fish-only retains hook/burst cues and disables cashier notes; off cancels all output, including pending tails. The mode is stored in the authenticated 1317-byte v7 save (sound stays at offset 260), with no separate settings file; v1–v5 migration defaults to all sounds.

## 不规则短震 v27 / Irregular pulse texture v27

用独立伪随机状态选择每一下的低频、时长和静音间隙，替代每240 ms固定一次的节拍。当前20%占空比，频率110–170 Hz、响20–50 ms、停20–70 ms，起音间隔40–120 ms，实际会受到游戏线程更新频率的量化和延迟。每次上钩根据毫秒时间与已有状态初始化声音随机数，随后只推进声音自身状态，不修改钓鱼、战斗或运动随机数，也不写入存档。

只在实际冲刺挣扎时持续；平静、体力恢复、被动鱼种和退出搏鱼立即停止。上钩与收银音沿用原参数；声音三档继续有效。一次更新最多发一声，重复时间戳不会连发，卡顿不补播。32次不同上钩时刻验证节奏变化，另检查16 ms更新、卡顿、毫秒回绕、静音切换和完整游戏状态不被音效修改。

A private pseudorandom state varies each low-frequency pulse, its duration and its following silence. At 20% duty, pulses use 110–170 Hz for 20–50 ms followed by 20–70 ms of silence, giving 40–120 ms onset intervals before game-thread quantization/delay. Sound has its own state seeded on hooking, never advancing fishing, fight or motion randomness and never entering the save. Only actual bursts produce repeated cues; lulls, recovery, passive fish and fight exits stop them. Hook/cashier parameters and all three sound settings remain. Repeated callbacks cannot duplicate a pulse, and stalls never queue catch-up sounds. Host checks cover 32 different hook times, 16 ms updates, stalls, clock wrap, muting and an unchanged game-state snapshot.

本版没有实板试听或机壳震感测量，最终听感需用新版固件确认。

Hardware sound and enclosure vibration have not been auditioned or measured for this version.
