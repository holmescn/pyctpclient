"""Simple python CTP client.
"""
import re
import os
import sys
from setuptools import setup, find_packages, Extension

if sys.platform == 'win32':
    extra_compile_args = [
        "/MP", "/std:c++17",                        # standard
        "/O2", "/Ob2", "/Oi", "/Ot", "/Oy", "/GL",  # Optimization
        "/wd4819"                                   # 936 code page
    ]
    extra_link_args = []
elif sys.platform == "linux":
    extra_compile_args = [
        "-std=c++17",  # standard
        "-O3",         # Optimization
        "-Wno-delete-incomplete", "-Wno-sign-compare",
        '-Wall', '-Wextra', '-Wno-unknown-pragmas', '-Wno-unused-parameter'
    ]
    extra_link_args = ["-lstdc++"]
else:
    raise ValueError('Platform %s is not supportted.' % sys.platform)


with open('src/pyctpclient/__init__.py', 'r') as fp:
    for line in fp.readlines():
        matched = re.match(r'__version__\s*=\s*"(.*)"', line)
        if matched:
            version = matched.group(1)


ctpclient_ext = Extension(
    'pyctpclient.ctpclient',
    sources=[
        'src/ctpclient_ext/binding.cpp',
        'src/ctpclient_ext/ctpclient.cpp',
        'src/ctpclient_ext//mdspi.cpp',
        'src/ctpclient_ext//traderspi.cpp'
    ],
    include_dirs=[
        os.path.abspath('./pybind11/include'),
    ],
    library_dirs=[
        os.path.abspath('./src/pyctpclient/lib')
    ],
    libraries=["thostmduserapi", "thosttraderapi"],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    runtime_library_dirs=["$ORIGIN"],
    language="cpp"
)

setup(
    name='pyctpclient',
    platforms=['linux', 'win32', 'win64'],
    version=version,
    description='CTP python client.',
    long_description = __doc__,
    author='Holmes Conan',
    author_email='holmesconan@gmail.com',
    url='https://github.com/holmescn/pyctpclient/',
    license='Apache-2.0',
    ext_modules=[ctpclient_ext],
    packages=find_packages("src"),
    package_dir={"": "src"},
    keywords="ctp quant quantitative investment trading algotrading",
    project_urls={
        "Bug Tracker": "https://github.com/holmescn/pyctpclient/issues",
        "Documentation": "https://github.com/holmescn/pyctpclient",
        "Source Code": "https://github.com/holmescn/pyctpclient",
    },
    include_package_data=True,
        package_data={"": [
        "*.dll",
        "*.so",
        "*.pyd",
    ]},
    classifiers=[
        "Development Status :: 3 - Alpha",
        # "Development Status :: 4 - Beta",
        # "Development Status :: 5 - Production/Stable",
        "Operating System :: Microsoft :: Windows :: Windows 7",
        "Operating System :: Microsoft :: Windows :: Windows 8",
        "Operating System :: Microsoft :: Windows :: Windows 10",
        "Operating System :: Microsoft :: Windows :: Windows Server 2008",
        "Operating System :: Microsoft :: Windows :: Windows Server 2012",
        "Operating System :: Microsoft :: Windows :: Windows Server 2012",
        "Operating System :: POSIX :: Linux"
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Topic :: Office/Business :: Financial :: Investment",
        "Programming Language :: Python :: Implementation :: CPython",
        "License :: OSI Approved :: MIT License",
        "Natural Language :: Chinese (Simplified)"
    ],
)
