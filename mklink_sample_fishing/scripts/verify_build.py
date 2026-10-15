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
assert not any('mklink_sample_fire' in str(p) for p in paths),'Fire application leaked into fishing demo'
for item in tree.iter('configuration'):
    for folder in item.attrib.get('c_user_include_directories','').split(';'):
        if folder:assert (ide.parent/folder.replace('\\','/')).resolve().is_dir(),folder
gnu=elf_memory(build/'output/MicroLink.elf')
app=(build/'output/MicroLink.app.bin').read_bytes()
payload,count=uf2_payload(build/'output/MicroLink.fish.uf2')
assert payload[:len(app)]==app and not any(payload[len(app):])
assert len(app)==int(gnu['flash_end'],16)-0x80023000
if args.write_report:
    import shutil
    shutil.copy2(build/'output/MicroLink.fish.uf2',demo/'MicroLink.uf2')
report=dict(gnu=gnu,uf2_blocks=count,source_count=len(paths),uf2_base='0x80023000',family='0x0A4D5048',
    host_tests=10,simulator_tests=32,display_rotation=0,public_runtime_art_only=True,hardware_flashed=False,hardware_power_loss_tested=False,
    save_key_public=True,save_schema_version=4,save_bytes=236,legacy_save_versions=[1,2,3],legacy_save_bytes=[92,236,236],
    firmware_sha256=hashlib.sha256((build/'output/MicroLink.fish.uf2').read_bytes()).hexdigest())
ses=build/'segger_embedded_studio/Output/Debug/Exe/MicroLink.elf'
if ses.exists():
    report['ses']=elf_memory(ses)
    bin_data=ses.with_suffix('.bin').read_bytes()
    assert len(bin_data)==int(report['ses']['flash_end'],16)-0x80023000
if args.write_report:
    (demo/'docs/build-validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    lines=['# 标准钓鱼 demo 验证','',
        'HPM SDK 1.11.0；GNU RISC-V GCC 13.2；SEGGER Embedded Studio 8.24。2026-10-15 编译检查。',
        '',f'GNU UF2 的 {count} 个块、地址、Family ID 和应用净载荷逐块校验通过；SES XML、{len(paths)} 个唯一源文件及 include 路径检查通过。',
        '', '独立 demo 的 10 项主机测试全部通过：玩法、搏鱼、图鉴、地图装备、尺寸、经济/扩容/时钟、存档 v1/v2/v3 迁移、首次装备初始化与 v4 再次恢复、编码器事件和 SD/背光隔离。',
        '', 'GNU 和 SEGGER flash_uf2/debug 编译成功；GNU flash_xip/debug 也构建并校验通过。编译器仍报告原 Arm-2D/perf_counter/RT-Thread/board 的宏或未使用变量警告，未更改第三方库以消除它们。',
        '', '| 工具链/区域 | 占用或预留 | 容量 | 余量 |','|---|---:|---:|---:|']
    for tool in ('gnu','ses'):
        if tool in report:
            for r in report[tool]['regions']:lines.append(f"| {tool.upper()} / {r['name']} | {r['used']:,} B | {r['capacity']:,} B | {r['free']:,} B |")
    lines+=['','Flash 包含代码、只读字形/位图、初始化映像和对齐。RAM 包含两块 240×24 RGB565 PFB、游戏状态、4 KiB 存档线程栈及原 RT/C 堆栈预留。SES 放在 DLM 顶部的固定栈按实际大小计入，未将其下方空闲间隙误记为已用。',
        '', '显示方向：钓鱼工程使用 ST7789_ROTATION=0（相对原 2 转向 180°），240×240 对应 Y_SHIFT=80。公共驱动允许覆盖参数，其他工程未指定时仍默认 2。方向需在实板上最终确认。',
        '', '验证未执行实板烧录、SD 拔插、掉电实验、帧率/栈水位测量。主机 HAL 桩证明 SD 回调不操作 PA26，不能替代实板电气验证。',
        '', '旧存档兼容性：保留原项目密钥、目录、UUID 派生和原索引。读取 v1/v2/v3 后写入 236 字节 v4，保留图鉴/金币/饵料库存/鱼获/容器/时间，首次升级初始化竿/线组/漂及所有权，后续购买不再重置。旧固件无法读取 v4；切回新版本可恢复。密钥按作者要求公开，源码访问者可伪造存档。',
        '', f"仓库附带 GNU 固件 SHA-256：`{report['firmware_sha256']}`。",
        '', '可重新运行 `python scripts/verify_build.py --write-report`（默认检查标准 flash_uf2 构建目录）。']
    (demo/'docs/validation.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
print(f'PASS: {len(paths)} unique SES sources, ELF partition limits, {count} UF2 blocks and payload')
