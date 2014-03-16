Retraining the reranker
-----------------------
If you're experimenting with new reranker features or want to build a
reranker for a different treebank, you will want to retrain the
reranker.

Retraining the reranker takes a considerable amount of time, disk space
and RAM. At Brown we use a dual Opteron machine with 16Gb RAM, and it
takes around two days (editors note: this was written in 2006, when
these numbers were a little more impressive...). You should be able to
do it with only 8Gb RAM, and maybe even with 4Gb RAM with an
appropriately tweaked kernel (e.g., ``sysctl overcommit\_memory``, and a
so-called 4Gb/4Gb split if you're using a 32-bit OS).

The time and memory you need depend on the features that the reranker
extracts and the size of the n-best tree training and development data.
You can change the features that are extracted by changing
``second-stage/programs/features/features.h``, and you can reduce the
size of the n-best tree data by reducing ``NPARSES`` in the ``Makefile``
from 50 to, say, 25.

You will need to edit the ``Makefile`` in order to retrain the reranker.
First, you need to set the variable ``PENNWSJTREEBANK`` in the
``Makefile`` to the directory that holds your version of the Penn WSJ
Treebank. For example::

    PENNWSJTREEBANK=/usr/local/data/Penn3/parsed/mrg/wsj/

If you're using cvlm-lbfgs as your estimator (the default), you'll also
need the Boost C++ and the
`libLBFGS <http://www.chokkan.org/software/liblbfgs/>`_ library in order
to retrain the reranker. libLBFGS is available at under the MIT license.
In Ubuntu, you'll need the ``liblbfgs-dev`` package::

    shell> sudo apt-get install liblbfgs-dev

For older versions of Ubuntu, you may need to install a PPA to get
liblbfgs-dev::

    shell> sudo add-apt-repository --yes ppa:ktm5j/uva-cs-ppa
    shell> sudo apt-get update

`Boost <http://www.boost.org/>`_ can be obtained with the libboost-dev
package in Ubuntu::

    shell> sudo apt-get install libboost-dev

While many modern Linux distributions come with the Boost C++ libraries
pre-installed, if the Boost C++ libraries are not included in your
standard libraries and headers, you will need to install them and add an
include file specification for them in your ``GCCFLAGS``. For example,
if you have installed the Boost C++ libraries in ``/home/mj/C++/boost``,
then your ``$GCCFLAGS`` environment variable should be something like::

    shell> setenv GCCFLAGS "-march=pentium4 -mfpmath=sse -msse2 -mmmx -I /home/mj/C++/boost"

or::

    shell> setenv GCCFLAGS "-march=opteron -m64 -I /home/mj/C++/boost"

Once this is set up, you retrain the reranker as follows::

    shell> make reranker 
    shell> make nbesttrain
    shell> make eval-reranker

The script ``train-eval-reranker.sh`` does all of this.

The ``reranker`` goal builds all of the programs, ``nbesttrain``
constructs the 20 folds of n-best parses required for training, and
``eval-reranker`` extracts features, estimates their weights and
evaluates the reranker's performance on the development data (dev) and
the two test data sets (test1 and test2).

If you have a parallel processor, you can run 2 (or more) jobs in
parallel by running::

    shell> make -j 2 nbesttrain

Currently this only helps for ``nbesttrain`` (but this is the slowest
step, so maybe this is not so bad).

The ``Makefile`` contains a number of variables that control how the
training process works. The most important of these is the ``VERSION``
variable. You should do all of your experiments with
``VERSION=nonfinal``, and only run with ``VERSION=final`` once to
produce results for publication.

If ``VERSION`` is ``nonfinal`` then the reranker trains on WSJ PTB
sections 2-19, sections 20-21 are used for development, section 22 is
used as test1 and section 24 is used as test2 (this approximately
replicates the Collins 2000 setup).

If ``VERSION`` is ``final`` then the reranker trains on WSJ PTB sections
2-21, section 24 is used for development, section 22 is used as test1
and section 23 is used as test2.

The ``Makefile`` also contains variables you may want to change, such as
``NBEST``, which specfies how many parses per sentence are extracted
from each sentence, and ``NFOLDS``, which specifies how many folds are
created.

If you decide to experiment with new features or new feature weight
estimators, take a close look at the ``Makefile``. If you change the
features please also change ``FEATURESNICKNAME``; this way your new
features won't over-write our existing ones. Similarly, if you change
the feature weight estimator please pick a new ``ESTIMATORNICKNAME`` and
if you change the n-best parser please pick a new
``NBESTPARSERNICKNAME``; this way you new n-best parses or feature
weights won't over-write the existing ones.

To get rid of (many of) the object files produced in compilation, run::

    shell> make clean

Training, especially constructing the 20 folds of n-best parses,
produces a lot of temporary files which you can remove if you want to.
To remove the temporary files used to construct the 20 fold n-best
parses, run::

    shell> make nbesttrain-clean

All of the information needed by the reranker is in
``second-stage/models``. To remove everything except the information
needed for running the reranking parser, run::

    shell> make train-clean

To clean up everything, including the data needed for running the
reranking parser, run::

    shell> make real-clean
