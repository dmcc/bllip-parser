/* lmdata.h
 *
 * Mark Johnson, 11th April 2005
 *
 * Data structures for weighted log-likelihood estimator from partially
 * visible data.
 *
 * Special cases features with count 1.
 */

#ifndef DATA_H
#define DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <float.h>
#include <stdio.h>

/* typedef size_t feature_type; */
typedef unsigned int size_type;
typedef size_type feature_type;
#define FEATURE_FORMAT "%u"

typedef double Float;
#define FLOAT_FORMAT "%lg"
#define FLOAT_MAX DBL_MAX

typedef float DataFloat;
#define DATAFLOAT_FORMAT "%g"

#define DATAFLOAT_MAX FLT_MAX
#define DATAFLOAT_EPS FLT_EPSILON

typedef struct {
  Float Pyx_factor;                    /* Pyx \propto factor ^ f-score */
  unsigned int Px_propto_g       : 1;  /* default is Px = 1 */
} corpusflags_type;

typedef struct {
  feature_type f;             /* feature */
  DataFloat c;                /* number of times feature occured */
} fc_type;

typedef struct {
  feature_type *f;            /* array of features with count 1 */
  size_type  nf;              /* number of features with count 1 */
  fc_type *fc;                /* array of feature counts */
  size_type  nfc;             /* number of feature counts */
  DataFloat   Pyx;            /* probability this parse is correct, 0 when not correct */
  DataFloat   p;	      /* number of parse edges */
  DataFloat   w;	      /* number of correct parse edges */
} parse_type;

typedef struct {
  parse_type   *parse;        /* array of parses */
  size_type    nparses;	      /* total number of parses */
  size_type    correct_index; /* index of correct parse, nparses when no correct parse */
  DataFloat    Px;            /* probability of this sentence, 0 when no correct parse */
  DataFloat    g;	      /* number of gold edges */
} sentence_type;

typedef struct {
  sentence_type *sentence;    /* array of sentences */
  size_type     nsentences;   /* number of sentences */
  size_type     nfeatures;    /* number of features */
  size_type	maxnparses;   /* maximum number of parses in a sentence */
  size_type	nloserparses; /* number of incorrect parses in all sentences */
} corpus_type;

/*! max_score_index returns the index of the parse with the highest score in s */

size_type max_score_index(const sentence_type *s, const Float w[]);

/*! read_corpus() reads corpus from in. */

corpus_type *read_corpus(corpusflags_type *flags, FILE *in);

/*! read_corpus_file() reads corpus from the file named filename.  
 *! If the filename suffix ends in .bz2 it is popened with bzcat,
 *! if it ends in .gz it is popened with zcat.
 */

corpus_type *read_corpus_file(corpusflags_type *flags, const char* filename);


/***********************************************************************
 *                                                                     *
 *                      linear logistic regression                     *
 *                                                                     *
 ***********************************************************************/

/*! corpus_stats() returns - count * log P(winners|parses), 
 *!  increments E_Ew[f] with the difference of the expected 
 *!  value of f for all parses and just for the winners
 *!  (i.e., the derivative of - count * log P(winners|parses)),
 *!  and increments sum_g, sum_p and sum_w.
 */


Float sentence_stats(sentence_type *s, const Float w[], Float score[], Float E_Ew[],
		     Float *sum_g, Float *sum_p, Float *sum_w);

Float corpus_stats(corpus_type *c, const Float w[], Float E_Ew[], 
	           Float *sum_g, Float *sum_p, Float *sum_w);


/***********************************************************************
 *                                                                     *
 *                          EM-style log loss                          *
 *                                                                     *
 ***********************************************************************/

/*! emll_corpus_stats() returns the EM-like log loss - E_P~(x)[log E_w[P~|x]],
 *!  sets dL_dw[f] to its derivative, sets the precision/recall scores,
 *!  and increments ncorrect.
 */

Float emll_corpus_stats(corpus_type *c, const Float w[], Float dL_dw[], 
			Float *sum_g, Float *sum_p, Float *sum_w);

/***********************************************************************
 *                                                                     *
 *                   linear pairwise log loss regression               *
 *                                                                     *
 ***********************************************************************/

/*! pwlog_corpus_stats() returns the sum of the pairwise log loss function
 *! on all winner-loser pairs, increments dL_dw[] with its derivative,
 *! and increments sum_g, sum_p and sum_w.
 */

Float pwlog_corpus_stats(corpus_type *c, const Float w[], Float dL_dw[], 
			 Float *sum_g, Float *sum_p, Float *sum_w);



/***********************************************************************
 *                                                                     *
 *                linear log pairwise exp loss regression              *
 *                                                                     *
 ***********************************************************************/

/*! log_exp_corpus_stats() returns the log sum of the pairwise exp loss function
 *! on all winner-loser pairs, increments dL_dw[] with its derivative,
 *! and increments sum_g, sum_p and sum_w.
 */

Float log_exp_corpus_stats(corpus_type *c, const Float w[], Float dL_dw[], 
			   Float *sum_g, Float *sum_p, Float *sum_w);


/***********************************************************************
 *                                                                     *
 *                   linear pairwise exp loss regression               *
 *                                                                     *
 ***********************************************************************/

/*! exp_corpus_stats() returns the sum of the pairwise exp loss function
 *! on all winner-loser pairs, increments dL_dw[] with its derivative,
 *! and increments sum_g, sum_p and sum_w.
 */

Float exp_corpus_stats(corpus_type *c, const Float w[], Float dL_dw[], 
		       Float *sum_g, Float *sum_p, Float *sum_w);


/***********************************************************************
 *                                                                     *
 *                      expected f-score optimization                  *
 *                                                                     *
 ***********************************************************************/

/*! fscore_corpus_stats() returns the statistics needed to calculate
 *! the expected f-score and its derivatives wrt the model parameters.
 *! It returns the expected f-score.
 */

Float fscore_corpus_stats(corpus_type *c, const Float w[], Float dFdw[],
			  Float *sum_g, Float *sum_p, Float *sum_w);

/***********************************************************************
 *                                                                     *
 *                        averaged perceptron                          *
 *                                                                     *
 ***********************************************************************/

/*! ap_sentence() handles a single round of the averaged perceptron.
 *!
 *!  s           - sentence data
 *!  w           - current weight vector, updated if necessary
 *!  dw          - weight update amount
 *!  weightdecay - weight decay factor (per round)
 *!  sum_w       - cumulative sum of weight vectors
 *!  it          - current iteration
 *!  changed[k]  - iteration at which w[k] was last changed
 */


void ap_sentence(sentence_type *s, Float w[], Float dw, Float weightdecay,
		 Float sum_w[], size_type it, size_type changed[]);


/*! wap_sentence() handles a single round of the weighted averaged perceptron.
 *!
 *!  s          - sentence data
 *!  w          - current weight vector, updated if necessary
 *!  dw         - weight update factor
 *!  feat_class - feature -> class vector
 *!  class_dw   - class weight update factor
 *!  sum_w      - cumulative sum of weight vectors
 *!  it         - current iteration
 *!  changed[k] - iteration at which w[k] was last changed
 */

void wap_sentence(sentence_type *s, Float w[], 
		  Float dw, const size_type feat_class[], const Float class_dw[],
		  Float sum_w[], size_type it, size_type changed[]);


/***********************************************************************
 *                                                                     *
 *                     logistic neural network                         *
 *                                                                     *
 ***********************************************************************/

/*! lnn_weights_type{} holds pointers to the separate weight components
 *! of the two-level neural network.
 */

typedef struct {
  Float *w1;            /*!< level 1 weights: w1[nhidden]            */
  Float *b0;            /*!< level 0 biases:  b0[nhidden]            */
  Float *w0;            /*!< level 0 weights: w0[nhidden][nfeatures] */
} lnn_weights_type;

/*! lnn_unpack_weights_type() sets a lnn_weights_type to point to the appropriate
 *! places in the single long weight vector w[].
 */

void lnn_unpack_weights_type(Float w[], size_type nhidden, size_type nfeatures, 
			     lnn_weights_type *wt);

/*! lnn_corpus_stats() returns - log P(winners|parses), 
 *!  sets dL_dw[], and increments sum_g, sum_p and sum_w.
 */

Float lnn_corpus_stats(corpus_type *c, /*!< Training corpus */
		       size_type nhidden, /*!< number of hidden units */
		       Float w[],      /*!< weights in lnn_unpack_weights_type() format */
		       Float dL_dw[],  /*!< derivative of weights */
		       Float *sum_g,   /*!< number of nonterminal edges in gold trees */
		       Float *sum_p,   /*!< number of nonterminal edges in parse trees */
		       Float *sum_w    /*!< number of correct edges */
		       );

#ifdef __cplusplus
};
#endif

#endif /* DATA_H */
