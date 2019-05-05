import os
import sys
from setuptools import setup, find_packages, Extension
from src.pyctpclient import __version__

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
        'thostmduserapi',
        'thosttraderapi'
    ]
    package_data = ['*.lib', '*.md']
elif sys.platform == "linux":
    include_dirs = ['/usr/include']
    library_dirs = [
        '/usr/lib/x86_64-linux-gnu',
        os.path.abspath("./lib")
    ]
    libraries = [
        'boost_python3',
        'thostmduserapi',
        'thosttraderapi'
    ]
    package_data = ['lib/*.so']
else:
    raise NotImplemented


ctpclient_ext = Extension('pyctpclient.ctpclient',
                          sources=[
                            'ctpclient_ext/binding.cpp',
                            'ctpclient_ext/ctpclient.cpp',
                            'ctpclient_ext//mdspi.cpp',
                            'ctpclient_ext//traderspi.cpp'
                          ],
                          include_dirs=include_dirs,
                          library_dirs=library_dirs,
                          libraries=libraries)

setup(
    name='pyctpclient',
    platforms=['linux'],
    version=__version__,
    description='CTP python client.',
    long_description = '''
CTP python client wrapped by Boost::python3.
''',
    author='Holmes Conan',
    author_email='holmesconan@gmail.com',
    url='https://github.com/holmescn/pyctpclient/',
    license='Apache-2.0',
    ext_modules=[ctpclient_ext],
    packages=find_packages(),
    keywords="ctp client",
    project_urls={
        "Bug Tracker": "https://github.com/holmescn/pyctpclient/issues",
        "Documentation": "https://github.com/holmescn/pyctpclient",
        "Source Code": "https://github.com/holmescn/pyctpclient",
    },
    package_data={
        '': package_data,
    }
)
