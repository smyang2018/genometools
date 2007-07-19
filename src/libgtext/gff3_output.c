/*
  Copyright (c) 2006 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include <assert.h>
#include "libgtcore/undef.h"
#include "libgtext/gff3_output.h"

void gff3_output_leading(GenomeFeature *gf, GenFile *outfp)
{
  GenomeNode *gn;
  GenomeFeatureType type;
  double score;

  assert(gf);

  gn = (GenomeNode*) gf;
  type = genome_feature_get_type(gf);

  genfile_xprintf(outfp, "%s\t%s\t%s\t%lu\t%lu\t",
                  str_get(genome_node_get_seqid(gn)),
                  genome_feature_get_source(gf),
                  genome_feature_type_get_cstr(type),
                  genome_node_get_start(gn),
                  genome_node_get_end(gn));
  score = genome_feature_get_score(gf);
  if (score == UNDEF_DOUBLE)
    genfile_xfputc('.', outfp);
  else
    genfile_xprintf(outfp, "%f", score);
  genfile_xprintf(outfp, "\t%c\t%c\t",
                  STRANDCHARS[genome_feature_get_strand(gf)],
                  PHASECHARS[genome_feature_get_phase(gf)]);
}
