"""Export user-editable Markdown shop dialogue to a small runtime C table."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
keys=('greeting_empty','greeting_fish','greeting_giant','sold','bought','poor','empty','owned','full')
rows={}
for line in (root/'docs/merchant-dialogue.md').read_text(encoding='utf-8').splitlines():
    cells=[c.strip() for c in line.strip().strip('|').split('|')]
    if cells[0] not in keys:continue
    assert len(cells)==4 and cells[0] not in rows,line
    key,pose,a,b=cells;assert 0<=int(pose)<4 and all(0<len(s)<=16 for s in (a,b)),line
    rows[key]=(int(pose),a,b)
assert set(rows)==set(keys),'Keep all nine scene IDs.'
out=root/'app/fish_merchant_dialogue_data.h' if (root/'app').is_dir() else root/'src/fish/fish_merchant_dialogue_data.h'
out.write_text('/* Generated from docs/merchant-dialogue.md. */\n#ifndef FISH_MERCHANT_DIALOGUE_DATA_H\n#define FISH_MERCHANT_DIALOGUE_DATA_H\n'
    'typedef struct {unsigned char pose;const char *lines[2];} fish_merchant_dialogue_t;\n'
    'static const fish_merchant_dialogue_t fish_merchant_dialogue[9]={\n'+
    ''.join('    {'+str(rows[k][0])+',{'+','.join(json.dumps(s,ensure_ascii=False) for s in rows[k][1:])+'}},\n' for k in keys)+'};\n#endif\n',encoding='utf-8')
print('Exported 9 contexts, 18 lines and four expression mappings; glyph update required for new characters.')
