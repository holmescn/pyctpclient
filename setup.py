
from setuptools import setup, find_packages

VERSION = (0, 0, 1, 'alpha', 0)

f = open('README.md', 'r')
LONG_DESCRIPTION = f.read()
f.close()

setup(
    name='pyctpclient',
    version=VERSION,
    description='CTP Python client.',
    long_description=LONG_DESCRIPTION,
    long_description_content_type='text/markdown',
    author='Holmes Conan',
    author_email='holmesconan@gmail.com',
    url='https://github.com/holmescn/ctp.py/',
    license='Apache-2.0',
    packages=find_packages(exclude=['ez_setup', 'tests*']),
    package_data={'ctp': ['templates/*']},
    include_package_data=True,
    entry_points="""
        [console_scripts]
        ctp = ctp.main:main
    """,
)
