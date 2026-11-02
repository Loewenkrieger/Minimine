"""Apply bounded ClassPad compatibility fixes to a temporary pinned Azur copy."""
from pathlib import Path
import sys

p = Path(sys.argv[1]) / 'azur/src/gint/render.c'
s = p.read_text()
replacements = [
    ('void *azrp_cmdq_command(uint size, int fragment, int count)',
     'void *azrp_cmdq_command(size_t size, int fragment, int count)'),
    ('#include <string.h>', '#include <string.h>\nextern void bench_lcd_begin(void);'),
    ('r61523_win_set(0, DWIDTH - 1, 0, DHEIGHT - 1);', 'bench_lcd_begin();'),
    ('uint32_t cmd = azrp_cmdq_index.buf[i];',
     'uint32_t cmd = i < azrp_cmdq_index.cursor ? azrp_cmdq_index.buf[i] : 0xffffffff;'),
    ('cmd = azrp_cmdq_index.buf[++i];',
     '++i; cmd = i < azrp_cmdq_index.cursor ? azrp_cmdq_index.buf[i] : 0xffffffff;'),
    ('int size = azrp_width * azrp_frag_height * 2;',
     'int rows = azrp_height - frag * azrp_frag_height;\n'
     '            if(rows > azrp_frag_height) rows = azrp_frag_height;\n'
     '            int size = azrp_width * rows * 2;'),
    ('if(azrp_scale == 1)\n            azrp_r61523_fragment_x1(azrp_frag, DWIDTH * azrp_frag_height);\n'
     '        else if(azrp_scale == 2)\n            azrp_r61523_fragment_x2(azrp_frag, azrp_width, azrp_frag_height);',
     'int rows = azrp_height - frag * azrp_frag_height;\n'
     '        if(rows > azrp_frag_height) rows = azrp_frag_height;\n'
     '        if(azrp_scale == 1)\n            azrp_r61523_fragment_x1(azrp_frag, DWIDTH * rows);\n'
     '        else if(azrp_scale == 2)\n            azrp_r61523_fragment_x2(azrp_frag, azrp_width, rows);'),
]
for before, after in replacements:
    if s.count(before) != 1:
        raise SystemExit('Unexpected Azur source; refusing compatibility patch: ' + before[:70])
    s = s.replace(before, after)
p.write_text(s)
