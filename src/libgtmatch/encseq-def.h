/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#ifndef ENCSEQDEF_H
#define ENCSEQDEF_H
#include "libgtcore/str.h"
#include "libgtcore/strarray.h"
#include "symboldef.h"
#include "seqpos-def.h"
#include "alphadef.h"
#include "readmode-def.h"

#define ENCSEQFILESUFFIX     ".esq"

#define REVERSEPOS(TOT,POS) ((TOT) - 1 - (POS))

typedef struct Encodedsequence Encodedsequence;
typedef struct Encodedsequencescanstate Encodedsequencescanstate;
typedef struct Specialrangeiterator Specialrangeiterator;

typedef struct
{
  Seqpos leftpos,
         rightpos;
} Sequencerange;          /* \Typedef{Sequencerange} */

Seqpos getencseqtotallength(const Encodedsequence *encseq);

Uchar getencodedchar(const Encodedsequence *encseq,Seqpos pos,
                     Readmode readmode);

int flushencseqfile(const Str *indexname,Encodedsequence *encseq,Env *env);

void freeEncodedsequence(Encodedsequence **encseqptr,Env *env);

/*@null@*/ Encodedsequencescanstate *initEncodedsequencescanstate(
                                         const Encodedsequence *encseq,
                                         Readmode readmode,
                                         Env *env);

void freeEncodedsequencescanstate(Encodedsequencescanstate **esr,Env *env);

Uchar sequentialgetencodedchar(const Encodedsequence *encseq,
                               Encodedsequencescanstate *esr,
                               Seqpos pos);

/*@null@*/ Encodedsequence *files2encodedsequence(bool withrange,
                                                  const StrArray *filenametab,
                                                  bool plainformat,
                                                  Seqpos totallength,
                                                  const Specialcharinfo
                                                        *specialcharinfo,
                                                  const Alphabet *alphabet,
                                                  const char *str_sat,
                                                  Env *env);

/*@null@*/ Encodedsequence *mapencodedsequence(bool withrange,
                                               const Str *indexname,
                                               Seqpos totallength,
                                               const Specialcharinfo
                                                     *specialcharinfo,
                                               const Alphabet *alphabet,
                                               const char *str_sat,
                                               Env *env);

Encodedsequence *plain2encodedsequence(bool withrange,
                                       Specialcharinfo *specialcharinfo,
                                       const Uchar *seq1,
                                       Seqpos len1,
                                       const Uchar *seq2,
                                       unsigned long len2,
                                       const Alphabet *alphabet,
                                       Env *env);

bool hasspecialranges(const Encodedsequence *encseq);

Specialrangeiterator *newspecialrangeiterator(const Encodedsequence *encseq,
                                              bool moveforward,
                                              Env *env);

bool nextspecialrangeiterator(Sequencerange *range,Specialrangeiterator *sri);

void freespecialrangeiterator(Specialrangeiterator **sri,Env *env);

/*@null@*/ char *encseqaccessname(const Encodedsequence *encseq);

bool exhaustedrangeiterator(Specialrangeiterator *sri);

#endif
