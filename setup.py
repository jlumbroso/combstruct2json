# coding=utf-8

from setuptools import setup, Extension

# Compile the wrapper by statically linking the library.
c2j_ext_link = Extension("combstruct2json",
                         ["pyext/wrapper.c"],
                         extra_objects=["./libcombstruct2json.a"])

# Compile the wrapper by recompiling everything.
c2j_ext = Extension("combstruct2json",
                    ["src/pywrapper.c", "parser.tab.c", "lex.yy.c",
                    "src/absyn.c", "src/node.c"],

                    extra_compile_args=[
                        "-Wno-strict-prototypes",
                        "-Wno-unused-function",
                        "-Wno-unneeded-internal-declaration"])

setup(
    ext_modules=[c2j_ext],
    include_dirs=["."],

    name="combstruct2json",
    version="0.95",
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
