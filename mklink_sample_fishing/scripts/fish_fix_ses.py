"""Repair SDK v1.11's duplicate output-format attribute in this app only."""
from pathlib import Path
import sys
import re
import os
import html
import xml.etree.ElementTree as ET

project=Path(sys.argv[1])
source=project.read_text(encoding='utf-8')
if source.count('linker_output_format="hex"')==1 and source.count('linker_output_format="bin"')==1:
    source=source.replace('      linker_output_format="hex"\n','')

# The SDK generator drops CMake's per-source -Os. Apply the matching installed
# SES "Level 2 for size" setting only to infrequent save/filesystem paths.
def compact_save_file(match):
    path=match[2].replace('\\','/')
    compact=('/src/fish/crypto/source/' in path or '/lcd/fatfs/' in path or
             '/lcd/adapt/spi_sd_adapt.c' in path or
             path.rsplit('/',1)[-1] in {'fish_game.c','fish_save.c','fish_save_crypto.c','fish_save_key.c','fish_save_hpm.c','fish_resources.c','fish_resources_hpm.c','fish_fs_hpm.c'})
    body=match[3]
    if compact and 'gcc_optimization_level=' not in body:
        body=body.replace('<configuration Name="Common"','<configuration Name="Common" gcc_optimization_level="Level 2 for size"',1)
    return match[1]+body+match[4]
source=re.sub(r'(<file file_name="([^"]+)"[^>]*>)(.*?)(</file>)',compact_save_file,source,flags=re.S)

# Match the standard fire demo: SES also exports a usable application UF2.
demo=Path(sys.argv[2]).resolve()
python=Path(sys.argv[3]).as_posix()
script=os.path.relpath(demo/'scripts/uf2conv.py',project.parent).replace('\\','/')
output=demo/'MicroLink.uf2'
destination=os.path.relpath(output,project.parent).replace('\\','/')
command='"'+python+'" "$(ProjectDir)/'+script+'" "$(OutDir)/MicroLink.bin" -f 0x0A4D5048 -b 0x80023000 -c -o "$(ProjectDir)/'+destination+'"'
encoded=html.escape(command,quote=True)
source,count=re.subn(r'post_build_command="([^"]*)"',lambda m:'post_build_command="'+m[1]+' &amp;&amp; '+encoded+'"',source)
if count!=1:raise RuntimeError('Expected one SES post-build hook')
tree=ET.fromstring(source) # Do not silently emit an invalid IDE project.
files=[str((project.parent/item.attrib['file_name'].replace('\\','/')).resolve()).casefold()
       for item in tree.iter('file')]
if len(files)!=len(set(files)):
    raise RuntimeError('Duplicate source entries in the HPM-generated SES project')
project.write_text(source,encoding='utf-8')
