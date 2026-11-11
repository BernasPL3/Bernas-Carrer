"""Run the C++ engine tests and require exact JavaScript save parity."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shutil.which(os.environ.get('CXX', 'g++'))
node = shutil.which('node')
if not compiler or not node:
    raise SystemExit('Install a C++17 host compiler and Node.js. CXX can name the compiler executable.')

with tempfile.TemporaryDirectory(prefix='bernas-career-tests-') as directory:
    output = Path(directory) / ('engine-test.exe' if os.name == 'nt' else 'engine-test')
    subprocess.run([compiler, '-std=c++17', '-O2', '-I', str(root / 'native/include'),
                    str(root / 'tests/engine_test.cpp'), '-o', str(output)], check=True)
    subprocess.run([str(output)], check=True)
    subprocess.run([node, str(root / 'tests/engine_test.js'), str(output)], check=True)
