"""Import editable marine price bases without changing existing catch values."""
from pathlib import Path
import json,re
R=Path(__file__).resolve().parents[1]
doc=R/'docs/coast-species-review.md';text=doc.read_text(encoding='utf-8');prices={}
for line in text.splitlines():
    cells=[c.strip() for c in line.strip().strip('|').split('|')]
    if len(cells)!=7 or not cells[0].isdigit():continue
    index=int(cells[0]);assert index not in prices and 25<=index<=49,line
    value=int(cells[5]);assert 1<=value<=65535,line
    prices[index]=value
assert set(prices)==set(range(25,50)),'Keep all25 marine IDs and the price-base column.'
prices[44]=prices[25]*10;assert prices[44]<=65535
cat=json.loads((R/'data/species.json').read_text(encoding='utf-8'))
marine=json.loads((R/'data/marine_fish.json').read_text(encoding='utf-8'))
for s in cat['species'][25:]:s['price_base_g']=prices[s['index']]
for s in marine['species']:s['price']=prices[s['index']]
for name,data in [('species.json',cat),('marine_fish.json',marine)]:
    (R/'data'/name).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
p=R/('app/fish_economy.c' if (R/'app').exists() else 'src/fish/fish_economy.c');code=p.read_text(encoding='utf-8')
match=re.search(r'(const uint16_t fish_base_prices\[FISH_SPECIES_COUNT\]=\{)(.*?)(\};)',code,re.S);assert match
values=[int(x) for x in re.findall(r'\d+',match[2])];assert len(values)==50
for i,v in prices.items():values[i]=v
code=code[:match.start(2)]+','.join(map(str,values))+code[match.end(2):];p.write_text(code,encoding='utf-8')
lines=[]
for line in text.splitlines():
    cells=[c.strip() for c in line.strip().strip('|').split('|')]
    if len(cells)==7 and cells[0]=='44':cells[5]=str(prices[44]);line='| '+' | '.join(cells)+' |'
    lines.append(line)
doc.write_text('\n'.join(lines)+'\n',encoding='utf-8')
print('Imported25 marine price bases from Markdown; peacock inherits parent x10. Existing cargo prices remain historical.')
