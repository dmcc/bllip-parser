/*
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* data.c
 *
 * Mark Johnson, 27th July 2003,
 * modified Mark Johnson, 5th August 2003 to calculate the number of
 *  sentences in a corpus as it is read.
 * modified Mark Johnson, 25th August 2003 to special case features
 *  with a count of 1.
 * modified Mark Johnson, 28th August 2003 to handle precision & recall.
 * modified Mark Johnson, 4th September 2003 for averaged perceptron.
 * modified Mark Johnson, 21st September 2003 for weighted averaged perceptron.
 * modified Mark Johnson, 7th October 2003 for logistic neural net
 * modified Mark Johnson, 12th October 2003 to select best parse & mem alloc
 */

#include "data.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MALLOC(n)	malloc(n)
#define REALLOC(x,n)	realloc(x,n)
#define FREE(x)		free(x)

/* used on data never freed */

#define SMALLOC(n)		blockalloc_malloc(n)
#define SREALLOC(x,old_n,new_n)	blockalloc_realloc(x,old_n,new_n)

/***********************************************************************
 *                                                                     *
 *                             Utility                                 *
 *                                                                     *
 ***********************************************************************/

static DataFloat feature_value(parse_type *p, feature_type f) {
  size_t i;
  for (i = 0; i < p->nfc; ++i)
    if (p->fc[i].f == f)
      return p->fc[i].c;
  for (i = 0; i < p->nf; ++i)
    if (p->f[i] == f)
      return 1;
  return 0;
}  // feature_value

/* memory stuff */

#define NUNITS(N)    (1 + (N-1)/sizeof(align))

typedef int align;

/* blocksize is the number of align-sized units in the memory
 * block grabbed from malloc.  This setting will give approximately
 * 8Mb-sized blocks.  I subtract 10 align-sized units from exactly
 * 1Mb so that the block with my and the system's overhead will
 * probably still fit in a real 8Mb-size unit.
 */

#define BLOCKSIZE    (1048576-10)

struct blockalloc {
  align           data[BLOCKSIZE];
  size_t          top;
  struct blockalloc *next;
};

static struct blockalloc *current_block = NULL;

void blockalloc_free_all(void)
{
  struct blockalloc *p = current_block;

  while (p) {
    struct blockalloc *q = p->next;
    FREE(p);
    p = q;
  }

  current_block = NULL;
}

__inline__
static void* blockalloc_malloc(size_t n_char)
{
  struct blockalloc *p = current_block;
  size_t n = NUNITS(n_char);

  if (n_char == 0 || n > BLOCKSIZE)
    return NULL;

  if (p == NULL || p->top < n) {
    p = MALLOC(sizeof(struct blockalloc));
    assert(p != NULL);
    p->next = current_block;
    p->top = BLOCKSIZE;
    current_block = p;
  }

  p->top -= n;
  return (void *) &p->data[p->top];
}  // blockalloc_malloc()


__inline__
static void* blockalloc_realloc(void *p, size_t old_n_char, size_t new_n_char)
{
  void *new_p = SMALLOC(new_n_char);
  size_t n_move = old_n_char < new_n_char ? old_n_char : new_n_char;
  if (new_p == NULL)
    return NULL;
  if (n_move != 0)
    memcpy(new_p, p, n_move);
  FREE(p);
  return new_p;
}

/***********************************************************************
 *                                                                     *
 *                            read_corpus()                            *
 *                                                                     *
 ***********************************************************************/

#define MIN_NF   1024
#define MIN_NFC  1024

static size_t last_nf = 0;
static size_t last_nfc = 0;

static void read_parse(FILE *in, parse_type *p, feature_type *fmax) {
  feature_type f, *fp;
  fc_type *fcp;
  int nread;
  size_t nf = 0, nfc = 0;
  DataFloat c;

  size_t nf_max = (last_nf <= MIN_NF) ? MIN_NF : last_nf;
  size_t nfc_max = (last_nfc <= MIN_NFC) ? MIN_NFC : last_nfc;

  fcp = MALLOC(nfc_max*sizeof(fc_type));    /* allocate feature counts */
  assert(fcp != NULL);
  fp = MALLOC(nf_max*sizeof(feature_type)); /* allocate features w/ 1 count */
  assert(fp != NULL);

  p->p = 1;
  nread = fscanf(in, " P = " DATAFLOAT_FORMAT " ", &p->p); /* read p */
  assert(nread != EOF);

  p->w = 0;
  nread = fscanf(in, " W = " DATAFLOAT_FORMAT " ", &p->w); /* read pwinner */
  assert(nread != EOF);

  while ((nread = fscanf(in, " " FEATURE_FORMAT " ", &f))) { /* read feature */
    assert(nread != EOF);
    if (nread == 0)
      break;
    assert(f >= 0);

    c = 1.0;                                          /* default value for c */
    /* read feature count */
    nread = fscanf(in, " = " DATAFLOAT_FORMAT " ", &c); 
    assert(nread != EOF);

    if (f > *fmax)
      *fmax = f;

    if (c == 1.0) {
      if (nf >= nf_max) {
	nf_max *= 2;
	fp = REALLOC(fp, nf_max*sizeof(feature_type));
	assert(fp != NULL);
      }
      assert(nf<nf_max);
      fp[nf] = f;
      nf++;
    }
    else {   /* c != 1 */
      if (nfc >= nfc_max) {
	nfc_max *= 2;
	fcp = REALLOC(fcp, nfc_max*sizeof(fc_type));
	assert(fcp != NULL);
      }
      assert(nfc<nfc_max);
      fcp[nfc].f = f;
      fcp[nfc].c = c;
      nfc++;
    }
  }

  /* copy features into p */

  last_nf = p->nf = nf;
  p->f = SREALLOC(fp, nf_max*sizeof(feature_type), nf*sizeof(feature_type));
  if (nf > 0)
    assert(p->f != NULL);

  last_nfc = p->nfc = nfc;
  p->fc = SREALLOC(fcp, nfc_max*sizeof(fc_type), nfc*sizeof(fc_type));
  if (nfc > 0)
    assert(p->fc != NULL);

  fscanf(in, " ,");   /* read final ',' */

}  /* read_parse() */


/*! read_sentence() reads a sentence, normalizes pwinner on parses,
 *!  and updates maxnparses.
 */

int read_sentence(corpusflags_type *flags, FILE *in, sentence_type *s, 
		  feature_type *fmax, int *maxnparses) {
  int i, nread, best_fscore_index = -1, nwinners = 0;
  DataFloat fscore, best_logprob = -DATAFLOAT_MAX, best_fscore = -1;

  nread = fscanf(in, " G = " DATAFLOAT_FORMAT " ", &s->g);
  if (nread == EOF)
    return EOF;
  assert(nread == 0 || nread == 1);
  if (nread == 0)
    s->g = 1;

  nread = fscanf(in, " N = %u", &s->nparses);
  if (nread == EOF)
    return EOF;
  assert(nread == 1);
  assert(s->nparses >= 0);
  if (s->nparses > *maxnparses)
    *maxnparses = s->nparses;

  if (s->nparses > 0) {
    s->parse = SMALLOC(s->nparses*sizeof(parse_type));
    assert(s->parse != NULL);
  }
  else
    s->parse = NULL;

  for (i = 0; i < s->nparses; ++i) {
    read_parse(in, &s->parse[i], fmax);
    fscore = 2 * s->parse[i].w/(s->parse[i].p+s->g);
    if (fscore+DATAFLOAT_EPS >= best_fscore) {
      Float logprob = feature_value(&s->parse[i], 0); // logprob is feature 0
      if (fabs(fscore-best_fscore) < 2*DATAFLOAT_EPS) {
	++nwinners;                   // tied best f-scores    
	if (logprob > best_logprob) { // pick candidate with best logprob
	  best_fscore = fscore;
	  best_logprob = logprob;
	  best_fscore_index = i;
	}
      }
      else {
	best_fscore = fscore;
	best_logprob = logprob;
	best_fscore_index = i;
	nwinners = 1;
      }
    }
  }

  if (best_fscore_index >= 0) {  /* is there a winner? */
    Float sum_Pyx = 0;
    assert(nwinners > 0);
    s->Px = 1.0;		 /* indicate that there is a winner */
    if (flags->Pyx_factor > 1) {
      Float Z = 0;
      for (i = 0; i < s->nparses; ++i) {
	fscore = 2 * s->parse[i].w/(s->parse[i].p+s->g);
	Z += pow(flags->Pyx_factor, fscore - best_fscore);
      }
      for (i = 0; i < s->nparses; ++i) {
	fscore = 2 * s->parse[i].w/(s->parse[i].p+s->g);
	sum_Pyx += s->parse[i].Pyx = pow(flags->Pyx_factor, fscore - best_fscore) / Z;
      }
    }
    else if (flags->Pyx_factor > 0) { 
      /* Pyx_factor == 1; all winners get equal Pyx */
      for (i = 0; i < s->nparses; ++i) {
	Float fscore = 2 * s->parse[i].w/(s->parse[i].p+s->g);
	if (fabs(best_fscore-fscore) < 2*DATAFLOAT_EPS)
	  sum_Pyx += s->parse[i].Pyx = 1.0/nwinners;
	else
	  sum_Pyx += s->parse[i].Pyx = 0;
      }
    }	  
    else {  /* Pyx_factor == 0; f-score winner gets Pyx = 1, all others 0 */
      for (i = 0; i < s->nparses; ++i)  
	sum_Pyx += s->parse[i].Pyx = (i == best_fscore_index);
    }
    assert(fabs(sum_Pyx - 1) <= DATAFLOAT_EPS);
  }
  else {    /* no winner, set Px to 0.0 to indicate this */
    s->Px = 0.0;
    for (i = 0; i < s->nparses; ++i)
      s->parse[i].Pyx = 0.0;
  }

  return s->nparses;
}  /* read_sentence() */

corpus_type *read_corpus(corpusflags_type *flags, FILE *in, int nsentences) {
  sentence_type s;
  feature_type fmax = 0;
  int nread, i = 0, maxnparses = 0;
  Float sum_g = 0;

  corpus_type *c = SMALLOC(sizeof(corpus_type));
  assert(c != NULL);

  nread = fscanf(in, " S = %d ", &nsentences);
  assert(nread != EOF);
  c->sentence = MALLOC(nsentences*sizeof(sentence_type));
  assert(c->sentence != NULL);

  while (read_sentence(flags, in, &s, &fmax, &maxnparses) != EOF) {
    if (i >= nsentences) {
      nsentences *= 2;
      c->sentence = REALLOC(c->sentence, nsentences*sizeof(sentence_type));
      assert(c->sentence != NULL);
    }
    assert(i < nsentences);
    c->sentence[i++] = s;
    sum_g += s.g;
  }
  c->nsentences = i;
  c->sentence = SREALLOC(c->sentence, nsentences*sizeof(sentence_type),
			 c->nsentences*sizeof(sentence_type));
  assert(c->sentence != NULL);
  c->nfeatures = fmax+1;
  c->maxnparses = maxnparses;

  if (flags->Px_propto_g)
    for (i = 0; i < c->nsentences; ++i)  /* normalize Px */
      c->sentence[i].Px *= c->nsentences * c->sentence[i].g / sum_g;

  return c;
}  /* read_corpus() */


/***********************************************************************
 *                                                                     *
 *                      linear logistic regression                     *
 *                                                                     *
 *                            corpus_stats()                           *
 *                                                                     *
 ***********************************************************************/

/*! parse_score() returns the score for parse *p. */

__inline__
static Float parse_score(parse_type *p, const Float w[]) {
  int i;
  Float score = 0;
  for (i = 0; i < p->nf; ++i)
    score += w[p->f[i]];
  for (i = 0; i < p->nfc; ++i)
    score += p->fc[i].c * w[p->fc[i].f];
  return score;
}  /* parse_score() */


/*! sentence_scores() loads score[] with the scores of all parses in s,
 *! and sets max_correct_score and max_score.  Returns the index of the
 *! last highest scoring parse.
 */

size_t sentence_scores(sentence_type *s, const Float w[], Float score[],
		       Float *max_correct_score, Float *max_score) {
  int i, best_i = 0, best_correct_i = -1;
  Float sc;
  assert(s->nparses > 0);

  *max_score = score[0] = sc = parse_score(&s->parse[0], w);
  if (s->parse[0].Pyx > 0) {
    best_correct_i = 0;
    *max_correct_score = sc;
  }

  for (i = 1; i < s->nparses; ++i) {
    score[i] = sc = parse_score(&s->parse[i], w);
    if (sc >= *max_score) {
      best_i = i;
      *max_score = sc;
    }
    if (s->parse[i].Pyx > 0 && (best_correct_i < 0 || sc > *max_correct_score)) {
      best_correct_i = i;
      *max_correct_score = sc;
    }
  }

  assert(finite(*max_score));
  assert(s->Px == 0 || best_correct_i >= 0);
  /* assert(best_correct_i >= 0); assert(finite(*max_correct_score)); */
  return best_i;
}  /* sentence_scores() */


/*! sentence_stats() returns - log P~(x) (E_P~[w.f|x] - log Z_w(x))
 *!  increments E_Ew[f] with the difference of the expected 
 *!  value of f for the winners and for all parses,
 *!  and increments the precision/recall scores.
 */

Float sentence_stats(sentence_type *s, const Float w[], Float score[], Float E_Ew[],
		     Float *sum_g, Float *sum_p, Float *sum_w) {
  Float max_correct_score, max_score;
  int i, j, best_i;
  Float Z = 0, logZ, Ecorrect_score = 0;

  *sum_g += s->g;

  if (s->nparses <= 0) 
    return 0;

  best_i = sentence_scores(s, w, score, &max_correct_score, &max_score);
  *sum_p += s->parse[best_i].p;
  *sum_w += s->parse[best_i].w;
  
  if (s->Px == 0)  /* skip statistics calculation if Px == 0 */
    return 0;

  assert(max_correct_score <= max_score);

  for (i = 0; i < s->nparses; ++i) {   /* compute Z and Zw */
    assert(score[i] <= max_score);
    Z += exp(score[i] - max_score);
    assert(finite(Z));
    if (s->parse[i].Pyx > 0) 
      Ecorrect_score += s->parse[i].Pyx * score[i];
  }

  logZ = log(Z) + max_score;

  /* calculate expectations */

  for (i = 0; i < s->nparses; ++i) {
    Float cp = exp(score[i] - logZ);  /* P_w(y|x) */
    assert(finite(cp));

    if (s->parse[i].Pyx > 0)  /* P_e(y|x)  */
      cp -= s->parse[i].Pyx;

    assert(cp >= -1.0);
    assert(cp <= 1.0);
    
    cp *= s->Px;

    /* calculate expectations */

    for (j = 0; j < s->parse[i].nf; ++j)  /* features with 1 count */
      E_Ew[s->parse[i].f[j]] += cp;

    for (j = 0; j < s->parse[i].nfc; ++j) /* features with arbitrary counts */
      E_Ew[s->parse[i].fc[j].f] += cp * s->parse[i].fc[j].c;
  }
  return - s->Px * (Ecorrect_score - logZ);
}  /* sentence_stats() */
  

/* corpus_stats() returns - count * log P(winners|parses), 
 *  increments E_Ew[f] with the difference of the expected 
 *  value of f for all parses and just for the winners
 *  (i.e., the derivative of - count * log P(winners|parses)),
 *  and increments ncorrect.
 */

Float corpus_stats(corpus_type *c, const Float w[], Float E_Ew[], 
		   Float *sum_g, Float *sum_p, Float *sum_w) 
{
  Float neglogP = 0;
  Float *score = MALLOC(c->maxnparses*sizeof(Float));
  int i;

  assert(score != NULL);

  *sum_g = *sum_p = *sum_w = 0;          /* zero precision/recall counters */
  for (i = 0; i < c->nfeatures; ++i)     /* zero Ew_E[] */
    E_Ew[i] = 0;
  for (i = 0; i < c->nsentences; ++i)    /* collect stats from sentences */
    neglogP += sentence_stats(&c->sentence[i], w, score, E_Ew, 
			      sum_g, sum_p, sum_w);
  FREE(score);
  return neglogP;
}  /* corpus_stats() */


/***********************************************************************
 *                                                                     *
 *                        averaged perceptron                          *
 *                                                                     *
 ***********************************************************************/

/*! ap_update1() performs an averaged perceptron update on feature j.
 */

__inline__ static
void ap_update1(feature_type j, Float w[], Float update, Float sum_w[],
		size_t it, size_t changed[]) {
  sum_w[j] += (it - changed[j]) * w[j];
  changed[j] = it;
  w[j] += update;
}  /* ap_update1() */

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
		 Float sum_w[], size_t it, size_t changed[])
{
  Float max_winner_score, max_score;
  size_t best_i = sentence_scores(s, w, score, &max_winner_score, &max_score);

  if (score[0] <= max_score) { /* update between parse[0] and parse[best_i] */
    size_t j;
    parse_type *correct = &s->parse[0], *winner = &s->parse[best_i];

    if (winner->Pyx >= correct->Pyx)
      return;

    /* multiply update weight by importance of this pair */
    assert(correct->Pyx > 0);
    dw *= s->Px * fabs(correct->Pyx - winner->Pyx)/correct->Pyx;

    /* subtract winner's feature counts */
    for (j = 0; j < winner->nf; ++j) 
      ap_update1(winner->f[j], w, -dw, sum_w, it, changed);
    for (j = 0; j < winner->nfc; ++j)
      ap_update1(winner->fc[j].f, w, -dw*winner->fc[j].c, sum_w, it, changed);

    /* add correct's feature counts */
    for (j = 0; j < correct->nf; ++j)
      ap_update1(correct->f[j], w, dw, sum_w, it, changed);
    for (j = 0; j < correct->nfc; ++j)
      ap_update1(correct->fc[j].f, w, dw*correct->fc[j].c, sum_w, it, changed);
  }
}  /* ap_sentence() */


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
		  Float sum_w[], size_t it, size_t changed[])
{
  Float max_winner_score, max_score;
  size_t best_i = sentence_scores(s, w, score, &max_winner_score, &max_score);

  if (score[0] <= max_score) { /* update between parse[0] and parse[best_i] */
    size_t j;
    parse_type *correct = &s->parse[0], *winner = &s->parse[best_i];

    /* multiply update weight by importance of this pair */
    dw *= s->Px * fabs(correct->Pyx - winner->Pyx);

    /* subtract winner's feature counts */
    for (j = 0; j < winner->nf; ++j) {
      size_t f = winner->f[j];
      ap_update1(f, w, -dw*class_dw[feat_class[f]], sum_w, it, changed);
    }
    for (j = 0; j < winner->nfc; ++j) {
      size_t f = winner->fc[j].f;
      ap_update1(f, w, -dw*winner->fc[j].c*class_dw[feat_class[f]], 
		 sum_w, it, changed);
    }

    /* add correct's feature counts */
    for (j = 0; j < correct->nf; ++j) {
      size_t f = correct->f[j];
      ap_update1(f, w, dw*class_dw[feat_class[f]], sum_w, it, changed);
    }
    for (j = 0; j < correct->nfc; ++j) {
      size_t f = correct->fc[j].f;
      ap_update1(f, w, dw*correct->fc[j].c*class_dw[feat_class[f]], sum_w, it, changed);
    }
  }
}  /* wap_sentence() */


/***********************************************************************
 *                                                                     *
 *                     logistic neural network                         *
 *                                                                     *
 ***********************************************************************/

/*! lnn_unpack_weights_type() sets a lnn_weights_type to point to the appropriate
 *! places in the single long weight vector w[].
 */

void lnn_unpack_weights_type(Float w[], size_t nhidden, size_t nfeatures,
			     lnn_weights_type *wt) 
{
  wt->w1 = w;
  wt->b0 = w+nhidden;
  wt->w0 = w+2*nhidden;
}  /* lnn_unpack_weights_type() */


/*! lnn_parse_score() sets score0[] (the hidden units scores) and
 *! returns score1 for parse *p. 
 */

__inline__
static Float lnn_parse_score(parse_type *p, const lnn_weights_type* wt, 
			     size_t nhidden, size_t nfeatures,
			     Float score0[]) {
  Float score1 = 0;
  int j;
  for (j = 0; j < nhidden; ++j) {
    Float input = parse_score(p, &wt->w0[j*nfeatures]) + wt->b0[j];
    score0[j] = tanh(input);  /* tanh sigmoid activation function */
    score1 += wt->w1[j] * score0[j];
  }
  return score1;
}  /* lnn_parse_score() */


/*! lnn_sentence_scores() loads score[] with the scores of all parses in s,
 *! and sets max_correct_score and max_score.  Returns the index of the
 *! last highest scoring parse.
 */

size_t lnn_sentence_scores(sentence_type *s, const lnn_weights_type* wt, 
			   size_t nhidden, size_t nfeatures,
			   Float score0[], Float score1[],
			   Float *max_correct_score, Float *max_score) {
  int i, best_i = 0, best_correct_i = -1;
  Float sc;
  assert(s->nparses > 0);

  *max_score = score1[0] = sc 
    = lnn_parse_score(&s->parse[0], wt, nhidden, nfeatures, &score0[0]);

  if (s->parse[0].Pyx > 0) {
    best_correct_i = 0;
    *max_correct_score = sc;
  }

  for (i = 1; i < s->nparses; ++i) {
    score1[i] = sc 
      = lnn_parse_score(&s->parse[i], wt, nhidden, nfeatures, &score0[i*nhidden]);
    if (sc >= *max_score) {
      best_i = i;
      *max_score = sc;
    }
    if (s->parse[i].Pyx > 0 && (best_correct_i < 0 || sc > *max_correct_score)) {
      best_correct_i = i;
      *max_correct_score = sc;
    }
  }

  assert(finite(*max_score));
  assert(s->Px == 0 || best_correct_i >= 0);
  /* assert(best_correct_i >= 0); assert(finite(*max_correct_score)); */
  return best_i;
}  /* lnn_sentence_scores() */


/*! lnn_sentence_stats() returns - log P~(x) (E_P~[w.f|x] - log Z_w(x))
 *!  increments E_Ew[f] with the difference of the expected 
 *!  value of f for the winners and for all parses,
 *!  and increments the precision/recall scores.
 */

Float lnn_sentence_stats(sentence_type *s, const lnn_weights_type* wt,
			 size_t nhidden, size_t nfeatures,
			 Float score0[], Float score1[], 
			 const lnn_weights_type* dL_dwt,
			 Float *sum_g, Float *sum_p, Float *sum_w) 
{ 
  Float max_correct_score, max_score;
  int i, j, k;
  Float Z = 0, logZ, Ecorrect_score = 0;
  
  size_t best_i = lnn_sentence_scores(s, wt, nhidden, nfeatures,
				      score0, score1, 
				      &max_correct_score, &max_score);

  *sum_g += s->g;
  *sum_p += s->parse[best_i].p;
  *sum_w += s->parse[best_i].w;
  
  if (s->Px == 0)  /* skip statistics calculation if Px == 0 */
    return 0;

  assert(max_correct_score <= max_score);

  for (i = 0; i < s->nparses; ++i) {   /* compute Z and Zw */
    assert(score1[i] <= max_score);
    Z += exp(score1[i] - max_score);
    assert(finite(Z));
    if (s->parse[i].Pyx > 0) 
      Ecorrect_score += s->parse[i].Pyx * score1[i];
  }

  logZ = log(Z) + max_score;

  /* calculate expectations */

  for (i = 0; i < s->nparses; ++i) {
    Float cp = exp(score1[i] - logZ);  /* P_w(y|x) */
    assert(finite(cp));

    if (s->parse[i].Pyx > 0)  /* P_e(y|x)  */
      cp -= s->parse[i].Pyx;

    assert(cp >= -1.0);
    assert(cp <= 1.0);
    
    cp *= s->Px;   /* multiply in weight of sentence */

    /* calculate derivatives */

    for (j = 0; j < nhidden; ++j) {
      Float g = score0[i*nhidden+j];
      Float dg_dx = 1 - g*g;  /* for tanh activation function */
      Float backward = cp * wt->w1[j] * dg_dx;
      dL_dwt->w1[j] += cp * score0[i*nhidden+j];
      dL_dwt->b0[j] += backward;
      
      for (k = 0; k < s->parse[i].nf; ++k)  /* features with 1 count */
	dL_dwt->w0[j*nfeatures+s->parse[i].f[k]] += backward;
      
      for (k = 0; k < s->parse[i].nfc; ++k) /* features with arbitrary counts */
	dL_dwt->w0[j*nfeatures+s->parse[i].fc[k].f] 
	  += backward * s->parse[i].fc[k].c;
    }
  }
  return - s->Px * (Ecorrect_score - logZ);
}  /* lnn_sentence_stats() */
  

/*! lnn_corpus_stats() returns - log P(winners|parses), 
 *!  sets dL_dw[], and increments sum_g, sum_p and sum_w.
 */
  
Float lnn_corpus_stats(corpus_type *c, size_t nhidden, 
		       Float w[], Float dL_dw[],
		       Float *sum_g, Float *sum_p, Float *sum_w) 
{
  lnn_weights_type wt, dL_dwt;
  Float neglogP = 0;
  Float *score1 = MALLOC(c->maxnparses*sizeof(Float));
  Float *score0 = MALLOC(nhidden * c->maxnparses*sizeof(Float));

  int i;
  
  assert(score1 != NULL);
  assert(score0 != NULL);

  *sum_g = *sum_p = *sum_w = 0;          /* zero precision/recall counters */

  for (i = 0; i < nhidden*(c->nfeatures+2); ++i) 
    dL_dw[i] = 0;    /* zero dL_dw[] */

  lnn_unpack_weights_type(w, nhidden, c->nfeatures, &wt);
  lnn_unpack_weights_type(dL_dw, nhidden, c->nfeatures, &dL_dwt);

  for (i = 0; i < c->nsentences; ++i)  /* collect stats from sentences */
    neglogP += lnn_sentence_stats(&c->sentence[i], &wt, nhidden, c->nfeatures, 
				  score0, score1, &dL_dwt, sum_g, sum_p, sum_w);

  FREE(score0);
  FREE(score1);
  return neglogP;
}  /* lnn_corpus_stats() */
