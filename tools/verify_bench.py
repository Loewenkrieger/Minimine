from pathlib import Path
import json
import hashlib
import sys
from verify_hh3 import inspect
from bench_source_id import source_id

artifact, root = Path(sys.argv[1]), Path(sys.argv[2])
report = inspect(artifact, expected_names=('Minimine Bench',))
report['source_id'] = source_id(root)
report['azur_commit'] = (root / 'bench/azur-pin.txt').read_text().splitlines()[1]
report['reference_renderer_sha256'] = hashlib.sha256((root / 'benchmarks/1.0.0/render-reference.c').read_bytes()).hexdigest()
assert report['reference_renderer_sha256'] == '3a103f4d480964582a63e9cebcc987d4d927c1880e29ca00a888925c7dc0cc10'
report['hardware_tested'] = False
report['runtime_buffers'] = {
    'azur_heap_queue_bytes': 65536,
    'azur_fragment_alias': {'start': '0xe500e020', 'maximum_bytes': 10240},
    'xram_rows': {'start': '0xe5007020', 'bytes': 5120},
    'yram_rows': {'start': '0xe5017020', 'bytes': 5120},
    'note': 'On-chip areas are shared sequentially; ELF region counts only describe load segments.'
}
assert report['regions']['ilram']['used_bytes'] <= 4096
artifact.with_name('build-info.json').write_text(json.dumps(report, indent=2) + '\n')
print('Verified benchmark:', report['file_bytes'], 'bytes; SHA256', report['sha256'])
for name, region in report['regions'].items():
    if region['used_bytes']:
        print(name, region['used_bytes'], '/', region['capacity_bytes'], 'bytes')
