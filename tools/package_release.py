"""Package already-built English DOL files using only the standard library."""
from pathlib import Path
import hashlib
import json
import shutil
import struct
import zipfile

root = Path(__file__).resolve().parents[1]
dist = root / 'dist'
dist.mkdir(exist_ok=True)
targets = [
    ('wii', root / 'releases/wii/apps/bernas-career/boot.dol'),
    ('gamecube', root / 'releases/gamecube/bernas-career.dol'),
]
hashes = []
for platform, output in targets:
    source = root / f'native/bernas-career-{platform}.dol'
    if not source.is_file():
        raise SystemExit(f'Build {platform} first: make -C native PLATFORM={platform}')
    data = source.read_bytes()
    offsets = struct.unpack_from('>18I', data, 0)
    addresses = struct.unpack_from('>18I', data, 0x48)
    sizes = struct.unpack_from('>18I', data, 0x90)
    entry = struct.unpack_from('>I', data, 0xe0)[0]
    if not all(o >= 0x100 and o + n <= len(data) for o, n in zip(offsets, sizes) if n):
        raise SystemExit(f'Invalid DOL section: {source.name}')
    if not any(a <= entry < a + n for a, n in zip(addresses[:7], sizes[:7]) if n):
        raise SystemExit(f'Invalid DOL entry point: {source.name}')
    output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, output)
    hashes.append(hashlib.sha256(data).hexdigest() + '  ' + output.relative_to(root).as_posix())

wii = root / 'releases/wii/apps/bernas-career'
shutil.copy2(root / 'assets/icon.png', wii / 'icon.png')
(wii / 'meta.xml').write_text('''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>Bernas Career - English</name>
  <coder>Bernas</coder><version>1.0-en</version><release_date>20261111000000</release_date>
  <short_description>Your game. Your story.</short_description>
  <long_description>A football player career simulator. Train, simulate matches, improve and earn transfers. Fictional league: 8 clubs, 14 matchdays. D-pad: navigate; A: confirm; B: career; +: save; -: load; HOME: exit. FAT save: bernas-career.sav. Ordinary Memory Cards are not supported. English edition.</long_description>
</app>
''', encoding='utf-8')
(root / 'SHA256SUMS.txt').write_text('\n'.join(hashes) + '\n', encoding='utf-8')

def is_source(path):
    rel = path.relative_to(root)
    return (path.is_file() and not any(part.startswith('build-') or part in ('.git', '__pycache__', 'dist') for part in rel.parts)
            and not (rel.parts[0] == 'native' and path.suffix in ('.dol', '.elf', '.map', '.o', '.d')))

console_zip = root / 'web/bernas-career-wii-gamecube.zip'
with zipfile.ZipFile(console_zip, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for path in sorted(root.rglob('*')):
        rel = path.relative_to(root)
        if is_source(path) and (rel.parts[0] in ('native', 'releases') or rel.as_posix() in ('README.md', 'LICENSE', 'SHA256SUMS.txt')):
            archive.write(path, rel)

output = dist / 'Bernas-Career-GitHub-English.zip'
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for path in sorted(root.rglob('*')):
        if is_source(path):
            archive.write(path, path.relative_to(root))
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
    assert 'README.md' in archive.namelist()
    assert not any(name.startswith(('.git/', '.openai/')) for name in archive.namelist())
    print(json.dumps({'archive': str(output), 'files': len(archive.namelist()),
                      'bytes': output.stat().st_size, 'sha256': hashes}, indent=2))
