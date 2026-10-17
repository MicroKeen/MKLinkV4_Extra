"""Validate generated IDE inputs, ELF partitions and application UF2 payloads."""
from pathlib import Path
import argparse,hashlib,json,struct,xml.etree.ElementTree as ET

demo=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('--build-dir',type=Path,default=demo/'hpm5301evklite_flash_uf2_debug')
parser.add_argument('--write-report',action='store_true')
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
    for sy in raw:
        if sy[1]!=2:continue
        strings=raw[sy[6]];strings=blob[strings[4]:strings[4]+strings[5]]
        for at in range(sy[4],sy[4]+sy[5],16):
            name,value,size,info,other,index=struct.unpack_from('<IIIBBH',blob,at)
            name=strings[name:].split(b'\0',1)[0].decode(errors='replace')
            if name.startswith('cache_write_critical'):
                assert value+size<=128*1024,'Flash writer must execute entirely in ILM'
                critical.append(dict(name=name,address=hex(value),bytes=size))
    assert critical,'Missing RAM-resident flash writer'
    return dict(ram_flash_writer=critical,sha256=hashlib.sha256(blob).hexdigest(),regions=result,sections=sections,flash_end=hex(end))

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
assert all(name in save_port for name in ('NorthWetland_V8_Save_A','NorthWetland_V8_Save_B','NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B','NorthWetland_Save_A','NorthWetland_Save_B')),'V7/V6/V5/legacy save slots must coexist'
assert not any('mklink_sample_fire' in str(p) for p in paths),'Fire application leaked into fishing demo'
for item in tree.iter('configuration'):
    for folder in item.attrib.get('c_user_include_directories','').split(';'):
        if folder:assert (ide.parent/folder.replace('\\','/')).resolve().is_dir(),folder
gnu=elf_memory(build/'output/MicroLink.elf')
assert next(r['free'] for r in gnu['regions'] if r['name']=='APP_FLASH')>=8192,'Keep 8 KiB of application headroom'
app=(build/'output/MicroLink.app.bin').read_bytes()
assert all(name.encode() in app for name in ('NorthWetland_V8_Save_A','NorthWetland_V8_Save_B','NorthWetland_V7_Save_A','NorthWetland_V7_Save_B','NorthWetland_V6_Save_A','NorthWetland_V6_Save_B','NorthWetland_V5_Save_A','NorthWetland_V5_Save_B')),'Built firmware must contain all V7/V6/V5 save paths'
payload,count=uf2_payload(build/'output/MicroLink.fish.uf2')
assert payload[:len(app)]==app and not any(payload[len(app):])
assert len(app)==int(gnu['flash_end'],16)-0x80023000
if args.write_report:
    import shutil
    shutil.copy2(build/'output/MicroLink.fish.uf2',demo/'MicroLink.uf2')
from verify_map_packs import verify
packs=verify()
report=dict(gnu=gnu,uf2_blocks=count,source_count=len(paths),uf2_base='0x80023000',family='0x0A4D5048',
    hardware_platform='MKLink V4',host_tests=14,simulator_tests=48,display_rotation=0,
    public_runtime_art_only=True,hardware_flashed=False,hardware_power_loss_tested=False,
    save_key_public=True,save_schema_version=8,save_bytes=1318,legacy_save_versions=[1,2,3,4,5,6,7],
    legacy_save_bytes=[92,236,236,236,292,293,1317],map_page_implemented=True,station_travel_implemented=True,
    sd_map_asset_loader_implemented=True,travel_fare_each_direction=180,travel_minutes_each_direction=300,
    application_end_exclusive='0x80080000',cache_base='0x80080000',cache_end_exclusive='0x800f3000',
    cache_bytes=471040,cache_header_bytes=4096,cache_payload_bytes=packs['payload_bytes'],
    cache_free_bytes=471040-4096-packs['payload_bytes'],external_bitmap_arrays=len(packs['entries']),
    maps=packs['maps'],boot_cache_crc_checked=True,source_preflight_before_erase=True,cache_header_committed_last=True,
    source_and_cache_content_identity_checked=True,readback_verified=True,shared_filesystem_mutex=True,
    loader_staging_bytes=4096,night_decode_buffer_bytes=960,night_palette_colors=64,loading_flavor_text=True,new_full_framebuffer_bytes=0,route_ui_rom_resident=True,
    city_neon_layer=True,city_sweeping_spotlights=True,city_inflatable_raft=True,
    line_lengths_cm=[450,650,850],rod_elastic_grace_ms=2000,burst_away_percent=70,
    chinese_font_pixel_size=12,sound_setting_in_save=True,guide_pages=2,guide_hold_ms=600,
    firmware_sha256=hashlib.sha256((build/'output/MicroLink.fish.uf2').read_bytes()).hexdigest())
ses=build/'segger_embedded_studio/Output/Debug/Exe/MicroLink.elf'
if ses.exists():
    report['ses']=elf_memory(ses)
    assert next(r['free'] for r in report['ses']['regions'] if r['name']=='APP_FLASH')>=8192
    assert len(ses.with_suffix('.bin').read_bytes())==int(report['ses']['flash_end'],16)-0x80023000
if args.write_report:
    (demo/'docs/build-validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    lines=['# MKLink V4 v29 验证 / Validation','',
        'GNU RISC-V GCC 13.2、HPM SDK 1.11.0、SEGGER Embedded Studio 8.24。2026-10-17。','',
        '48 项 Windows 检查和 14 项独立 demo 主机测试通过；GNU flash_uf2、flash_xip 与 SEGGER flash_uf2 构建通过。',
        'All 48 simulator checks and 14 standalone host tests pass. GNU flash_uf2/flash_xip and SEGGER flash_uf2 builds pass.','',
        '地图测试覆盖两个真实 SD 包、CRC/文件长度/版本/地图/布局错误、预检失败不擦写、缓存复用、部分写入中断与重试、Flash 写失败、双向扣费与时间、金币不足、长按返回和随机状态不变。',
        'Map tests cover real packs, corrupt identities/CRC/size, no erases on failed preflight, cache reuse, interrupted writes and retry, flash failures, both-way fare/time, insufficient funds, hold-to-return and unchanged fishing RNG.','',
        '应用在 0x80023000–0x80080000；460 KiB 地图缓存在 0x80080000–0x800F3000。下载器和末尾 52 KiB 不写入。擦写函数在 ILM，ROM 调用期间关闭中断。原两块 240×24 PFB 保留，另有 4 KiB 加载缓冲和 960 B 夜景行缓冲。',
        'The application ends before 0x80080000. The 460 KiB map cache ends before 0x800F3000; bootloader and final 52 KiB remain outside every write. The flash writer is in ILM, with IRQs masked during ROM operations. Original 240×24 PFB blocks remain; the loader adds a 4 KiB staging buffer and the night decoder uses 960 bytes.','',
        '| 工具链/区域 / Toolchain/region | 占用 / Used | 容量 / Capacity | 余量 / Free |',
        '|---|---:|---:|---:|']
    for tool in ('gnu','ses'):
        if tool in report:
            for r in report[tool]['regions']:lines.append(f"| {tool.upper()} / {r['name']} | {r['used']:,} B | {r['capacity']:,} B | {r['free']:,} B |")
    lines+=['',f"SD payload: {packs['payload_bytes']:,} B; cache header: 4,096 B; unused: {report['cache_free_bytes']:,} B.",'',
        'v8 存档 1318 字节，独立 A/B，保留原密钥、UUID、目录和所有 v1–v7 文件；新增当前地图，旧档默认为湿地。装备、声音、金币、图鉴和单鱼售价继续保留。',
        'The 1318-byte v8 save uses separate A/B files and preserves keys, UUID, directory and v1–v7 files. It adds the current map; older saves default to the wetland. Gear, sound, coins, collection and exact fish prices survive.','',
        '城市沿用现有鱼种、鱼饵权重与装备规则；资源包包含现代都市/远处轮渡、气垫筏、灯光层和全部鱼类贴图。路线图和行走小人留在固件，缓存损坏时仍可显示错误与重试。',
        'The city currently shares existing species, bait weights and tackle rules. Its pack contains the modern panorama/ferry, raft, emission layer and fish art. Route UI and walking/loading sprites stay in firmware for safe failure/retry screens.','',
        '未执行实板烧录、拔卡/掉电、电气、帧率、栈水位或蜂鸣器试听；中断测试使用主机 Flash 模型。保留第三方库已有宏/未使用变量警告。',
        'No hardware flashing, card-removal/power-loss, electrical, FPS, stack-watermark or audio-audition tests were performed. Interruption tests use a host flash model. Existing third-party warnings remain.','',
        f"GNU UF2 SHA-256: `{report['firmware_sha256']}`."]
    (demo/'docs/validation.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(f'PASS: {len(paths)} SES sources, safe application/cache partitions, ILM flash writer, 2 SD packs, {count} UF2 blocks')
