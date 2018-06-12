# coding=utf-8

from distutils.core import setup, Extension

c2j_ext = Extension("combstruct.parser",
                    ["pyext/wrapper.c"],
                    extra_objects=["./libcombstruct2json.a"])

setup(
    ext_modules=[c2j_ext],
    include_dirs=["."],

    name="combstruct.parser",
    version="0.9",
    description=("Lightweight library to parse combstruct grammars, and "
                 "standalone tool to convert them to JSON."),

    author="Jérémie Lumbroso",
    author_email = "lumbroso@cs.princeton.edu",
    url = "https://github.com/jlumbroso/combstruct2json",
    keywords = ['combinatorics'],
    license = 'LGPLv3',
    classifiers = [
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Topic :: Scientific/Engineering :: Mathematics',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Intended Audience :: Developers',
        'Intended Audience :: Education',
        'Intended Audience :: Science/Research',
        'Operating System :: OS Independent',
        'License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)',
        'Development Status :: 5 - Production/Stable'
        ]
)
