#!/usr/bin/env python

from distutils.core import setup, Extension
import os
import subprocess
from os.path import join, exists

# If you are creating a sdist from the full bllipparser code base, you
# may need the swig and flex packages. The Python packages include the
# outputs of these commands so you can build the Python modules without
# these dependencies.

def run(args):
    cmd = ' '.join(map(str, args))
    print("Running %r" % cmd)
    message = None
    try:
        assert subprocess.check_call(args) == 0
    except OSError, exc:
        if exc.errno == 2:
            message = "Command %r not found." % args[0]
        else:
            message = "OSError: %r" % exc
    except AssertionError, exc:
        message = "Bad exit code from %r" % cmd
    if message:
        raise SystemExit("Error while running command: %s\nBuild failed!" %
                         message)

parser_base = 'first-stage/PARSE/'
parser_wrapper = 'swig/wrapper.C'
parser_wrapper_full = join(parser_base, parser_wrapper)

def is_newer(filename1, filename2):
    """Returns True if filename1 has a newer modification time than
    filename2."""
    return os.stat(filename1).st_mtime > os.stat(filename2).st_mtime

def maybe_run_swig(wrapper_filename, module_name, base_directory,
                   extra_deps=None):
    """Run SWIG if its outputs are missing or out of date."""
    module_filename = 'python/bllipparser/%s.py' % module_name
    swig_filename = join(base_directory, 'swig', 'wrapper.i')
    extra_deps = extra_deps or []
    if exists(wrapper_filename) and exists(module_filename):
        newer = any(is_newer(f, module_filename)
                    for f in [swig_filename] + extra_deps)
        if not newer:
            return

    print('Generating ' + module_name + ' SWIG wrapper files')
    run(['swig', '-python', '-c++', '-module',
         module_name, '-I' + base_directory,
         '-Wall', '-classic', '-outdir', 'python/bllipparser',
         '-o', wrapper_filename, swig_filename])

# generate parser SWIG files if needed
maybe_run_swig(parser_wrapper_full, 'CharniakParser', parser_base,
               extra_deps=[join(parser_base, 'SimpleAPI.' + suffix)
                           for suffix in 'Ch'])

parser_sources = (parser_wrapper, 'Bchart.C', 'BchartSm.C', 'Bst.C',
                  'FBinaryArray.C', 'CntxArray.C', 'ChartBase.C',
                  'ClassRule.C', 'ECArgs.C', 'Edge.C', 'EdgeHeap.C',
                  'ExtPos.C', 'Feat.C', 'Feature.C', 'FeatureTree.C',
                  'Field.C', 'FullHist.C', 'GotIter.C', 'InputTree.C',
                  'Item.C', 'Link.C', 'Params.C', 'ParseStats.C',
                  'SentRep.C', 'ScoreTree.C', 'Term.C', 'TimeIt.C',
                  'UnitRules.C', 'ValHeap.C', 'edgeSubFns.C',
                  'ewDciTokStrm.C', 'extraMain.C', 'fhSubFns.C',
                  'headFinder.C', 'headFinderCh.C', 'utils.C',
                  'MeChart.C', 'Fusion.C')
parser_sources = [join(parser_base, src) for src in parser_sources]

parser_module = Extension('bllipparser._CharniakParser',
                          sources=parser_sources, include_dirs=[parser_base],
                          libraries=['stdc++'])

reranker_base = 'second-stage/programs/features/'
reranker_wrapper = 'swig/wrapper.C'
reranker_wrapper_full = reranker_base + reranker_wrapper
reranker_read_tree = 'read-tree.cc'
reranker_read_tree_full = reranker_base + 'read-tree.cc'

# generate reranker SWIG files if needed
maybe_run_swig(reranker_wrapper_full, 'JohnsonReranker', reranker_base)

# generate reranker tree reader if needed
if not exists(reranker_read_tree_full):
    run(['flex', '-o' + reranker_read_tree_full,
        reranker_read_tree_full.replace('.cc', '.l')])

reranker_sources = [join(reranker_base, src) for src in
                    (reranker_wrapper, 'simple-api.cc', 'heads.cc',
                     reranker_read_tree, 'sym.cc')]

reranker_module = Extension('bllipparser._JohnsonReranker',
                            sources=reranker_sources,
                            extra_compile_args=['-iquote', reranker_base,
                                                '-DSWIGFIX'])

setup(name='bllipparser',
      version='2015.08.18',
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
      package_dir={'bllipparser': 'python/bllipparser'})
