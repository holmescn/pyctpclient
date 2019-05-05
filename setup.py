
from setuptools import setup, find_packages, Extension

f = open('README.md', 'r')
LONG_DESCRIPTION = f.read()
f.close()

ctpclient_ext = Extension('pkg.ctpclient',
                          sources=[
                            'src/ctpclient_ext/binding.cc',
                            'src/ctpclient_ext/ctpclient.cc',
                            'src/ctpclient_ext//mdspi.cc',
                            'src/ctpclient_ext//traderspi.cc'
                          ],
                          include_dirs=['/usr/local/include'],
                          library_dirs=['/usr/local/lib/boost'],
                          runtime_library_dirs=['/usr/local/lib/boost'],
                          libraries=['boost_python'])

setup(
    name='pyctpclient',
    version='0.0.1a0',
    description='CTP Python client.',
    long_description=LONG_DESCRIPTION,
    author='Holmes Conan',
    author_email='holmesconan@gmail.com',
    url='https://github.com/holmescn/pyctpclient/',
    license='Apache-2.0',
    package_dir={'': 'src'},
    packages=['pyctpclient'],
    ext_modules=[ctpclient_ext],
)
