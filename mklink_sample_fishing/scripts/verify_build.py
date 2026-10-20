"""Validate generated IDE inputs, ELF partitions and application UF2 payloads."""
from pathlib import Path
import argparse,hashlib,json,struct,xml.etree.ElementTree as ET

demo=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',type=Path,default=demo/'hpm5301evklite_flash_uf2_debug')
parser.add_argument('--write-report',action='store_true')
parser.add_argument('--gnu-only',action='store_true',help='Do not treat historical SES artifacts as a current build')
args=parser.parse_args()
build=args.build_dir.resolve()
regions=[('APP_FLASH',0x80023000,0x5d000),('ILM',0,128*1024),('DLM',0x80300,128*1024-768),('AHB_SRAM',0xf0400000,32*1024)]

def elf_memory(path):
    blob=path.read_bytes();h=struct.unpack_from('<16sHHIIIIIHHHHHH',blob)
    assert blob[:6]==b'\x7fELF\x01\x01' and h[2]==243,'Expected ELF32 little-endian RISC-V'
    raw=[struct.unpack_from('<10I',blob,h[6]+i*h[11]) for i in range(h[12])]
    ns=raw[h[13]];names=blob[ns[4]:ns[4]+ns[5]]
    sections=[dict(name=names[s[0]:].split(b'\0',1)[0].decode(),address=s[3],bytes=s[5],nobits=s[1]==8)
              for s in raw if s[2]&2 and s[5]]
    loads=[struct.unpack_from('<8I',blob,h[5]+i*h[9]) for i in range(h[10])]
    end=max(p[3]+p[4] for p in loads if p[0]==1 and p[4] and 0x80023000<=p[3]<0x80080000)
    result=[]
    for name,start,capacity in regions:
        local=[s for s in sections if start<=s['address']<start+capacity]
        stack=[s for s in local if s['name']=='.bss.block.stack']
        main=[s for s in local if s not in stack]
        used=end-start if name=='APP_FLASH' else max([s['address']+s['bytes']-start for s in main] or [0])+sum(s['bytes'] for s in stack)
        assert 0<=used<=capacity,(name,used,capacity)
        result.append(dict(name=name,used=used,capacity=capacity,free=capacity-used))
    critical=[]
    usb_entrypoints=[]
    watchdog_runtime=[]
    fault_capture=[]
    sd_guard_present=False
    for sy in raw:
        if sy[1]!=2:continue
        strings=raw[sy[6]];strings=blob[strings[4]:strings[4]+strings[5]]
        for at in range(sy[4],sy[4]+sy[5],16):
            name,value,size,info,other,index=struct.unpack_from('<IIIBBH',blob,at)
            name=strings[name:].split(b'\0',1)[0].decode(errors='replace')
            if index and (name in ('usbd_initialize','usbd_msc_sector_write','usbd_msc_sector_read') or name.startswith('usbd_msc_init')):
                usb_entrypoints.append(name)
            if index and name=='fish_sd_guard_capacity':sd_guard_present=True
            if index and name in ('fish_hpm_capture_reset','fish_hpm_storage_heartbeat','fish_hpm_watchdog_reset'):
                watchdog_runtime.append(name)
            if index and name in ('fish_hpm_record_fault','exception_handler'):
                assert value+size<=128*1024 and info>>4!=2,'CPU fault capture must be strong and ILM resident'
                fault_capture.append(dict(name=name,address=hex(value),bytes=size))
            if name.startswith(('cache_write_critical','cache_configure_critical')):
                assert value+size<=128*1024,'NOR configuration and writes must execute entirely in ILM'
                section=raw[index];start=section[4]+value-section[3]
                fence_i=b'\x0f\x10\x00\x00' in blob[start:start+size]
                assert fence_i,'Synchronise instructions before leaving NOR critical sections'
                critical.append(dict(name=name,address=hex(value),bytes=size,fence_i=True))
    assert all(any(row['name'].startswith(name) for row in critical) for name in ('cache_write_critical','cache_configure_critical')),'Missing RAM-resident NOR configuration/writer'
    assert all(name in usb_entrypoints for name in ('usbd_initialize','usbd_msc_sector_write','usbd_msc_sector_read')) and sd_guard_present,'Missing guarded USB importer'
    assert len(set(watchdog_runtime))==3,'Missing bounded-storage/watchdog diagnostics'
    assert len(fault_capture)==2,'Missing CPU fault capture override'
    return dict(ram_flash_writer=critical,usb_entrypoints=usb_entrypoints,sd_guard_present=True,
        watchdog_runtime=watchdog_runtime,fault_capture=fault_capture,sha256=hashlib.sha256(blob).hexdigest(),regions=result,sections=sections,flash_end=hex(end))

def uf2_payload(path):
    data=path.read_bytes();assert len(data)%512==0
    payload=bytearray();count=len(data)//512
    for i in range(count):
        b=data[i*512:(i+1)*512];h=struct.unpack_from('<8I',b)
        assert h[:2]==(0x0a324655,0x9e5d5157) and struct.unpack_from('<I',b,508)[0]==0x0ab16f30
        assert h[2]&0x2000 and h[7]==0x0a4d5048
        assert h[3]==0x80023000+i*256 and h[4]==256 and h[5]==i and h[6]==count
        assert h[3]+h[4]<=0x80080000
        payload.extend(b[32:288])
    return payload,count

ide=build/'segger_embedded_studio/mklink_sample_fishing.emProject'
tree=ET.fromstring(ide.read_text(encoding='utf-8'))
paths=[(ide.parent/f.attrib['file_name'].replace('\\','/')).resolve() for f in tree.iter('file')]
assert len(paths)==len(set(str(p).casefold() for p in paths)),'Duplicate SES sources'
assert all(p.is_file() for p in paths)
save_port=(demo/'src/fish/fish_save_hpm.c').read_text(encoding='utf-8')
assert all(name in save_port for name in ('NorthWetland_V13_Save_A','NorthWetland_V13_Save_B','NorthWetland_V12_Save_A','NorthWetland_V12_Save_B','NorthWetland_V11_Save_A','NorthWetland_V11_Save_B','NorthWetland_V10_Save_A','NorthWetland_V10_Save_B','NorthWetland_V9_Save_A','NorthWetland_V9_Save_B','NorthWetland_V8_Save_A','NorthWetland_V8_Save_B','NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B','NorthWetland_Save_A','NorthWetland_Save_B')),'V9 through legacy save slots must coexist'
assert not any('mklink_sample_fire' in str(p) for p in paths),'Fire application leaked into fishing demo'
for item in tree.iter('configuration'):
    for folder in item.attrib.get('c_user_include_directories','').split(';'):
        if folder:assert (ide.parent/folder.replace('\\','/')).resolve().is_dir(),folder
gnu=elf_memory(build/'output/MicroLink.elf')
assert next(r['free'] for r in gnu['regions'] if r['name']=='APP_FLASH')>=8192,'Keep 8 KiB of application headroom'
app=(build/'output/MicroLink.app.bin').read_bytes()
assert all(name.encode() in app for name in ('NorthWetland_V13_Save_A','NorthWetland_V13_Save_B','NorthWetland_V12_Save_A','NorthWetland_V12_Save_B','NorthWetland_V11_Save_A','NorthWetland_V11_Save_B','NorthWetland_V10_Save_A','NorthWetland_V10_Save_B','NorthWetland_V9_Save_A','NorthWetland_V9_Save_B','NorthWetland_V8_Save_A','NorthWetland_V8_Save_B','NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B')),'Built firmware must contain all current and legacy save paths'
payload,count=uf2_payload(build/'output/MicroLink.fish.uf2')
assert payload[:len(app)]==app and not any(payload[len(app):])
assert len(app)==int(gnu['flash_end'],16)-0x80023000
if args.write_report:
    import shutil
    shutil.copy2(build/'output/MicroLink.fish.uf2',demo/'MicroLink.uf2')
from verify_map_packs import verify
packs=verify()
icon_symbols={'c_bmpGameIconPalette','c_bmpGameIconIndices'}
assert icon_symbols.isdisjoint({entry['symbol'] for entry in packs['entries']}),'Boot icon must not depend on an SD pack'
assert all('arm2d.asset.'+symbol in {section['name'] for section in gnu['sections']} for symbol in icon_symbols),'Boot icon must be linked in application ROM'
scene_source=(demo/'src/fish/fish_scene.c').read_text(encoding='utf-8')
frame=scene_source[scene_source.index('static void frame_start('):scene_source.index('static IMPL_PFB_ON_DRAW(draw_scene)')]
loading=frame[frame.index('    if(game->mode==FISH_ROUTE || game->mode==FISH_LOADING)'):frame.index('    self->hint_box=')]
assert 'return;' in loading,'Loading/route early return lost during MCU export'
loading_page=scene_source[scene_source.index('static void loading_page('):scene_source.index('static void frame_start(')]
assert 'game_icon(tile,72,74)' in loading_page
assert all(name not in loading_page for name in ('shore_player_operation','scaled_species_bitmap','line(tile,'))
report=dict(gnu=gnu,uf2_blocks=count,source_count=len(paths),uf2_base='0x80023000',family='0x0A4D5048',
    build_date='2026-10-20',
    resource_version=packs['resource_version'],resource_version_id=packs['resource_version_id'],resource_pack_format=packs['pack_format'],
    conditional_usb_recovery=True,original_usb_test_build_withdrawn=True,normal_boot_usb_hidden=True,usb_msc_buffer_bytes=1024,
    sdk_normalized_sector_count=True,existing_fs_mount_required=True,mbr_bpb_bounds_checked=True,
    copy_received_bytes_displayed=True,copy_progress_percentage_fabricated=False,crc_probe_progress_linear=True,
    missing_file_and_version_diagnostics=True,watchdog_reset_flag_displayed=True,
    game_icon_rom_resident=True,game_icon_size=[48,40],game_icon_bitmap_bytes=525,
    game_icon_codec='row_rle_5bpp',game_icon_pixels_unchanged=True,
    game_icon_index_bits=5,game_icon_display_scale=2,game_icon_in_sd_pack=False,
    sd_transaction_budget_ms=2000,sd_spi_call_timeout_ms=100,watchdog_timeout_us=10000000,
    watchdog_refresh_at_transaction_boundaries=True,watchdog_refresh_inside_response_poll=False,
    watchdog_refresh_keys_irq_atomic=True,usb_and_scene_share_same_clock=True,
    nor_configuration_ilm_resident=True,nor_configuration_masks_interrupts=True,
    nor_configuration_watchdog_boundary_refresh=True,nor_write_watchdog_boundary_refresh=True,
    boot_verification_settle_ms=0,destination_frame_before_nor_initialization=True,
    nor_configuration_after_source_preflight=True,valid_cache_reconfigures_xpi=False,
    nor_configuration_instruction_sync=True,watchdog_reset_stage_best_effort=True,
    cache_crc_bytes_per_tick=16384,source_read_bytes_per_tick=4096,
    watchdog_reset_stage_bytes=28,watchdog_diagnostics_write_sd=False,
    diagnostic_revision='D12',rain_duration_game_hours=[2,48],independent_regional_weather=True,near_rain_max_opacity=224,large_fish_landing_gate=False,line_break_destroys_current_rig=True,line_prices=[120,200,450,600],pe8_line_city_price=600,pe8_line_small_fish_filter=True,free_fine_rig_below_coins=120,free_fine_rig_requires_empty_cargo=False,city_extra_water_offset_px=8,shore_actor_scale=1.8,city_stall_screen_size=[100,76],city_outdoor_monger_behind_counter=True,city_dry_night_dawn_mist=True,rain_and_distant_fog=True,rain_bitmap_bytes=0,wetness_fill_game_minutes=60,wetness_dry_game_minutes=120,wetness_speed_loss_percent=20,cpu_fault_register_capture=True,diagnostic_pause_before_sd=True,
    diagnostic_retry_by_encoder_release=True,hardware_root_cause_confirmed=False,
    exported_scene_loading_early_return_preserved=True,software_root_cause_reproduced=True,
    prior_d3_boot_confirmed_by_user=True,loading_icon='resident_seated_fisherman_boat',
    loading_icon_size=[96,80],loading_icon_position=[72,74],loading_icon_requires_transform=False,
    loading_common_tips=['正在整理钓具','正在前往码头'],loading_tip_interval_ms=7000,
    prior_fault_pc='0x80029f96',prior_fault_cause=5,prior_fault_value='0x008781e6',
    poll_advances_settle_state=True,
    stock_plus_capacity_preserved_when_layout_fits=True,sd_cpu_pointers_preserved=True,sd_cpu_rx_cache_invalidation=False,
    usb_disk_io_in_isr=False,usb_scene_thread_polling=True,usb_detach_after_full_pack_validation=True,
    usb_copy_quiet_ms=3000,usb_final_quiet_ms=1500,usb_waits_for_command_status=True,
    read_only_two_pack_preflight=True,lossless_indexed_shop_in_sd=True,
    hardware_usb_tested=False,ses_rebuilt=not args.gnu_only,
    hardware_platform='MKLink V4',test_counts_not_inferred_from_build=True,display_rotation=0,
    distribution_status='user_approved_publication',original_sd_integrity='unknown',git_push_allowed=True,user_reported_basic_stability=True,agent_hardware_tested=False,
    public_runtime_art_only=True,hardware_flashed=False,hardware_power_loss_tested=False,
    save_manual_io_retry=True,save_preflight_validation=True,save_error_stage_codes=True,city_raft_seated_sprite=True,common_wetland_player_pixels=True,world_foreground_shift_px=8,
    hold_collection_frame=True,hold_container_icon_only=True,night_road_darkening=True,
    sd_writes_single_sector_cmd24=True,sd_write_clock_hz=4000000,sd_read_clock_restored=True,sd_write_crc16=True,sd_delayed_response_poll=True,sd_busy_wait_bounded=True,sd_wire_diagnostics=True,
    large_fish_control_threshold_g=6500,large_fish_landing_stamina_percent=None,giant_additional_force_above_g=20000,
    save_key_public=True,save_schema_version=13,save_exact_position=True,save_clock_fraction_ms=4,save_bytes=1503,legacy_save_versions=[1,2,3,4,5,6,7,8,9,10,11,12],
    legacy_save_bytes=[92,236,236,236,292,293,1317,1318,1470],map_page_implemented=True,station_travel_implemented=True,
    city_fish_implemented=True,city_fish_count=12,new_fish_count=9,collection_count=24,internal_species_slots=25,retired_redtail_slot=21,bighead_slot=24,
    silver_carp_limit_cm_kg=[100,40],bighead_carp_limit_cm_kg=[145,60],
    silver_carp_layer='surface',silver_carp_rarity='bronze',bighead_carp_layers=['middle','lower'],bighead_carp_rarity='silver',
    city_depth_max_cm=1600,city_nearshore_depth_cm=[360,420],old_v9_silver_profile_preserved=True,
    city_independent_profiles=True,cross_map_catch_origin_preserved=True,
    city_shop_implemented=True,city_shop_hours=[9,21],city_tackle_hours=[6,16],wetland_shop_hours=[7,18],
    city_clerk_expressions=5,city_clerk_dialogue_contexts=15,city_monger_dialogue_contexts=15,
    city_atmospheric_panorama=True,weight_kg_above_grams=1000,weight_kg_decimal_places=1,
    hold_portraits_fit_visible_alpha_bounds=True,hold_portrait_area=[160,56],
    moon_before_foreground_buildings=True,city_ground_solid_at_y=136,
    generated_park_lamp_bitmaps=True,independent_soft_light_texture=[36,36],
    park_light_hours=[18,6],park_light_alpha_upper_bound=96,park_light_behind_actor=True,
    sd_map_asset_loader_implemented=True,travel_fare_each_direction=180,travel_minutes_each_direction=300,
    application_end_exclusive='0x80080000',cache_base='0x80080000',cache_end_exclusive='0x800f3000',
    cache_bytes=471040,cache_header_bytes=4096,cache_payload_bytes=packs['payload_bytes'],
    cache_free_bytes=471040-4096-packs['payload_bytes'],external_bitmap_arrays=len(packs['entries']),
    maps=packs['maps'],boot_cache_crc_checked=True,source_preflight_before_erase=True,cache_header_committed_last=True,
    source_and_cache_content_identity_checked=True,readback_verified=True,shared_filesystem_mutex=True,
    loader_staging_bytes=4096,shared_day_night_decode_buffer_bytes=960,panorama_palette_colors=64,loading_flavor_text=True,new_full_framebuffer_bytes=0,route_ui_rom_resident=True,
    virtual_route_size=[640,480],route_viewport=[8,40,224,136],route_pan_ms=360,route_view_state_bytes=16,
    route_thumbnail_size=[64,32],route_selected_alpha=255,route_unselected_alpha=192,
    route_compact_overview=True,route_pan_only_when_network_outgrows_viewport=True,
    route_terrain_symbols=['river','bridge','mountains','forest','pond','reeds','city_blocks'],
    route_terrain_bitmap_bytes=0,
    city_neon_layer=True,city_sweeping_spotlights=True,city_inflatable_raft=True,
    line_lengths_cm=[450,650,850,1050],rod_elastic_grace_ms=2000,burst_away_percent=70,
    chinese_font_pixel_size=12,sound_setting_in_save=True,guide_pages=2,guide_hold_ms=600,
    firmware_sha256=hashlib.sha256((build/'output/MicroLink.fish.uf2').read_bytes()).hexdigest())
ses=build/'segger_embedded_studio/Output/Debug/Exe/MicroLink.elf'
if ses.exists() and not args.gnu_only:
    report['ses']=elf_memory(ses)
    assert next(r['free'] for r in report['ses']['regions'] if r['name']=='APP_FLASH')>=8192
    assert len(ses.with_suffix('.bin').read_bytes())==int(report['ses']['flash_end'],16)-0x80023000
if args.write_report:
    (demo/'docs/build-validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    lines=['# MKLink V4 D12 发布核验 / D12 publication validation','',
        'D12：V13仍1503B，补存具体位置、朝向、时间与天气，资源启动不再传送车站。SD写入计算CRC16，等待延迟响应与忙结束，保留超时/回读/A/B并增加W阶段诊断。两地独立2～48小时雨期，分层长雨线和下延都市晨夜雾；码头、船位、岸上主角、鱼摊及路灯调整。删除最后距离锁，增强青鱼/鱤鱼真实冲刺。线组120/200/450G，都市600G八编PE，断线损坏；少于120G且没有细线即可免费补给。鱼摊06:00–16:00、便利店09:00–21:00。沿用2026.10.19.5资源，作者反馈实机基本稳定，已授权发布；掉电/拔卡和堆栈峰值尚未专项验收。',
        'D12 preserves exact outdoor position, time and regional weather in a1503-byte V13 save. SD writes use CRC16, bounded delayed-response/busy polling and wire diagnostics. Independent2–48h rain, lower city mist, dock/raft/actor/stall/lamp corrections and stronger black-carp/yellowcheek surges are included. The landing gate is removed. Rigs cost120/200/450G plus a600G city PE rig; true breaks destroy rigs, and a free fine rig is available below120G when none is owned. Existing2026.10.19.5 packs remain compatible. The author reports basic on-device stability and approved publication; power-loss/card-removal and hardware peak tests remain outstanding.','',
        'GNU RISC-V GCC 13.2、HPM SDK 1.11.0。2026-10-20。本次仅重建 GNU flash_uf2，SEGGER 工程输入已检查，未重新编译 SEGGER。','',
        '旧的容量错误测试版保持撤回。当前版本恢复原下载器的 SD 扇区映射，开放前检查原文件系统、容量和 MBR/BPB 边界；CPU 阻塞 SPI 路径不做 DMA 地址转换或接收后缓存失效。',
        'The original faulty-capacity build remains withdrawn. The current local importer restores stock SD LBA mapping with existing-volume/geometry checks, exclusive ownership and CPU-blocking SPI pointers. Model checks do not establish integrity of user media or hardware acceptance. No user-device writes or flashing were performed. Test results are recorded separately.','',
        f'Resource version: {packs["resource_version"]}; format {packs["pack_format"]}. Valid resources keep USB hidden. Missing packages can expose the existing SD only after its filesystem/geometry pass preflight; complete CRC checks and command drain precede detach. The author approved publishing D12 after reporting basic stability on2026-10-20.','',
        '地图测试覆盖两个真实 SD 包、CRC/文件长度/版本/地图/布局错误、预检失败不擦写、缓存复用、部分写入中断与重试、Flash 写失败、双向扣费与时间、金币不足、长按返回和随机状态不变。',
        'Map tests cover real packs, corrupt identities/CRC/size, no erases on failed preflight, cache reuse, interrupted writes and retry, flash failures, both-way fare/time, insufficient funds, hold-to-return and unchanged fishing RNG.','',
        '应用在 0x80023000–0x80080000；460 KiB 地图缓存在 0x80080000–0x800F3000。下载器和末尾 52 KiB 不写入。NOR 初始化和擦写函数均在 ILM，ROM 调用期间关闭中断，调用前后刷新看门狗。原两块 240×24 PFB 保留，另有 4 KiB 加载缓冲和 960 B 夜景行缓冲。',
        'The application ends before 0x80080000. The 460 KiB map cache ends before 0x800F3000; bootloader and final 52 KiB remain outside every write. The flash writer is in ILM, with IRQs masked during ROM operations. Original 240×24 PFB blocks remain; the loader adds a 4 KiB staging buffer and the night decoder uses 960 bytes.','',
        '| 工具链/区域 / Toolchain/region | 占用 / Used | 容量 / Capacity | 余量 / Free |',
        '|---|---:|---:|---:|']
    for tool in ('gnu','ses'):
        if tool in report:
            for r in report[tool]['regions']:lines.append(f"| {tool.upper()} / {r['name']} | {r['used']:,} B | {r['capacity']:,} B | {r['free']:,} B |")
    lines+=['',f"SD payload: {packs['payload_bytes']:,} B; cache header: 4,096 B; unused: {report['cache_free_bytes']:,} B.",'',
        'v13 存档 1503 字节，独立 A/B，保留原密钥、UUID、目录和所有 v1–v12 文件；保留旧红尾和旧白鲢记录，追加鳙鱼与来源。装备、声音、金币、图鉴和单鱼售价继续保留。',
        'The 1503-byte V13 save uses separate A/B files, preserving keys, UUID, directory and all V1–V12 files. Bighead is appended; old redtail and D6 silver carp records remain valid. Gear, sound, coins, collection and exact fish prices survive.','',
        '城市12种鱼采用独立数值与饵料权重，超级体型不为旧装备削弱。两图资源包包含24项活动鱼及旧红尾兼容贴图。路线图和坐船加载图标留在固件，缓存损坏时仍可显示错误与重试。',
        'The twelve city fish use independent profiles and bait weights; giants are not weakened for old gear. Both packs carry twenty-four active collection portraits plus the retired redtail art. Route UI and the seated fisherman/boat icon stay in firmware for failure/retry screens.','',
        '未执行实板烧录、拔卡/掉电、电气、帧率、栈水位或蜂鸣器试听；中断测试使用主机 Flash 模型。保留第三方库已有宏/未使用变量警告。',
        'No hardware flashing, card-removal/power-loss, electrical, FPS, stack-watermark or audio-audition tests were performed. Interruption tests use a host flash model. Existing third-party warnings remain.','',
        f"GNU UF2 SHA-256: `{report['firmware_sha256']}`."]
    (demo/'docs/validation.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(f'PASS: {len(paths)} SES sources, safe application/cache partitions, ILM flash writer, 2 SD packs, {count} UF2 blocks')
