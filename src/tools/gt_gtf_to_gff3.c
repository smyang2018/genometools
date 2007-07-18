/*
  Copyright (c) 2006-2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006-2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include "libgtcore/option.h"
#include "libgtcore/versionfunc.h"
#include "libgtext/gff3_out_stream.h"
#include "libgtext/gtf_in_stream.h"

static OPrval parse_options(int *parsed_args, bool *be_tolerant, int argc,
                            const char **argv, Env *env)
{
  OptionParser *op;
  Option *option;
  OPrval oprval;
  op = option_parser_new("[gtf_file]", "Parse GTF2.2 file and show it as GFF3.",
                         env);
  /* -tolerant */
  option = option_new_bool("tolerant", "be tolerant when parsing the GTF file",
                           be_tolerant, false, env);
  option_parser_add_option(op, option, env);
  /* parse */
  oprval = option_parser_parse_max_args(op, parsed_args, argc, argv,
                                        versionfunc, 1, env);
  option_parser_delete(op, env);
  return oprval;
}

int gt_gtf_to_gff3(int argc, const char **argv, Env *env)
{
  GenomeStream *gtf_in_stream = NULL, *gff3_out_stream = NULL;
  GenomeNode *gn;
  int parsed_args, had_err = 0;
  bool be_tolerant;
  env_error_check(env);

  /* option parsing */
  switch (parse_options(&parsed_args, &be_tolerant, argc, argv, env)) {
    case OPTIONPARSER_OK: break;
    case OPTIONPARSER_ERROR: return -1;
    case OPTIONPARSER_REQUESTS_EXIT: return 0;
  }

  /* create a gtf input stream */
  gtf_in_stream = gtf_in_stream_new(argv[parsed_args], be_tolerant, env);
  if (!gtf_in_stream)
    had_err = -1;

  if (!had_err) {
    /* create a gff3 output stream */
    /* XXX: use proper genfile */
    gff3_out_stream = gff3_out_stream_new(gtf_in_stream, NULL, env);

    /* pull the features through the stream and free them afterwards */
    while (!(had_err = genome_stream_next_tree(gff3_out_stream, &gn, env)) &&
           gn) {
      genome_node_rec_delete(gn, env);
    }
  }

  /* free */
  genome_stream_delete(gff3_out_stream, env);
  genome_stream_delete(gtf_in_stream, env);

  return had_err;
}
