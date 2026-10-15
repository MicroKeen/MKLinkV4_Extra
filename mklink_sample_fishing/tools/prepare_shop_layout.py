"""Export portable display-rack anchors; these are also Canvas hit regions."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
path=root/'data/shop_layout.json'
layout=json.loads(path.read_text(encoding='utf-8'))
assert len(layout['items'])==16 and [i['index'] for i in layout['items']]==list(range(16))
rects=[i['rect'] for i in layout['items']]
assert all(0<=x and 50<=y and x+w<=240 and y+h<=179 for x,y,w,h in rects)
output=root/'app/fish_shop_layout.h' if (root/'app').is_dir() else root/'src/fish/fish_shop_layout.h'
output.write_text('/* Generated from data/shop_layout.json. */\n#ifndef FISH_SHOP_LAYOUT_H\n#define FISH_SHOP_LAYOUT_H\n'
 '#include "fish_economy.h"\ntypedef struct {int16_t x,y;uint8_t w,h;} fish_shop_anchor_t;\n'
 'static const fish_shop_anchor_t fish_shop_anchors[FISH_SHOP_COUNT]={\n'+
 ''.join('    {'+','.join(map(str,r))+'},\n' for r in rects)+'};\n#endif\n',encoding='utf-8')
print('Exported 16 display-rack regions from portable JSON.')
