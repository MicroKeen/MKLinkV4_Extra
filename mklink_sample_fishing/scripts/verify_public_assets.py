"""Check the published demo contains only runtime C artwork and its licenses."""
from pathlib import Path
import json,re
demo=Path(__file__).resolve().parents[1]
assert not any(p.is_file() for p in (demo/'assets').rglob('*')),'Source/export assets must stay local.'
for path in (demo/'docs').rglob('*'):
    assert path.suffix.lower() not in {'.png','.bmp','.jpg','.jpeg','.svg','.gif','.webp'},path
report=json.loads((demo/'docs/build-validation.json').read_text(encoding='utf-8'))
sections={s['name'] for s in report['gnu']['sections']}
from verify_map_packs import verify
packs=verify();external={e['symbol'] for e in packs['entries']}
count=0;shared=0;seen=set()
for path in (demo/'src/fish/assets').rglob('*.c'):
    code=path.read_text(encoding='utf-8')
    for symbol in re.findall(r'const uint(?:8|16)_t\s+(c_bmp\w+)\s*\[',code):
        seen.add(symbol)
        if symbol in external:
            assert 'SD_PACK_ARRAY' in code and 'arm2d.asset.'+symbol not in sections
            count+=1;continue
        shared+=1
        assert ('arm2d.asset.'+symbol in sections or 'arm2d.asset.FONT.'+symbol in sections),(path.name,symbol,'not linked')
        count+=1
assert external<=seen
assert (demo/'src/fish/assets/OFL.txt').is_file()
assert (demo/'src/fish/assets/fish_sky_horizon.c').is_file()
print(f'PASS: {shared} shared ROM / {len(external)} SD bitmap arrays; no source artwork, TTFs or previews; font license retained.')
