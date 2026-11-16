# D26R1 猫咪跟随与船上休息 / Cat companion and boat rest

2026-11-02，MKLink V4。沿用D26全部内容、V21的2147B存档及完整2026.10.31.4格式3资源；已装该资源时只需更新固件。

都市便利店外流浪猫的每小时候选出现概率由35%提高到65%。仍为12:00～24:00、每小时的前30分钟；读档不重抽，玩家仍在附近时保持有猫／没猫状态，不会突然刷新或消失。好感50收养、每天最多喂两次和每次见面三次互动保留。

收养由全局好感保存，不属于某个地图。猫随乘车到湿地木船、城市气垫筏、海滩快艇；上岸后留在船上坐着或睡觉。刚收养时先保留岸边的猫，离开视野后上船，避免当面消失。三图共用同一份好感、罐头及每日喂食次数；换地图、加载失败或读档不会丢失猫。

船上背包底部按“图鉴、声音、休息、存档、返回”排列；岸上不显示休息，原四项顺序保留。旋转选中“休息”后持续按住，600ms后以20倍速度推进游戏时间，松开立刻恢复正常；按住时旋转离开休息也停止，松开不会误触新选项。短按休息不抛竿、不更换装备。

例如长按进入加速后，现实1秒推进20个游戏分钟。船保持停住，鱼种和鱼获不重抽；日期、昼夜、天气、潮湿、拿铁／泡面时效、猫的每日次数和04:00赶海刷新按同一游戏时间推进。休息结束请求存档；只存推进后的时间等状态，不保存正在长按的状态，重启不会自行继续加速。

主机核验涵盖96日的1152个候选小时、六条有向跨图路线、失败加载、各目的地存读档，以及三图休息阈值、松开、旋转取消、短按、岸上排除、跨午夜、增益／天气同步、32位毫秒回绕。渲染检查包括海图快艇猫及三图休息菜单。实际设备仍需验收。

D26R1 raises hourly city-cat visit candidates from35% to65%, retaining the noon-to-midnight half-hour windows, deterministic saves and visible-state lock. Adoption is global across all three maps; friendship, food and the daily feed limit survive travel, failed loading and saves. Cats sit or sleep on the wooden boat, raft and speedboat.

The boat backpack adds Rest between Sound and Save. Select it and hold for600ms to advance game time at20x; release or rotate away to stop. Shore menus omit Rest. Weather, date, wetness, refreshment timers and daily resets advance together while the boat stays still. Saves retain the resulting world state, never an active held button. V21 and2026.10.31.4 resources are unchanged; hardware acceptance remains pending.

## 验证结果 / Results

127／127项主机检查通过，96日1152个候选小时中有730次出现；六条有向路线及目的地读档、失败加载、跨图共享每日次数通过。休息的精确阈值、松开、旋转取消、短按、岸上排除、跨午夜、天气／潮湿／增益同步、32位时钟回绕通过。84帧渲染含三种船的猫和三图休息菜单。存档与资源布局不变。

实板帧率、按键手感、堆栈峰值与掉电仍需设备验收；未操作实体SD或烧录。
