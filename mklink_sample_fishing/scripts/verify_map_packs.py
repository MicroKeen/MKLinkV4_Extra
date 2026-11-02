"""Verify deployed SD packs against firmware addresses and compiled identities."""
from pathlib import Path
import hashlib,json,re,struct,zlib
D=Path(__file__).resolve().parents[1]
def verify():
    root=D/'sdcard/FishingGame_Resources_DO_NOT_DELETE'
    m=json.loads((root/'manifest.json').read_text(encoding='utf-8'))
    code=(D/'src/fish/fish_asset_layout.h').read_text(encoding='utf-8')
    def constant(name):return int(re.search(r'#define '+name+r' (0x[0-9a-f]+|\d+)u',code)[1],0)
    assert m['payload_bytes']==constant('FISH_ASSET_PAYLOAD_BYTES')<=0x52000
    assert int(m['layout_crc'],16)==constant('FISH_ASSET_LAYOUT_CRC')
    assert m['pack_format']==constant('FISH_ASSET_PACK_FORMAT')==3
    assert m['resource_version_id']==constant('FISH_ASSET_VERSION_ID')
    assert (root/'VERSION.txt').read_text().strip()==m['resource_version']
    assert len(m['maps'])==constant('FISH_ASSET_MAP_COUNT')==3
    end=0
    for e in m['entries']:
        assert e['offset']%4==0 and e['offset']>=end
        assert e['offset']==constant('FISH_OFFSET_'+e['symbol'])
        end=e['offset']+e['bytes'];assert end<=m['payload_bytes']
    night=constant('FISH_ASSET_NIGHT_OFFSET');sky=constant('FISH_ASSET_SKYLINE_OFFSET')
    day=constant('FISH_ASSET_DAY_OFFSET')
    assert end<=day and day+28928==sky and sky+240==night and night+28928==m['payload_bytes']
    for row in m['maps']:
        assert row['file']==('maps/0_wetland.fgpk','maps/1_city_river.fgpk','maps/2_beach.fgpk')[row['id']]
        b=(root/row['file']).read_bytes();assert len(b)==row['bytes']==64+m['payload_bytes']
        assert hashlib.sha256(b).hexdigest()==row['sha256']
        magic,version,map_id,layout,n,crc,resource_version,hcrc=struct.unpack_from('<4s7I',b)
        assert magic==b'FGRP' and version==3 and map_id==row['id']
        assert resource_version==constant('FISH_ASSET_VERSION_ID')
        assert layout==constant('FISH_ASSET_LAYOUT_CRC') and n==m['payload_bytes']
        assert crc==zlib.crc32(b[64:])==constant(f'FISH_ASSET_MAP{map_id}_CRC')
        assert hcrc==zlib.crc32(b[:28]) and not any(b[32:64])
        assert max(b[64+sky:64+sky+240])<=120
        assert max(b[64+day+128:64+day+28928])<64
        layer=b[64+night:64+night+28928]
        if map_id:assert max(layer[128:])<64 and any(layer[:128])
        else:assert not any(layer)
    common=m['common'];b=(root/common['file']).read_bytes()
    assert common['id']==constant('FISH_COMMON_RESOURCE_ID')==3
    assert len(b)==common['bytes']==64+constant('FISH_COMMON_PAYLOAD_BYTES')
    assert hashlib.sha256(b).hexdigest()==common['sha256']
    magic,version,resource_id,layout,n,crc,resource_version,hcrc=struct.unpack_from('<4s7I',b)
    assert magic==b'FGRP' and version==3 and resource_id==3
    assert layout==constant('FISH_ASSET_LAYOUT_CRC') and resource_version==constant('FISH_ASSET_VERSION_ID')
    assert n==constant('FISH_COMMON_PAYLOAD_BYTES') and crc==constant('FISH_COMMON_CRC')==zlib.crc32(b[64:])
    assert hcrc==zlib.crc32(b[:28]) and not any(b[32:64])
    assert n<=constant('FISH_COMMON_CACHE_BYTES')-4096
    assert constant('FISH_COMMON_CACHE_BYTES')==constant('FISH_MAP_CACHE_OFFSET')==0x20000
    assert constant('FISH_MAP_CACHE_OFFSET')+4096+m['payload_bytes']<=0x73000
    for im in common['images']:
        raw=b[64+im['offset']:64+im['offset']+im['bytes']];count=im['width']*im['height'];bits=im['bits']
        assert len(raw)==im['bytes'] and bits in (4,5) and count<=96*56
        palette=struct.unpack_from('<32H',raw);indices=raw[64:64+(count*bits+7)//8];mask=raw[64+len(indices):]
        rgb=bytearray();alpha=bytearray()
        for i in range(count):
            bit=i*bits;v=indices[bit//8]|(indices[bit//8+1]<<8 if bit//8+1<len(indices) else 0)
            rgb+=struct.pack('<H',palette[(v>>(bit%8))&((1<<bits)-1)])
            alpha.append(255*((mask[i//8]>>(i%8))&1))
        assert hashlib.sha256(rgb+alpha).hexdigest()==im['decoded_sha256']
    return m
if __name__=='__main__':
    m=verify();print(f'PASS: {len(m["entries"])} fixed map arrays, 3 scene packs +1 lossless shared fish pack, all CRC/SHA-256 checked, safe 460 KiB cache and 64-color night layer')
