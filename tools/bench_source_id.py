from pathlib import Path
import hashlib
import sys

def source_id(root):
    key = lambda p: p.relative_to(root).as_posix()
    files = sorted((p for folder in ('src', 'bench') for p in (root / folder).rglob('*') if p.is_file()), key=key)
    files += sorted((p for p in (root / 'tools').glob('*bench*') if p.is_file()), key=key)
    frozen = root / 'benchmarks/1.0.0/render-reference.c'
    if frozen.exists():
        files.append(frozen)
    h = hashlib.sha256()
    for p in files:
        h.update(p.relative_to(root).as_posix().encode() + b'\0' + p.read_bytes() + b'\0')
    return h.hexdigest()

if __name__ == '__main__':
    print(source_id(Path(sys.argv[1])))
