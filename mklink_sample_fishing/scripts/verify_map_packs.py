"""Verify deployed SD packs against firmware addresses and compiled identities."""
from pathlib import Path
import hashlib,json,re,struct,zlib
D=Path(__file__).resolve().parents[1]
def verify():
    root=D/'sdcard/FishingGame_Resources_DO_NOT_DELETE'
    m=json.loads((root/'manifest.json').read_text(encoding='utf-8'))
    code=(D/'src/fish/fish_asset_layout.h').read_text(encoding='utf-8')
    def constant(name):return int(re.search(r'#define '+name+r' (0x[0-9a-f]+|\d+)u',code)[1],0)
    assert m['payload_bytes']==constant('FISH_ASSET_PAYLOAD_BYTES')<=0x72000
    assert int(m['layout_crc'],16)==constant('FISH_ASSET_LAYOUT_CRC')
    end=0
    for e in m['entries']:
        assert e['offset']%4==0 and e['offset']>=end
        assert e['offset']==constant('FISH_OFFSET_'+e['symbol'])
        end=e['offset']+e['bytes'];assert end<=m['payload_bytes']
    night=constant('FISH_ASSET_NIGHT_OFFSET');sky=constant('FISH_ASSET_SKYLINE_OFFSET')
    assert end<=sky and sky+240==night and night+28928==m['payload_bytes']
    for row in m['maps']:
        b=(root/row['file']).read_bytes();assert len(b)==row['bytes']==64+m['payload_bytes']
        assert hashlib.sha256(b).hexdigest()==row['sha256']
        magic,version,map_id,layout,n,crc,hcrc=struct.unpack_from('<4s6I',b)
        assert magic==b'FGRP' and version==1 and map_id==row['id']
        assert layout==constant('FISH_ASSET_LAYOUT_CRC') and n==m['payload_bytes']
        assert crc==zlib.crc32(b[64:])==constant(f'FISH_ASSET_MAP{map_id}_CRC')
        assert hcrc==zlib.crc32(b[:24]) and not any(b[28:64])
        assert max(b[64+sky:64+sky+240])<=120
        layer=b[64+night:64+night+28928]
        if map_id:assert max(layer[128:])<64 and any(layer[:128])
        else:assert not any(layer)
    return m
if __name__=='__main__':
    m=verify();print(f'PASS: {len(m["entries"])} fixed map arrays, 2 CRC/SHA-256 checked SD packs, safe 460 KiB cache and 64-color night layer')
