import re
import os
import sys
from setuptools import setup, find_packages, Extension

if sys.platform == 'win32':
    VCPKG_DIR = os.getenv("VCPKG_DIR")
    VCPKG_INCLUDE_DIR = os.path.join(VCPKG_DIR, "installed", "x64-windows", "include")
    VCPKG_LIBARAY_DIR = os.path.join(VCPKG_DIR, "installed", "x64-windows", "lib")
    include_dirs = [VCPKG_INCLUDE_DIR]
    library_dirs = [
        VCPKG_LIBARAY_DIR,
        os.path.abspath(".\\lib"),
    ]
    libraries = [
        'boost_python37-vc140-mt',
    ]
    extra_compile_args = []
elif sys.platform == "linux":
    include_dirs = [
        os.path.abspath("./pybind11/include")
    ]
    library_dirs = [
        os.path.abspath("./lib")
    ]
    libraries = [
    ]
    extra_compile_args = ['-std=c++14', '-Wall', '-Wextra', '-Wno-unknown-pragmas', '-Wno-unused-parameter']
else:
    raise ValueError('Platform %s is not supportted.' % sys.platform)

libraries.extend([
    'boost_system',
    'boost_filesystem',
    'thostmduserapi',
    'thosttraderapi'
])


with open('src/pyctpclient/__init__.py', 'r') as fp:
    for line in fp.readlines():
        matched = re.match(r'__version__\s*=\s*"(.*)"', line)
        if matched:
            version = matched.group(1)


ctpclient_ext = Extension('pyctpclient._ctpclient',
                          sources=[
                            'src/ctpclient_ext/binding.cpp',
                            'src/ctpclient_ext/ctpclient.cpp',
                            'src/ctpclient_ext//mdspi.cpp',
                            'src/ctpclient_ext//traderspi.cpp'
                          ],
                          extra_compile_args=extra_compile_args,
                          include_dirs=include_dirs,
                          library_dirs=library_dirs,
                          libraries=libraries)

setup(
    name='pyctpclient',
    platforms=['linux'],
    version=version,
    description='CTP python client.',
    long_description = '''
CTP python client wrapped by Boost::python3.
''',
    author='Holmes Conan',
    author_email='holmesconan@gmail.com',
    url='https://github.com/holmescn/pyctpclient/',
    license='Apache-2.0',
    ext_modules=[ctpclient_ext],
    packages=find_packages("src"),
    package_dir={"": "src"},
    keywords="ctp client",
    project_urls={
        "Bug Tracker": "https://github.com/holmescn/pyctpclient/issues",
        "Documentation": "https://github.com/holmescn/pyctpclient",
        "Source Code": "https://github.com/holmescn/pyctpclient",
    },
    include_package_data=True
)
