/*
  Copyright (c) 2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <assert.h>
#include "libgtcore/undef.h"
#include "libgtext/genome_node_iterator.h"
#include "libgtext/genome_stream_rep.h"
#include "libgtext/uniq_stream.h"

struct UniqStream{
  const GenomeStream parent_instance;
  GenomeStream *in_stream;
  GenomeNode *first_node,
             *second_node;
};

#define uniq_stream_cast(GS)\
        genome_stream_cast(uniq_stream_class(), GS)

static bool nodes_are_equal_feature_trees(GenomeNode *first_node,
                                          GenomeNode *second_node, Env *env)
{
  bool equal = false;
  GenomeNodeIterator *gni_a, *gni_b;
  GenomeFeature *gf_a, *gf_b;
  env_error_check(env);
  gf_a = genome_node_cast(genome_feature_class(), first_node);
  gf_b = genome_node_cast(genome_feature_class(), second_node);
  if (gf_a && gf_b) {
    gni_a = genome_node_iterator_new(first_node, env);
    gni_b = genome_node_iterator_new(second_node, env);
    for (gf_a = (GenomeFeature*) genome_node_iterator_next(gni_a, env),
         gf_b = (GenomeFeature*) genome_node_iterator_next(gni_b, env);
         gf_a && gf_b;
         gf_a = (GenomeFeature*) genome_node_iterator_next(gni_a, env),
         gf_b = (GenomeFeature*) genome_node_iterator_next(gni_b, env)) {
      if (!gf_b || !genome_features_are_similar(gf_a, gf_b))
        break;
    }
    gf_b = (GenomeFeature*) genome_node_iterator_next(gni_b, env);
    if (!gf_a && !gf_b)
      equal = true;
    genome_node_iterator_delete(gni_a, env);
    genome_node_iterator_delete(gni_b, env);
    return equal;
  }
  return false;
}

static bool uniq(GenomeNode **first_node, GenomeNode **second_node, Env *env)
{
  double first_score, second_score;
  env_error_check(env);
  assert(*first_node && *second_node);
  if (nodes_are_equal_feature_trees(*first_node, *second_node, env)) {
    first_score = genome_feature_get_score((GenomeFeature*) *first_node);
    second_score = genome_feature_get_score((GenomeFeature*) *second_node);
    if ((first_score == UNDEF_DOUBLE && second_score == UNDEF_DOUBLE) ||
        (first_score != UNDEF_DOUBLE && second_score == UNDEF_DOUBLE) ||
        (first_score != UNDEF_DOUBLE && second_score != UNDEF_DOUBLE &&
         first_score >= second_score)) {
      /* keep first node */
      genome_node_rec_delete(*second_node);
    }
    else {
      /* keep second node */
      genome_node_rec_delete(*first_node);
      *first_node = *second_node;
    }
    *second_node = NULL;
    return true;
  }
  return false;
}

static int uniq_stream_next_tree(GenomeStream *gs, GenomeNode **gn, Env *env)
{
  UniqStream *us;
  int had_err;
  env_error_check(env);
  us = uniq_stream_cast(gs);

  assert(!us->second_node); /* the second buffer is always empty when this
                               function is called */
  if (!us->first_node) {
    /* both buffers are empty */
    had_err = genome_stream_next_tree(us->in_stream, &us->first_node, env);
    if (had_err)
      return had_err;
    if (!us->first_node) {
      *gn = NULL;
      return 0;
    }
  }

  /* uniq loop */
  for (;;) {
    assert(us->first_node && !us->second_node);
    had_err = genome_stream_next_tree(us->in_stream, &us->second_node, env);
    if (!had_err && us->second_node) {
      if (!uniq(&us->first_node, &us->second_node, env))
        break; /* no uniq possible */
    }
    else
      break;
  }

  /* serve node */
  if (!had_err) {
    assert(us->first_node);
    *gn = us->first_node;
    us->first_node = us->second_node;
    us->second_node = NULL;
  }

  return had_err;
}

static void uniq_stream_free(GenomeStream *gs)
{
  UniqStream *us = uniq_stream_cast(gs);
  genome_node_rec_delete(us->first_node);
  genome_node_rec_delete(us->second_node);
}

const GenomeStreamClass* uniq_stream_class(void)
{
  static const GenomeStreamClass gsc = { sizeof (UniqStream),
                                         uniq_stream_next_tree,
                                         uniq_stream_free };
  return &gsc;
}

GenomeStream* uniq_stream_new(GenomeStream *in_stream, Env *env)
{
  GenomeStream *gs;
  UniqStream *us;
  assert(in_stream && genome_stream_is_sorted(in_stream));
  gs = genome_stream_create(uniq_stream_class(), true, env);
  us = uniq_stream_cast(gs);
  us->in_stream = in_stream;
  return gs;
}
