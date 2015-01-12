#!/usr/bin/env python

from distutils.core import setup, Extension
import os, subprocess
from os.path import join, exists

# If you are creating a sdist from the full bllipparser code base, you
# may need the swig and flex packages. The Python packages include the
# outputs of these commands so you can build the Python modules without
# these dependencies.

def run(args):
    print "Running %r" % ' '.join(map(str, args))
    subprocess.check_call(args)

parser_base = 'first-stage/PARSE/'
parser_wrapper = 'parser_wrapper.C'
parser_wrapper_full = join(parser_base, parser_wrapper)

def is_newer(filename1, filename2):
    """Returns True if filename1 has a newer modification time than
    filename2."""
    return os.stat(filename1).st_mtime > os.stat(filename2).st_mtime

def maybe_run_swig(wrapper_filename, module_name, base_directory):
    """Run SWIG if its outputs are missing or out of date."""
    module_filename = 'python/bllipparser/%s.py' % module_name
    swig_filename = join(base_directory, 'swig', 'wrapper.i')
    if exists(wrapper_filename) and exists(module_filename):
        if not is_newer(swig_filename, module_filename):
            return

    print 'Generating', module_name, 'SWIG wrapper files'
    run(['swig', '-python', '-c++', '-module',
        module_name, '-Wall', '-classic', '-outdir', 'python/bllipparser',
        '-o', wrapper_filename, swig_filename])

# generate parser SWIG files if needed
maybe_run_swig(parser_wrapper_full, 'CharniakParser', parser_base)

parser_sources = [join(parser_base, src) for src in
    (parser_wrapper, 'Bchart.C', 'BchartSm.C', 'Bst.C', 'FBinaryArray.C',
     'CntxArray.C', 'ChartBase.C', 'ClassRule.C', 'ECArgs.C', 'Edge.C',
     'EdgeHeap.C', 'ExtPos.C', 'Feat.C', 'Feature.C', 'FeatureTree.C',
     'Field.C', 'FullHist.C', 'GotIter.C', 'InputTree.C', 'Item.C',
     'Link.C', 'Params.C', 'ParseStats.C', 'SentRep.C', 'ScoreTree.C',
     'Term.C', 'TimeIt.C', 'UnitRules.C', 'ValHeap.C', 'edgeSubFns.C',
     'ewDciTokStrm.C', 'extraMain.C', 'fhSubFns.C', 'headFinder.C',
     'headFinderCh.C', 'utils.C', 'MeChart.C')
]

parser_module = Extension('bllipparser._CharniakParser',
    sources=parser_sources, include_dirs=[parser_base],
    libraries=['stdc++'])

reranker_base = 'second-stage/programs/features/'
reranker_wrapper = 'reranker_wrapper.C'
reranker_wrapper_full = reranker_base + reranker_wrapper
reranker_read_tree = 'read-tree.cc'
reranker_read_tree_full = reranker_base + 'read-tree.cc'

# generate reranker SWIG files if needed
maybe_run_swig(reranker_wrapper_full, 'JohnsonReranker', reranker_base)

# generate reranker tree reader if needed
if not exists(reranker_read_tree_full):
    run(['flex', '-o' + reranker_read_tree_full,
        reranker_read_tree_full.replace('.cc', '.l')])

reranker_sources = [reranker_base + src for src in 
    (reranker_wrapper, 'simple-api.cc', 'heads.cc', reranker_read_tree,
     'sym.cc')]

# what's with the -O0? well, using even the lowest levels of optimization
# (gcc -O1) causes one symbol which we wrap with SWIG to be inlined and
# disappear in _JohnsonReranker.so which causes an ImportError.  this will
# hopefully be addressed in the near future.
reranker_module = Extension('bllipparser._JohnsonReranker',
    sources=reranker_sources,
    extra_compile_args=['-iquote', reranker_base, '-O0'])

setup(name='bllipparser',
    version='2015.01.11',
    description='Python bindings for the BLLIP natural language parser',
    long_description=file('README-python.rst').read(),
    author='Eugene Charniak, Mark Johnson, David McClosky, many others',
    maintainer='David McClosky',
    maintainer_email='notsoweird+pybllipparser@gmail.com',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: Apache Software License',
        'Natural Language :: English',
        'Operating System :: POSIX',
        'Topic :: Scientific/Engineering :: Artificial Intelligence',
    ],
    url='http://github.com/BLLIP/bllip-parser',
    license='Apache 2.0',
    platforms=['POSIX'],
    ext_modules=[parser_module, reranker_module],
    packages=['bllipparser'],
    package_dir={'bllipparser' : 'python/bllipparser'},
)
