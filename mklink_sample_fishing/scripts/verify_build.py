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
assert all(name in save_port for name in ("/NorthWetland_V%u_Save_%c_DO_NOT_DELETE.sav","/NorthWetland_Save_%c_DO_NOT_DELETE.sav","FISH_SAVE_VERSION-slot/2")),"Versioned and legacy save path construction required"
assert not any('mklink_sample_fire' in str(p) for p in paths),'Fire application leaked into fishing demo'
for item in tree.iter('configuration'):
    for folder in item.attrib.get('c_user_include_directories','').split(';'):
        if folder:assert (ide.parent/folder.replace('\\','/')).resolve().is_dir(),folder
gnu=elf_memory(build/'output/MicroLink.elf')
assert next(r['free'] for r in gnu['regions'] if r['name']=='APP_FLASH')>=8192,'Keep 8 KiB of application headroom'
app=(build/'output/MicroLink.app.bin').read_bytes()
assert all(name.encode() in app for name in ("/NorthWetland_V%u_Save_%c_DO_NOT_DELETE.sav","/NorthWetland_Save_%c_DO_NOT_DELETE.sav")),"Firmware must contain modern and legacy save formats"
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
    build_date='2026-11-02',
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
    diagnostic_revision='D29',neutral_tier_priors=[70,27,3],bait_can_exclude_bronze=True,gold_pool_cap_percent=5,turtle_encounter_weight=3,turtle_shrimp_weight=6,turtle_smallfish_weight=4,most_fish_horizontal_unrestricted=True,fish_only_sound='strike_window_only',strike_window_cue_hz=160,strike_window_cue_ms=60,strike_window_cue_once=True,rest_save_requests_coalesced=True,rest_checkpoint_wall_ms=300000,rest_exit_flush_latest=True,city_large_box_capacity_g=80000,city_large_box_price=1800,city_big_bucket_for_sale=False,save_slots=38,predator_burst_extras=True,snakehead_hook_pressure_percent=100,fight_mass_drag=True,fight_final_safe_gain_min=1,heavy_burst_load_percent=150,line_response_percent=[160,100,65,100,80],line_comfortable_weight_g=[300,1500,6500,40000,60000],max_line_response_percent=220,large_safe_reeling_clears_slack=True,beach_station_x=-312,beach_stall_x=-112,beach_station_screen_size=[192,96],beach_stall_screen_size=[112,76],beach_employee_expressions=4,beach_employee_modern_uniform=True,common_fish_pack=True,lossless_fish_codec=True,fish_raw_bytes=455616,fish_packed_bytes=packs['common']['payload_bytes'],chub_mackerel_native_size=[96,56],common_cache_bytes=131072,map_cache_bytes=339968,fish_decode_slots=2,fish_decode_ahb_bytes=32256,map_switch_preserves_common_bank=True,beach_map_implemented=True,beach_world_width=1680,beach_sand_width=768,beach_depth_max_cm=12500,beach_shelf_edge_x=1080,beach_boat_limit_depth_cm=12500,beach_modern_fish_stall=True,beach_stall_hours=[360,960],beach_road_seawall=True,beach_wooden_pier=False,beach_boat_moors_at_reef=True,beach_road_world_range=[-384,112],beach_road_orientation="horizontal",beach_parking_bays=2,beach_road_native_size=[248,36],beach_land_world_range=[-384,480],beach_sand_world_range=[-384,384],beach_walk_range=[-352,480],beach_reef_world_range=[272,512],beach_tidal_flat_width=240,beach_foraging_space_reserved=True,beach_foraging_playable=True,beach_reef_transition=True,beach_fish_implemented=True,marine_records=25,marine_croaker_rarity="gold",marine_night_surface_species=[34,45],sea_light_target_species=[34,45,46],sea_line_price=1200,sea_line_length_cm=20000,sea_heavy_rod_price=6000,sea_heavy_rod_cast_px=126,sea_heavy_rod_reel_percent=150,sea_lamp_price=2800,sea_lamp_world_sprite_pixels=[12,16],sea_lamp_light_texture_pixels=[96,28],sea_lamp_in_save=True,marine_sinking_cm_per_s=200,marine_empty_retrieve_base_cm_per_s=300,marine_range_includes_depth=True,all_maps_range_includes_depth=True,line_length_cm=[450,650,2000,2500,20000],bottom_grade_limits_exclusive_cm=[500,1000,None,None,None],short_line_fallback_order=["middle","surface"],strong_rig_all_freshwater_bottom=True,beach_sand_reef_blend_px=32,marine_price_changes=14,short_line_falls_back_to_surface=True,sea_pe_bottom_grade_limit_cm=None,marine_fishing_species=15,marine_foraging_placeholders=0,marine_foraging_species=9,forage_spots_per_area=[3,6],forage_refresh_minute=240,forage_tool_prices=[120,180],forage_tool_durability=20,forage_escape_percent=60,forage_qte_timeout_ms=6500,ragworm_wait_percent=65,forage_state_in_save=True,oarfish_special_per_mille=10,peacock_variant_per_mille=10,sea_cabin_capacity_g=200000,sea_cabin_price=2500,sea_cabin_map=2,save_readback_chunk_bytes=512,save_readback_last_chunk_bytes=99,backpack_bitmap_in_scene_pack=True,marine_card_max_visible_pixels=[180,56],beach_motor_speedboat=True,beach_lighthouse_rotation_ms=12000,beach_island_count=1,beach_clean_sand=True,beach_stronger_storm=True,beach_route_position='city_southeast',cat_small_popup=True,cat_no_visible_respawn=True,cat_stroll_span_px=56,moon_phases=8,moon_cycle_game_days=24,city_store_rain_shelter=True,city_stall_rain_shelter=True,cat_companion=True,cat_hourly_chance_percent=85,cat_visit_full_hour=True,cat_follow_maps=[0,1,2],boat_rest_menu=True,bag_clock_visible=True,rest_elapsed_visible=True,rest_elapsed_retained_on_release=True,rest_elapsed_in_save=False,boat_rest_speed=20,boat_rest_hold_ms=600,boat_rest_release_stops=True,boat_rest_saved_active=False,cat_food_price=25,cat_feed_per_day=2,cat_actions_per_encounter=3,cat_friendship_to_join=50,cat_variant_per_mille=25,default_variant_per_mille=10,visible_depth_modes=3,legacy_depth_storage_ids=8,lifelike_lure_price=600,lifelike_lure_consumable=False,bamboo_max_line_index=1,stern_fallback_by_remaining_capacity=True,shop_wheel_direction=-1,shore_walk_speed_px_per_s=[32,64],shore_two_gears=True,rain_duration_game_hours=[2,48],independent_regional_weather=True,near_rain_max_opacity=224,large_fish_landing_gate=False,line_break_destroys_current_rig=False,line_break_damage_percent=10,line_damage_roll_once_per_break=True,puffer_escape_bite_percent=20,puffer_bite_independent=True,puffer_double_rig_consumption=False,line_prices=[120,200,450,600],pe8_line_city_price=600,pe8_line_small_fish_filter=True,free_fine_rig_below_coins=120,free_fine_rig_requires_empty_cargo=False,city_extra_water_offset_px=8,shore_actor_scale=1.8,city_stall_screen_size=[100,76],city_outdoor_monger_behind_counter=True,city_dry_night_dawn_mist=True,rain_and_distant_fog=True,rain_bitmap_bytes=0,wetness_fill_game_minutes=60,wetness_dry_game_minutes=120,wetness_speed_loss_percent=20,cpu_fault_register_capture=True,diagnostic_pause_before_sd=True,
    motorboat_slow_px_per_s=10,motorboat_fast_px_per_s=60,motorboat_fast_wake=True,lighthouse_fog_scattering=True,lighthouse_after_night_tint=True,lighthouse_continuous_rotation=True,lighthouse_front_back_perspective=True,lighthouse_foreground_depth_clipping=True,lighthouse_run_length_opacity_fill=True,coast_skyline_preserves_blue_island=True,coast_fog_parallax_divisor=8,
    forage_batch_shell_count=[2,4],forage_batch_ragworm_count=[3,5],forage_gold_chance_percent=3,forage_gold_species=[48,49],forage_gold_base_prices=[80,80],bronze_price_floor=20,silver_low_price_target=50,
    forage_random_positions=True,forage_min_spacing_px=2,forage_allow_clusters=True,forage_pin_shape='inverted_drop',forage_ground_dot=False,forage_auto_stop=False,
    forage_tool_hud_position='upper_right',forage_tool_hud_background=False,
    forage_pin_visibility_radius_px=48,forage_pin_tip_at_ground=True,forage_pin_hidden_behind_actor=True,
    forage_qte_fastest_period_ms=1200,forage_qte_target_width_px=52,forage_qte_minimum_press_window_ms=200,
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
    read_only_common_and_three_map_preflight=True,lossless_indexed_shop_in_sd=True,
    hardware_usb_tested=False,ses_rebuilt=not args.gnu_only,
    hardware_platform='MKLink V4',test_counts_not_inferred_from_build=True,display_rotation=0,
    distribution_status='d29',original_sd_integrity='unknown',git_push_allowed=False,user_reported_basic_stability=False,agent_hardware_tested=False,
    public_runtime_art_only=True,hardware_flashed=False,hardware_power_loss_tested=False,
    save_manual_io_retry=True,save_preflight_validation=True,save_error_stage_codes=True,city_raft_seated_sprite=True,common_wetland_player_pixels=True,world_foreground_shift_px=8,
    hold_collection_frame=True,hold_container_icon_only=True,night_road_darkening=True,
    sd_writes_single_sector_cmd24=True,sd_write_clock_hz=4000000,sd_read_clock_restored=True,sd_write_crc16=True,sd_delayed_response_poll=True,sd_busy_wait_bounded=True,sd_wire_diagnostics=True,
    large_fish_control_threshold_g=6500,large_fish_landing_stamina_percent=None,giant_additional_force_above_g=20000,
    city_long_rod_price=3500,city_long_rod_cast_px=180,city_long_rod_reel_percent=125,umbrella_price=160,noodles_price=28,noodles_duration_game_hours=4,noodles_handling_percent=110,noodles_strike_window_percent=100,refreshments_stack=False,save_key_public=True,save_schema_version=22,save_exact_position=True,save_clock_fraction_ms=4,save_bytes=2147,legacy_save_versions=[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21],
    legacy_save_bytes=[92,236,236,236,292,293,1317,1318,1470,1485,1495,1503,1511,2037,2062,2097],map_page_implemented=True,station_travel_implemented=True,
    city_fish_implemented=True,city_fish_count=12,new_fish_count=9,collection_count=49,internal_species_slots=50,retired_redtail_slot=21,bighead_slot=24,
    silver_carp_limit_cm_kg=[100,40],bighead_carp_limit_cm_kg=[145,60],
    silver_carp_layer='surface',silver_carp_rarity='bronze',bighead_carp_layers=['middle','lower'],bighead_carp_rarity='silver',
    city_depth_max_cm=1600,city_nearshore_depth_cm=[360,420],old_v9_silver_profile_preserved=True,
    city_independent_profiles=True,cross_map_catch_origin_preserved=True,
    city_shop_implemented=True,city_shop_hours=[9,21],city_tackle_hours=[6,16],wetland_shop_hours=[7,18],
    city_clerk_expressions=5,city_clerk_dialogue_contexts=16,city_monger_dialogue_contexts=15,
    city_atmospheric_panorama=True,weight_kg_above_grams=1000,weight_kg_decimal_places=1,
    hold_portraits_fit_visible_alpha_bounds=True,hold_portrait_area=[160,56],
    moon_before_foreground_buildings=True,city_ground_solid_at_y=136,
    generated_park_lamp_bitmaps=True,independent_soft_light_texture=[36,36],
    park_light_hours=[18,6],park_light_alpha_upper_bound=96,park_light_behind_actor=True,
    sd_map_asset_loader_implemented=True,travel_fare_each_direction=None,travel_fares_by_map=[[0,180,460],[180,0,280],[460,280,0]],travel_minutes_each_direction=300,
    application_end_exclusive='0x80080000',cache_base='0x80080000',cache_end_exclusive='0x800f3000',
    cache_bytes=471040,cache_header_bytes=8192,cache_payload_bytes=packs['payload_bytes'],
    cache_free_bytes=471040-8192-packs['payload_bytes']-packs['common']['payload_bytes'],common_payload_bytes=packs['common']['payload_bytes'],common_cache_free_bytes=131072-4096-packs['common']['payload_bytes'],map_cache_free_bytes=471040-131072-4096-packs['payload_bytes'],external_bitmap_arrays=len(packs['entries']),
    common=packs['common'],maps=packs['maps'],boot_cache_crc_checked=True,source_preflight_before_erase=True,cache_header_committed_last=True,
    source_and_cache_content_identity_checked=True,readback_verified=True,shared_filesystem_mutex=True,
    loader_staging_bytes=4096,shared_day_night_decode_buffer_bytes=960,panorama_palette_colors=64,loading_flavor_text=True,new_full_framebuffer_bytes=0,route_ui_rom_resident=True,
    virtual_route_size=[640,480],route_viewport=[8,40,224,136],route_pan_ms=360,route_view_state_bytes=16,
    route_thumbnail_size=[64,32],route_selected_alpha=255,route_unselected_alpha=192,
    route_compact_overview=False,route_pan_only_when_network_outgrows_viewport=True,
    route_terrain_symbols=['river','bridge','mountains','forest','pond','reeds','city_blocks'],
    route_terrain_bitmap_bytes=0,
    city_neon_layer=True,city_sweeping_spotlights=True,city_inflatable_raft=True,
    line_lengths_cm=[450,650,2000,2500,20000],rod_elastic_grace_ms=2000,burst_away_percent=70,
    chinese_font_pixel_size=12,sound_setting_in_save=True,guide_pages=2,guide_hold_ms=600,
    firmware_sha256=hashlib.sha256((build/'output/MicroLink.fish.uf2').read_bytes()).hexdigest())
ses=build/'segger_embedded_studio/Output/Debug/Exe/MicroLink.elf'
if ses.exists() and not args.gnu_only:
    report['ses']=elf_memory(ses)
    assert next(r['free'] for r in report['ses']['regions'] if r['name']=='APP_FLASH')>=8192
    assert len(ses.with_suffix('.bin').read_bytes())==int(report['ses']['flash_end'],16)-0x80023000
if args.write_report:
    (demo/'docs/build-validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    lines=['# MKLink V4 D29 构建核验 / Build validation','',
        'D29：三图水平分布、金级概率与鳖饵料复核；猫85%整小时候选。详见[分布设定](encounters-d29.md)。D28R2：仅鱼讯只在可提竿窗口响一次；全部开启保留上钩、挣扎和收银。详见[声音设置](strike-cue-d28-r2.md)。D28R1：长按休息合并存档请求，结束时保存最新状态，保留真实五分钟检查点。详见[休息存档](rest-save-d28-r1.md)。D28：江河新增80kg大型储鱼箱1800G、下架大木桶；黑鱼首口重压，鳡／斑鳜／大梭鱼／金枪鱼等挣扎特征复核。V22仍2147B，保留V21旧档与资源。详见[predators-d28.md](predators-d28.md)。D27：全鱼种搏鱼平衡复核，修正大鱼收线过快、强线双重减压，以及深水末段零进度／疲劳误判松线。详见[fight-d27.md](fight-d27.md)。D26R2：背包常显游戏时间，休息显示本次已过去的小时／分钟，松开保留结果。详见[时间显示](cat-rest-d26-r2.md)。D26R1：猫每小时出现概率65%，三图跟随；船上选中休息，长按600ms后20倍推进时间，松开恢复。存档和资源不变。详见[猫与休息](cat-rest-d26-r1.md)。本版：海图1680px、右端125m，连续旋转灯塔，批量赶海与两种金色生物，偏低铜／银基价20G／50G。V21存档2147B迁移旧档，完整资源2026.10.31.4。详见[本版设定](rewards-d26.md)、[回归核验](rewards-d26-build-validation.json)、[内存报告](rewards-d26-memory.md)。本地未上传。 / Extended deep sea, continuous lighthouse rotation, batch shore rewards, gold species and price increases. V21 migrates older saves; matching2026.10.31.4 resources. Local only.','',
        '以下为历史版本摘要；以本版设定及本文件数值表为准。 / Earlier revision notes follow; current settings and measured region sizes take precedence.','',
        'D24：三图按线组和真实线长钓底；细线<5m、通用<10m，强韧及以上只受线长限制。强韧20m、八编25m，无法钓底依次转中层／表层。合入作者MD价格；沙滩与棱角礁石32px渐接。完整资源2026.10.31.1，V19旧档保留。本地未上传。 / Shared depth-aware rig resolution, longer freshwater tackle, edited prices and a seamless sand/reef join; compatible V19 saves and matching resources.','',
        '历史D19：海滩现代鱼档、公路堤岸、礁石衔接，以及缓坡大陆架到右四分之一陡降的深水曲线；船只最右水深114米。配套完整2026.10.29.3格式3资源，V17存档不变；海鱼待加入，本地未推送。 / D19 adds a modern coastal vendor, seawall road, reef transition and a continental slope reaching114m at the boat limit. Matching resources required; marine species deferred.','',
        '历史D18：28张鱼图无损压缩75.2%，独立公共鱼图包/缓存，换地图只擦写场景分区。原海滩D17：新增右下海滩锚点、1440像素长地图、快艇、单岛灯塔及强暴雨；V17独立存档，配套三图资源2026.10.29.1。海鱼尚未加入，本版未上传。 D16历史：轻量猫选项框、岸边往返与屏内状态锁、八阶段月相及店面遮雨；继承D15：流浪猫好感/每日喂食/同伴与2.5%变异加成；修复升级容器误放流，限制竹竿配线，三档钓层及600G永久拟真假饵。V16存档1511B，保留所有V1–V15旧档。 / D16 adds a small cat menu, stable visible visits, eight moon phases and roof shelter; V16 saves retain all older files.','',
        '继承更新：商店列表反转滚轮方向；岸上增加32/64像素每秒两档步行，反向先停、交互点自动停车。旧存档与SD资源包保持兼容。 / Local revision: reversed shop-wheel navigation and32/64px/s shore walking, retaining reverse-to-stop and interaction-point stops. Existing saves and packs remain compatible.','',
        '断线后仅10%概率损坏线组，其余保留；河豚中钩后逃脱额外独立20%概率咬断，最多消耗一副；泡面4游戏小时只提升收放线速度10%，不增加提竿窗口。热拿铁保持6游戏小时、收放线和提竿窗口各+10%。采用情境化提示，旧存档和配套SD资源保持兼容。以下为继承的D13功能。',
        'Each line break makes one10% rig-damage roll. Hooked puffer escapes add an independent20% bite-through chance, consuming at most one rig. Noodles grant10% handling only for four game hours; latte keeps both10% bonuses for six hours. Existing saves and packs remain compatible. Inherited D13 features follow.','',
        'D13新增都市3500G高碳长竿（7.2米、125%收线、大鱼张力缓冲）、160G永久雨伞及28G泡面（4游戏小时、收放线速度+10%）。雨伞岸上手持、船上固定，阻止新潮湿。泡面和35G拿铁共用状态，后买替换不叠加。V14仍1503B并独立保留V13等旧档；配套资源2026.10.19.5不变。D13新增内容待实机验证，D12稳定反馈不等同新功能已验收。',
        'D13 adds a3500G city long rod (7.2m,125% retrieval, reduced detent tension above6.5kg), a160G permanent umbrella and28G noodles (four game hours,+10% handling only). The umbrella is held on shore and mounted on the boat. Meal/drink buffs replace rather than stack. V14 remains1503bytes and retains V13 files. The same SD packs work. New D13 features await on-device testing.','',
        'GNU RISC-V GCC 13.2、HPM SDK 1.11.0。2026-11-02。本次仅重建 GNU flash_uf2，SEGGER 工程输入已检查，未重新编译 SEGGER。','',
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
        '历史V16存档为1511字节，原8字节猫状态的空余位保存出现/走动状态，独立A/B，保留原密钥、UUID、目录和所有V1–V15文件。装备、声音、金币、图鉴和单鱼售价继续保留。',
        'Historical: the 1511-byte V16 save packs cat visibility/walk state into the existing8cat bytes and uses separate A/B files, preserving keys, UUID, directory and all V1–V15 files. Bighead is appended; old redtail and D6 silver carp records remain valid. Gear, sound, coins, collection and exact fish prices survive.','',
        '城市12种鱼采用独立数值与饵料权重，超级体型不为旧装备削弱。三图共用53张公共鱼图、49项可见图鉴，旧红尾保留兼容。路线图和坐船加载图标留在固件，缓存损坏时仍可显示错误与重试。',
        'The twelve city fish use independent profiles and bait weights; giants are not weakened for old gear. Three maps share53 portraits and49 visible collection entries, retaining retired redtail compatibility. Route UI and the seated fisherman/boat loading icon stay in firmware for failure/retry screens.','',
        '未执行实板烧录、拔卡/掉电、电气、帧率、栈水位或蜂鸣器试听；中断测试使用主机 Flash 模型。保留第三方库已有宏/未使用变量警告。',
        'No hardware flashing, card-removal/power-loss, electrical, FPS, stack-watermark or audio-audition tests were performed. Interruption tests use a host flash model. Existing third-party warnings remain.','',
        f"GNU UF2 SHA-256: `{report['firmware_sha256']}`."]
    (demo/'docs/validation.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(f'PASS: {len(paths)} SES sources, safe application/cache partitions, ILM flash writer, 4 SD packs, {count} UF2 blocks')
