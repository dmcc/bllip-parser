/* data.h
 *
 * Mark Johnson, 27th July 2003, modified 16th March 2004
 *
 * Data structures for weighted log-likelihood estimator from partially
 * visible data.
 *
 * Special cases features with weight 1.
 */

#ifndef DATA_H
#define DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef unsigned int feature_type;
#define FEATURE_FORMAT "%u"

typedef double Float;
#define FLOAT_FORMAT "%lg"

typedef float DataFloat;
#define DATAFLOAT_FORMAT "%g"

#define DATAFLOAT_MAX 1e10
#define DATAFLOAT_EPS 1e-7

void blockalloc_free_all(void);        /* releases all corpus data */

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
  size_t  nf;                 /* number of features with count 1 */
  fc_type *fc;                /* array of feature counts */
  size_t  nfc;                /* number of feature counts */
  DataFloat   Pyx;            /* probability this parse is a winner, 0 when no winner */
  DataFloat   p;	      /* number of parse edges */
  DataFloat   w;	      /* number of correct parse edges */
} parse_type;

typedef struct {
  parse_type   *parse;        /* array of parses */
  unsigned int nparses;	      /* total number of parses */
  DataFloat    Px;            /* probability of this sentence, 0 when no winner */
  DataFloat    g;	      /* number of gold edges */
} sentence_type;

typedef struct {
  sentence_type *sentence;    /* array of sentences */
  size_t        nsentences;   /* number of sentences */
  size_t	nfeatures;    /* number of features */
  size_t	maxnparses;   /* maximum number of parses in a sentence */
} corpus_type;


/*! read_corpus() reads corpus from in.  nsentences is an estimate of
 *! the number of sentences.
 */

corpus_type *read_corpus(corpusflags_type *flags, FILE *in, int nsentences);


/***********************************************************************
 *                                                                     *
 *                      linear logistic regression                     *
 *                                                                     *
 ***********************************************************************/

/*! sentence_scores() loads score[] with the scores of all parses in s,
 *! and sets max_correct_score and max_score.  Returns the index of the
 *! last highest scoring parse.
 */

size_t sentence_scores(sentence_type *s, const Float w[], Float score[],
		       Float *max_correct_score, Float *max_score);

/*! corpus_stats() returns - count * log P(winners|parses), 
 *!  increments E_Ew[f] with the difference of the expected 
 *!  value of f for all parses and just for the winners
 *!  (i.e., the derivative of - count * log P(winners|parses)),
 *!  and increments ncorrect.
 */

Float corpus_stats(corpus_type *c, const Float w[], Float E_Ew[], 
	           Float *sum_g, Float *sum_p, Float *sum_w);


/***********************************************************************
 *                                                                     *
 *                        averaged perceptron                          *
 *                                                                     *
 ***********************************************************************/

/*! ap_sentence() handles a single round of the averaged perceptron.
 *!
 *!  s          - sentence data
 *!  score      - working vector (loaded with parse scores)
 *!  w          - current weight vector, updated if necessary
 *!  dw         - weight update amount
 *!  sum_w      - cumulative sum of weight vectors
 *!  it         - current iteration
 *!  changed[k] - iteration at which w[k] was last changed
 */


void ap_sentence(sentence_type *s, Float score[], Float w[], Float dw, 
		 Float sum_w[], size_t it, size_t changed[]);


/*! wap_sentence() handles a single round of the weighted averaged perceptron.
 *!
 *!  s          - sentence data
 *!  score      - working vector (loaded with parse scores)
 *!  w          - current weight vector, updated if necessary
 *!  dw         - weight update factor
 *!  feat_class - feature -> class vector
 *!  class_dw   - class weight update factor
 *!  sum_w      - cumulative sum of weight vectors
 *!  it         - current iteration
 *!  changed[k] - iteration at which w[k] was last changed
 */

void wap_sentence(sentence_type *s, Float score[], Float w[], 
		  Float dw, const size_t feat_class[], const Float class_dw[],
		  Float sum_w[], size_t it, size_t changed[]);


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

void lnn_unpack_weights_type(Float w[], size_t nhidden, size_t nfeatures, 
			     lnn_weights_type *wt);

/*! lnn_corpus_stats() returns - log P(winners|parses), 
 *!  sets dL_dw[], and increments sum_g, sum_p and sum_w.
 */

Float lnn_corpus_stats(corpus_type *c, /*!< Training corpus */
		       size_t nhidden, /*!< number of hidden units */
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
