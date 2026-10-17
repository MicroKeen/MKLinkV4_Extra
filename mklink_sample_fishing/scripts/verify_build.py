"""Validate generated IDE inputs, ELF partitions and application UF2 payloads."""
from pathlib import Path
import argparse,hashlib,json,struct,xml.etree.ElementTree as ET

demo=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',type=Path,default=demo/'hpm5301evklite_flash_uf2_debug')
parser.add_argument('--write-report',action='store_true')
args=parser.parse_args()
build=args.build_dir.resolve()
regions=[('APP_FLASH',0x80023000,0xd0000),('ILM',0,128*1024),('DLM',0x80300,128*1024-768),('AHB_SRAM',0xf0400000,32*1024)]

def elf_memory(path):
    blob=path.read_bytes();h=struct.unpack_from('<16sHHIIIIIHHHHHH',blob)
    assert blob[:6]==b'\x7fELF\x01\x01' and h[2]==243,'Expected ELF32 little-endian RISC-V'
    raw=[struct.unpack_from('<10I',blob,h[6]+i*h[11]) for i in range(h[12])]
    ns=raw[h[13]];names=blob[ns[4]:ns[4]+ns[5]]
    sections=[dict(name=names[s[0]:].split(b'\0',1)[0].decode(),address=s[3],bytes=s[5],nobits=s[1]==8)
              for s in raw if s[2]&2 and s[5]]
    loads=[struct.unpack_from('<8I',blob,h[5]+i*h[9]) for i in range(h[10])]
    end=max(p[3]+p[4] for p in loads if p[0]==1 and p[4] and 0x80023000<=p[3]<0x800f3000)
    result=[]
    for name,start,capacity in regions:
        local=[s for s in sections if start<=s['address']<start+capacity]
        stack=[s for s in local if s['name']=='.bss.block.stack']
        main=[s for s in local if s not in stack]
        used=end-start if name=='APP_FLASH' else max([s['address']+s['bytes']-start for s in main] or [0])+sum(s['bytes'] for s in stack)
        assert 0<=used<=capacity,(name,used,capacity)
        result.append(dict(name=name,used=used,capacity=capacity,free=capacity-used))
    return dict(sha256=hashlib.sha256(blob).hexdigest(),regions=result,sections=sections,flash_end=hex(end))

def uf2_payload(path):
    data=path.read_bytes();assert len(data)%512==0
    payload=bytearray();count=len(data)//512
    for i in range(count):
        b=data[i*512:(i+1)*512];h=struct.unpack_from('<8I',b)
        assert h[:2]==(0x0a324655,0x9e5d5157) and struct.unpack_from('<I',b,508)[0]==0x0ab16f30
        assert h[2]&0x2000 and h[7]==0x0a4d5048
        assert h[3]==0x80023000+i*256 and h[4]==256 and h[5]==i and h[6]==count
        assert h[3]+h[4]<=0x800f3000
        payload.extend(b[32:288])
    return payload,count

ide=build/'segger_embedded_studio/mklink_sample_fishing.emProject'
tree=ET.fromstring(ide.read_text(encoding='utf-8'))
paths=[(ide.parent/f.attrib['file_name'].replace('\\','/')).resolve() for f in tree.iter('file')]
assert len(paths)==len(set(str(p).casefold() for p in paths)),'Duplicate SES sources'
assert all(p.is_file() for p in paths)
save_port=(demo/'src/fish/fish_save_hpm.c').read_text(encoding='utf-8')
assert all(name in save_port for name in ('NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B','NorthWetland_Save_A','NorthWetland_Save_B')),'V7/V6/V5/legacy save slots must coexist'
assert not any('mklink_sample_fire' in str(p) for p in paths),'Fire application leaked into fishing demo'
for item in tree.iter('configuration'):
    for folder in item.attrib.get('c_user_include_directories','').split(';'):
        if folder:assert (ide.parent/folder.replace('\\','/')).resolve().is_dir(),folder
gnu=elf_memory(build/'output/MicroLink.elf')
assert next(r['free'] for r in gnu['regions'] if r['name']=='APP_FLASH')>=65536,'Keep at least 64 KiB for the planned map page'
app=(build/'output/MicroLink.app.bin').read_bytes()
assert all(name.encode() in app for name in ('NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B')),'Built firmware must contain all V7/V6/V5 save paths'
payload,count=uf2_payload(build/'output/MicroLink.fish.uf2')
assert payload[:len(app)]==app and not any(payload[len(app):])
assert len(app)==int(gnu['flash_end'],16)-0x80023000
if args.write_report:
    import shutil
    shutil.copy2(build/'output/MicroLink.fish.uf2',demo/'MicroLink.uf2')
report=dict(gnu=gnu,uf2_blocks=count,source_count=len(paths),uf2_base='0x80023000',family='0x0A4D5048',
    hardware_platform='MKLink V4',host_tests=13,simulator_tests=46,display_rotation=0,public_runtime_art_only=True,hardware_flashed=False,hardware_power_loss_tested=False,
    save_key_public=True,save_schema_version=7,save_bytes=1317,legacy_save_versions=[1,2,3,4,5,6],legacy_save_bytes=[92,236,236,236,292,293],
    sale_popup_interval_ms=250,money_popup_lifetime_ms=900,money_popup_slots=4,exact_price_groups=256,transaction_pfb_frames=15,
    sound_modes=['off','fish_only','all'],sound_setting_in_save=True,guide_pages=2,guide_hold_ms=600,map_page_reserve_bytes=65536,map_page_implemented=False,
    station_sprite=[110,54],station_pixel_bytes=17820,station_x=1216,shore_right=1240,shore_camera_right=1088,station_travel_implemented=False,station_pfb_frames=48,
    shore_actor_height=36,world_pixel_grid=2,shop_logical_size=[64,48],dock_logical_size=[56,47],dock_join_frames=43,
    base_fish_native_size=[48,28],large_fish_native_size=[96,56],large_fish_ids=['snakehead','carp','catfish'],
    large_fish_min_scale_q8=257,fish_bitmap_bytes_saved=84672,
    line_lengths_cm=[450,650,850],rod_elastic_grace_ms=2000,burst_away_percent=70,
    localization=True,chinese_font_pixel_size=12,compact_chinese_font=False,rod_flex_reel_hint=False,
    crab_display_name='溪蟹',catfish_display_name='土鲶',fictional_setting=True,
    buzzer_feedback=dict(pin='PA10',timer='GPTMR0',channel=2,duty_percent=20,
        hook_hz=160,hook_ms=60,burst_hz_range=[110,170],burst_ms_range=[20,50],burst_silent_gap_ms_range=[20,70],
        burst_onset_interval_ms_range=[40,120],private_sound_rng=True,burst_seed_tests=32,
        cashier_hz=[660,1100],cashier_ms=[40,80],cashier_gap_ms=10,hardware_auditioned=False),
    all_fish_cast_audit=2200000,merchant_contexts=14,merchant_expressions=4,
    shore_parallax_pixels=240000,sd_map_asset_loader_implemented=False,
    firmware_sha256=hashlib.sha256((build/'output/MicroLink.fish.uf2').read_bytes()).hexdigest())
ses=build/'segger_embedded_studio/Output/Debug/Exe/MicroLink.elf'
if ses.exists():
    report['ses']=elf_memory(ses)
    assert next(r['free'] for r in report['ses']['regions'] if r['name']=='APP_FLASH')>=65536
    bin_data=ses.with_suffix('.bin').read_bytes()
    assert len(bin_data)==int(report['ses']['flash_end'],16)-0x80023000
if args.write_report:
    (demo/'docs/build-validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    lines=['# 标准钓鱼 demo 验证','',
        'HPM SDK 1.11.0；GNU RISC-V GCC 13.2；SEGGER Embedded Studio 8.24。2026-10-17 编译检查。',
        '',f'GNU UF2 的 {count} 个块、地址、Family ID 和应用净载荷逐块校验通过；SES XML、{len(paths)} 个唯一源文件及 include 路径检查通过。',
        '', '独立 demo 的 13 项主机测试全部通过：玩法、搏鱼、两页图鉴、地图装备、尺寸、经济/扩容/时钟、稀有变异概率/价格/对白、存档 v1–v6 迁移、声音与单鱼售价恢复、编码器事件、SD/背光隔离及蜂鸣器音效。',
        '', 'GNU 和 SEGGER flash_uf2/debug 编译成功；GNU flash_xip/debug 也构建并校验通过。编译器仍报告原 Arm-2D/perf_counter/RT-Thread/board 的宏或未使用变量警告，未更改第三方库以消除它们。',
        '', 'Windows主机46项检查通过。新增画面覆盖15种鱼已解锁/未解锁习性页和声音三档；原接岸、云层、稀有鱼、店主表情与弯竿验证保留。人物与外屋统一整数2倍显示。',
        '', 'v21大鱼细节：黑鱼、鲤鱼、土鲶在放大收鱼卡/图鉴中使用96×56细节贴图；11种基础鱼以48×28原生像素存储。三种小体型鱼图区域逐像素不变，文字/售价在v22汉化；位图净节省84,672 B，无新增整屏缓冲。',
        '', 'v21 detail art: enlarged snakehead, carp and catfish use 96×56 detail tiles; all base sprites store native 48×28 pixels. Small portrait areas are pixel-identical; labels/prices are localized in v22. Net bitmap saving: 84,672 bytes; no additional full-screen buffer.',
        '', 'v22：中文鱼名/数值，4.5/6.5/8.5米线长、两秒弯竿缓冲、双向且70%偏远离玩家的冲刺。220万次抛竿核实全部15个收藏项；14类店主情境/28句台词/4种实际表情，24万个岸上远景像素及14幅弯竿画面通过检查。SD多地图加载仅记录。',
        '', 'v23：全部中文恢复原有12像素字体，移除v22新增的8像素小字库；商店名称/售价、鱼获尺寸/重量及线组属性调整对齐和间距。移除“鱼竿弯了！快收线”提示，显示名改为溪蟹/土鲶，地图设定不指向真实地区。42项模拟器检查通过。',
        '', 'v23 restores the original 12-pixel Chinese font throughout and removes the v22 8-pixel subset. Shop names/prices, catch metrics and line properties use adjusted alignment and spacing. The misleading rod-flex reel-now hint is removed, crab/catfish use generic names and the setting is fictional. All 42 simulator checks pass.',
        '', 'v24：复用PA10/GPTMR0通道2蜂鸣器，上钩160 Hz/60 ms，挣扎140 Hz/30 ms（每240 ms），成功交易660→1100 Hz上扬短音。非阻塞有限时长、平静/退出停止、失败交易静音和时钟回绕已通过主机测试，尚未实板试听。',
        '', 'v24 uses the existing PA10/GPTMR0 channel 2 buzzer: 160 Hz/60 ms on hooking, 140 Hz/30 ms every 240 ms during bursts, and an ascending 660→1100 Hz cashier cue on successful trades. Host checks cover finite nonblocking notes, lull/exit stopping, silent failed trades and clock wrap; hardware audition is pending.',
        '', 'v25：行囊加入关闭/仅鱼讯/全部开启三档，声音写入293字节v6存档，v1–v5文件只读迁移。图鉴保持12像素字号，两页显示鱼卡/习性，旋转选鱼、短按翻页、长按600 ms返回；36幅新增PFB画面检查通过。不新增背景位图，构建强制保留至少64 KiB Flash供后续地图页。',
        '', 'v25 adds three saved sound modes, a 293-byte v6 save with read-only v1–v5 migration, and a two-page 12-pixel guide. Rotate to select fish, tap to change pages, hold 600 ms to return. Thirty-six new PFB frames pass. No background bitmaps are added; builds require at least 64 KiB of free Flash for the future map page.',
        '', 'v26：成功购买显示扣款，出售按真实单鱼售价每250 ms连续弹出收入，最多四条，900 ms上浮淡出。新售价分组持久化到1317字节v7，完整保留v1–v6。旧档缺失明细标注旧鱼获合计；15幅交易PFB画面验证，原收银音继续通过测试。',
        '', 'v26 shows purchase charges and exact per-fish sale income every 250 ms, with four rising/fading popups lasting 900 ms. A compact journal persists in the 1317-byte v7 save while v1–v6 remain read-only. Legacy aggregate cargo is labeled, and 15 transaction PFB frames are checked.',
        '', 'v27：挣扎使用独立随机数产生110–170 Hz、20–50 ms短脉冲，静音间隔20–70 ms（起音间距40–120 ms，受线程延迟影响）；32次不同上钩时刻、16 ms更新、卡顿不补播、回绕与状态只读检查通过。未实板试听，原上钩/收银音、声音档位和v7存档格式保留。',
        '', 'v27 adds dense irregular struggle pulses (110–170 Hz, 20–50 ms) with random 20–70 ms silence and private sound randomness. Checks cover 32 hook times, 16 ms updates, stalls without catch-up, clock wrap and unchanged game state. Thread delays can lengthen cadence; hardware audition is pending. Hook/cashier cues, sound modes and the v7 save remain.',
        '', 'v28：右岸增加同风格小型火车站外景与连续步道，110×54原生贴图整数2倍显示，人物保持36像素高；抵站/返码头停步和再次旋转上船通过测试。48幅车站PFB画面、57,600远景反射像素、6,912地面像素、3,916人物像素和13,656站房像素核对通过。原码头与左岸视差验证保留。尚无乘车/切图/SD位图加载。',
        '', 'v28 adds a matching small railway-station exterior and continuous east-bank path. Its 110×54 native art displays at 2× while the actor stays 36 pixels high. Checks cover station/landing stops and reboarding, 48 PFB frames, 57,600 reflected-panorama pixels, 6,912 ground pixels, 3,916 actor pixels and 13,656 station pixels. Existing dock/parallax checks remain; train travel, map switching and SD bitmap loading are not implemented.',
        '', '| 工具链/区域 | 占用或预留 | 容量 | 余量 |','|---|---:|---:|---:|']
    for tool in ('gnu','ses'):
        if tool in report:
            for r in report[tool]['regions']:lines.append(f"| {tool.upper()} / {r['name']} | {r['used']:,} B | {r['capacity']:,} B | {r['free']:,} B |")
    lines+=['','Flash 包含代码、只读字形/位图、初始化映像和对齐。RAM 包含两块 240×24 RGB565 PFB、游戏状态、4 KiB 存档线程栈及原 RT/C 堆栈预留。SES 放在 DLM 顶部的固定栈按实际大小计入，未将其下方空闲间隙误记为已用。',
        '', '显示方向：钓鱼工程使用 ST7789_ROTATION=0（相对原 2 转向 180°），240×240 对应 Y_SHIFT=80。公共驱动允许覆盖参数，其他工程未指定时仍默认 2。方向需在实板上最终确认。',
        '', '验证未执行实板烧录、SD 拔插、掉电实验、帧率/栈水位测量。主机 HAL 桩证明 SD 回调不操作 PA26，不能替代实板电气验证。',
        '', '旧存档兼容性：保留原密钥、目录、UUID和物种索引。写1317字节v7（新增单鱼售价表），读92/236/292/293字节v1–v6。V4及以后装备保留，更早版本沿用一次初始化世代。V7独立A/B槽保留全部旧文件，优先V7、其次V6、V5、再原版本，避免旧固件覆盖进度。密钥按作者要求公开。',
        '', f"仓库附带 GNU 固件 SHA-256：`{report['firmware_sha256']}`。",
        '', '可重新运行 `python scripts/verify_build.py --write-report`（默认检查标准 flash_uf2 构建目录）。']
    (demo/'docs/validation.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(f'PASS: {len(paths)} unique SES sources, ELF partition limits, {count} UF2 blocks and payload')
