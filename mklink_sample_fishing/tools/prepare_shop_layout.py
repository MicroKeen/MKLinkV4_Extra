"""Export fixed-pixel geometry for the scrolling shop list."""
from pathlib import Path
import json
root=Path(__file__).resolve().parents[1]
path=root/'data/shop_layout.json'
layout=json.loads(path.read_text(encoding='utf-8'))['list']
assert layout['rows']==5 and layout['x']+layout['w']<=134
assert layout['y']+layout['rows']*layout['row_h']<=165
output=root/'app/fish_shop_layout.h' if (root/'app').is_dir() else root/'src/fish/fish_shop_layout.h'
output.write_text('/* Generated from data/shop_layout.json. */\n#ifndef FISH_SHOP_LAYOUT_H\n#define FISH_SHOP_LAYOUT_H\n'
 'enum {\n'+''.join('    FISH_SHOP_LIST_'+k.upper()+'='+str(v)+',\n' for k,v in layout.items())+'};\n#endif\n',encoding='utf-8')
print('Exported five-row shop list geometry.')
