"""Export the six editable tackle descriptions without changing item IDs."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
keys=('bamboo','fiberglass','carbon','red_white','feather','glow')
rows={}
for line in (root/'docs/tackle-descriptions.md').read_text(encoding='utf-8-sig').splitlines():
    cells=[c.strip() for c in line.strip().strip('|').split('|')]
    if cells[0] not in keys:continue
    assert len(cells)==5 and cells[0] not in rows,line
    assert 0<len(cells[2])<=13 and '\n' not in cells[2],line
    rows[cells[0]]=cells[2]
assert set(rows)==set(keys),'Keep all six item IDs.'
out=root/'app/fish_tackle_descriptions.h' if (root/'app').is_dir() else root/'src/fish/fish_tackle_descriptions.h'
code='/* Generated from docs/tackle-descriptions.md. */\n#ifndef FISH_TACKLE_DESCRIPTIONS_H\n#define FISH_TACKLE_DESCRIPTIONS_H\n'
for name,ids in [('fish_rod_descriptions',keys[:3]),('fish_float_descriptions',keys[3:])]:
    code+='static const char *const '+name+'[3]={'+','.join(json.dumps(rows[k],ensure_ascii=False) for k in ids)+'};\n'
out.write_text(code+'#endif\n',encoding='utf-8')
print('Exported six user-authored tackle descriptions.')
