/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include "libgtcore/env.h"
#include "libgtcore/str.h"
#include "types.h"
#include "arraydef.h"
#include "chardef.h"
#include "alphadef.h"

#include "guessprot.pr"
#include "scanpaths.pr"
#include "readnextline.pr"

 struct _Alphabet
{
  Uchar characters[UCHAR_MAX+1],     /* array of characters to show */
        mapdomain[UCHAR_MAX+1],      /* list of characters mapped */
        symbolmap[UCHAR_MAX+1];      /* mapping of the symbols */
  unsigned int domainsize,           /* size of domain of symbolmap */
               mapsize,              /* size of image of map, i.e. */
                                     /* mapping to [0..mapsize-1] */
               mappedwildcards;      /* number of mapped wildcards */
};

/*EE
  This file implements the datatype \texttt{alphabet}.
*/

/*
  Some constants for the standard alphabet used. The name says it all.
*/

#define DNABASES                     "aAcCgGtTuU"
#define DNAWILDCARDS                 "nsywrkvbdhmNSYWRKVBDHM"
#define MAPSIZEDNA                   ((unsigned int) 5)
#define DNAALPHABETDOMAIN            DNABASES DNAWILDCARDS
#define PROTEINUPPERAMINOACIDS       "LVIFKREDAGSTNQYWPHMC"
#define PROTEINLOWERAMINOACIDS       "lvifkredagstnqywphmc"
#define MAPSIZEPROTEIN               ((unsigned int) 21)
#define PROTEINWILDCARDS             "XUBZ*-"
#define PROTEINALPHABETDOMAIN        PROTEINUPPERAMINOACIDS PROTEINWILDCARDS

/*
  We use the following macro to access the \texttt{I}-th character of
  a line.
*/

#define LINE(I)          line.spaceUchar[I]

/*EE
  We have developed a simple format to specify an alphabet
  and a corresponding alphabet transformation. This format specifies the
  characters of the alphabet (including wild card characters)
  and the pairs of symbols which are to be considered identical.
  The format is best explained by some examples. Consider a file
  containing the following lines:
  \begin{alltt}
  aA
  cC
  gG
  tTuU
  nsywrkvbdhmNSYWRKVBDHM
  \end{alltt}
  These line specify that the input sequence
  are allowed to contain the symbols \(a,c,g,t,u,n,s,y\)
  \(w,r,k,v,b,d,h,m\)
  in either lower or upper case. Moreover, the first four lines specify that
  \(a=A\), \(c=C\), \(g=G\), and \(t=T=u=U\). The last line specifies the
  wildcard symbols, which are replaced by unique symbols. Note that any
  wildcard symbol does not even match itself, if it occurs at different
  positions. Thus no false matches will be delivered. Note that however,
  a degenerate match may contain a wildcard, since this always leads
  to a mismatch.

  Consider a file containing the following lines:
  \begin{alltt}
  LVIF i
  KR +
  ED -
  AG s
  ST o
  NQ n
  YW a
  P p
  H h
  M m
  C c
  XBZ* x
  \end{alltt}
  This specifies the Protein alphabet
  \(L,V,I,F,K,R,E,D,A,G,S,T,N,Q,Y,W,P,H,M,C\) with some extra symbols
  \(X,U,B,Z,\ast\). All symbols occurring on the same line to the left of
  the first white space are considered to be pairwise equivalent. The symbol
  after the first white can be considered to be a comment. The symbols on
  the last line are considered to be wildcards. Again they are replaced
  by a unique character. This is the given parameter wildcard, if
  wildcard $>$ 0.
*/

/*
  The following function reads a file in the format as explained above.
  \texttt{mapfile} is the input filename. \texttt{fpin} is the corresponding
  file name. If the argument
  \texttt{wildcard} is larger than 0, then the characters  in the last
  line of the symbol mapping file are mapped to \texttt{wildcard}. Otherwise,
  they are mapped to the value one smaller than the line number they appear
  in (counting from 1). The result of the parsing is stored in
  \texttt{alpha}.
*/

static int readsymbolmapviafp(Alphabet *alpha,
                              Uchar wildcard,
                              const Str *mapfile,
                              FILE *fpin,
                              Env *env)
{
  Uchar cc;
  unsigned cnum, linecount = 0;
  ArrayUchar line;
  Uint column;
  bool blankfound, ignore, preamble = true;

  env_error_check(env);
  alpha->domainsize = alpha->mapsize = alpha->mappedwildcards = 0;
  for (cnum=0; cnum<=UCHAR_MAX; cnum++)
  {
    alpha->symbolmap[cnum] = (Uchar) UNDEFCHAR;
  }
  INITARRAY(&line,Uchar);
  while (true)
  {
    line.nextfreeUchar = 0;
    if (readnextline(fpin,&line,env) == EOF)
    {
      break;
    }
    linecount++;
    ignore = false;
    if (line.nextfreeUchar > 0)
    {
      assert(line.spaceUchar != NULL);
      if (preamble)
      {
        if (LINE(0) == (Uchar) '#')
        {
          ignore = true;
        } else
        {
          preamble = false;
        }
      }
      if (!ignore)
      {
        blankfound = false;
        for (column=0; column<line.nextfreeUchar; column++) 
        { /* for all chars in line */
          cc = LINE(column);
          if (ispunct((Ctypeargumenttype) cc) ||
              isalnum((Ctypeargumenttype) cc))
          {
            if (alpha->symbolmap[(unsigned int) cc] != (Uchar) UNDEFCHAR)
            {
              env_error_set(env,"cannot map symbol '%c' to %lu: "
                            "it is already mapped to %lu",
                             cc,
                             (Showuint) alpha->mapsize,
                             (Showuint) alpha->symbolmap[(unsigned int) cc]);
              return -1;
            }
            /* get same value */
            alpha->symbolmap[(unsigned int) cc] = (Uchar) alpha->mapsize;
            alpha->mapdomain[alpha->domainsize++] = cc;
          } else
          {
            if (cc == (Uchar) ' ')    /* first blank in line found */
            {
              blankfound = true;
              /*@innerbreak@*/ break;
            }
            env_error_set(env,
                          "illegal character '%c' in line %u of mapfile %s",
                          cc,linecount,str_get(mapfile));
            return -2;
          }
        }
        if (blankfound)
        {
          if (isspace((Ctypeargumenttype) LINE(column+1)))
          {
            env_error_set(env,
                          "illegal character '%c' at the end of "
                          "line %u in mapfile %s",
                          LINE(column+1),linecount,str_get(mapfile));
            return -3;
          }
          /* use next character to display character */
          alpha->characters[alpha->mapsize++] = LINE(column+1);
        } else
        {
          /* use first character of line to display character */
          alpha->characters[alpha->mapsize++] = LINE(0);
        }
      }
    }
  }
  for (cnum=0;cnum<=UCHAR_MAX; cnum++)
  {
    if (alpha->symbolmap[cnum] == (Uchar) (alpha->mapsize - 1))
    {
      if (wildcard > 0)
      {
        alpha->symbolmap[cnum] = wildcard; /* modify mapping for wildcard */
      }
      alpha->mappedwildcards++;
    }
  }
  if (wildcard > 0)
  {
    alpha->characters[(unsigned int) wildcard] 
      = alpha->characters[alpha->mapsize-1];
  }
  FREEARRAY(&line,Uchar);
  return 0;
}

/*EE
  The following function reads in a symbol map.
  \texttt{mapfile} is the input filename.
  If the argument
  \texttt{wildcard} is larger than 0, then the characters in the last
  line of the symbol mapping file are mapped to \texttt{wildcard}. Otherwise,
  they are mapped to \(i-1\) if they appear on line number \(i\)
  (counting from 1). The result of the parsing is stored in
  \texttt{alpha}.
*/

static int readsymbolmap(Alphabet *alpha,Uchar wildcard,
                         const Str *mapfile,Env *env)
{
  FILE *fpin;

  env_error_check(env);
  fpin = scanpathsforfile("MKVTREESMAPDIR",mapfile,env);
  if (fpin == NULL)
  {
    return -1;
  }
  if (readsymbolmapviafp(alpha,wildcard,mapfile,fpin,env) != 0)
  {
    return -2;
  }
  env_fa_xfclose(fpin,env);
  return 0;
}

static void assignDNAsymbolmap(Uchar *symbolmap)
{
  unsigned int cnum;

  for (cnum=0; cnum<=UCHAR_MAX; cnum++)
  {
    symbolmap[cnum] = (Uchar) UNDEFCHAR;
  }
  symbolmap[(unsigned int) 'a'] = (Uchar) 0;
  symbolmap[(unsigned int) 'A'] = (Uchar) 0;
  symbolmap[(unsigned int) 'c'] = (Uchar) 1;
  symbolmap[(unsigned int) 'C'] = (Uchar) 1;
  symbolmap[(unsigned int) 'g'] = (Uchar) 2;
  symbolmap[(unsigned int) 'G'] = (Uchar) 2;
  symbolmap[(unsigned int) 't'] = (Uchar) 3;
  symbolmap[(unsigned int) 'T'] = (Uchar) 3;
  symbolmap[(unsigned int) 'u'] = (Uchar) 3;
  symbolmap[(unsigned int) 'U'] = (Uchar) 3;
  for (cnum=0; DNAWILDCARDS[cnum] != '\0'; cnum++)
  {
    symbolmap[(unsigned int) DNAWILDCARDS[cnum]] = (Uchar) WILDCARD;
  }
}

/*EE
  The following function initializes the alphabet \texttt{alpha}
  in the same way as \texttt{readsymbolmap}, if it would be
  applied to a map file with the following content:
  \begin{alltt}
  aA
  cC
  gG
  tTuU
  nsywrkvbdhmNSYWRKVBDHM
  \end{alltt}
  If the argument \texttt{wildcard} is 0, then the wildcard characters
  in the last line are mapped to 4. Otherwise they are mapped to
  the character \texttt{WILDCARD}, as defined in \texttt{chardef.h}
*/

static void assignDNAalphabet(Alphabet *alpha)
{
  alpha->domainsize = (unsigned int) strlen(DNAALPHABETDOMAIN);
  alpha->mappedwildcards = (unsigned int) strlen(DNAWILDCARDS);
  memcpy(alpha->mapdomain,
         (Uchar *) DNAALPHABETDOMAIN,
         (size_t) alpha->domainsize);
  alpha->mapsize = MAPSIZEDNA;
  memcpy(alpha->characters,"acgt",(size_t) (MAPSIZEDNA-1));
  alpha->characters[WILDCARD] = (Uchar) DNAWILDCARDS[0];
  alpha->characters[MAPSIZEDNA-1] = (Uchar) DNAWILDCARDS[0];
  assignDNAsymbolmap(alpha->symbolmap);
}

static void assignproteinsymbolmap(Uchar *symbolmap)
{
  unsigned int cnum;

  for (cnum=0; cnum<=UCHAR_MAX; cnum++)
  {
    symbolmap[cnum] = (Uchar) UNDEFCHAR;
  }
  for (cnum=0; PROTEINUPPERAMINOACIDS[cnum] != '\0'; cnum++)
  {
    symbolmap[(unsigned int) PROTEINUPPERAMINOACIDS[cnum]] = (Uchar) cnum;
  }
  for (cnum=0; PROTEINWILDCARDS[cnum] != '\0'; cnum++)
  {
    symbolmap[(unsigned int) PROTEINWILDCARDS[cnum]] = (Uchar) WILDCARD;
  }
}

/*EE
  The following function initializes the alphabet \texttt{alpha}
  in the same way as \texttt{readsymbolmap}, if it would be
  applied to a map file with the following content:
  \begin{alltt}
  L
  V
  I
  F
  K
  R
  E
  D
  A
  G
  S
  T
  N
  Q
  Y
  W
  P
  H
  M
  C
  XUBZ*-
  \end{alltt}
  If the argument \texttt{wildcard} is 0, then the wildcard characters
  in the last line are mapped to 20. Otherwise they are mapped to
  the character \texttt{WILDCARD}, as defined in \texttt{chardef.h}
*/

static void assignProteinalphabet(Alphabet *alpha)
{
  alpha->domainsize = (unsigned int) strlen(PROTEINALPHABETDOMAIN);
  alpha->mappedwildcards = (unsigned int) strlen(PROTEINWILDCARDS);
  memcpy(alpha->mapdomain,
         (Uchar *) PROTEINALPHABETDOMAIN,(size_t) alpha->domainsize);
  alpha->mapsize = MAPSIZEPROTEIN;
  memcpy(alpha->characters,PROTEINUPPERAMINOACIDS,(size_t) MAPSIZEPROTEIN-1);
  alpha->characters[WILDCARD] = (Uchar) PROTEINWILDCARDS[0];
  alpha->characters[MAPSIZEPROTEIN-1] = (Uchar) PROTEINWILDCARDS[0];
  assignproteinsymbolmap(alpha->symbolmap);
}

static void assignProteinorDNAalphabet(Alphabet *alpha,const char *inputfile)
{
  if (guessifproteinsequencestream(inputfile))
  {
    assignProteinalphabet(alpha);
  } else
  {
    assignDNAalphabet(alpha);
  }
}

/*@null@*/ Alphabet *assigninputalphabet(bool isdna,
                                         bool isprotein,
                                         const Str *smapfile,
                                         const char *sequencefilename,
                                         Env *env)
{
  Alphabet *alpha;

  env_error_check(env);
  ALLOCASSIGNSPACE(alpha,NULL,Alphabet,(size_t) 1);
  if (isdna)
  {
    assignDNAalphabet(alpha);
  } else
  {
    if (isprotein)
    {
      assignProteinalphabet(alpha);
    } else
    {
      if (str_length(smapfile) > 0)
      {
        if (readsymbolmap(alpha,
                         (Uchar) WILDCARD,
                         smapfile,
                         env) != 0)
        {
          return NULL;
        }
      } else
      {
        assignProteinorDNAalphabet(alpha,sequencefilename);
      }
    }
  }
  return alpha;
}

const Uchar *getsymbolmapAlphabet(const Alphabet *alpha)
{
  return alpha->symbolmap;
}

unsigned int getnumofcharsAlphabet(const Alphabet *alpha)
{
  return alpha->mapsize-1;
}

unsigned int getmapsizeAlphabet(const Alphabet *alpha)
{
  return alpha->mapsize;
}

const Uchar *getcharactersAlphabet(const Alphabet *alpha)
{
  return alpha->characters;
}

Uchar *copycharactersAlphabet(const Alphabet *alpha,Env *env)
{
  Uchar *characters;

  ALLOCASSIGNSPACE(characters,NULL,Uchar,alpha->domainsize);
  (void) memcpy(characters,alpha->characters,(size_t) alpha->domainsize);
  return characters;
}

void freeAlphabet(Alphabet **alpha,Env *env)
{
  FREESPACE(*alpha);
}

void outputalphabet(FILE *fpout,const Alphabet *alpha)
{
  Uchar currentcc, previouscc = 0, firstinline = 0;
  unsigned int cnum, linenum = 0;
  bool afternewline = true;

  for (cnum=0; cnum < alpha->domainsize; cnum++)
  {
    currentcc = alpha->mapdomain[cnum];
    if (cnum > 0)
    {
      if (alpha->symbolmap[currentcc] != alpha->symbolmap[previouscc])
      {
        if (firstinline != alpha->characters[linenum])
        {
          fprintf(fpout," %c",alpha->characters[linenum]);
        }
        (void) putc('\n',fpout);
        afternewline = true;
        linenum++;
      } else
      {
        afternewline = false;
      }
    }
    (void) putc((Fputcfirstargtype) currentcc,fpout);
    if (afternewline)
    {
      firstinline = currentcc;
    }
    previouscc = currentcc;
  }
  if (firstinline != alpha->characters[linenum])
  {
    fprintf(fpout," %c",alpha->characters[linenum]);
  }
  (void) putc((Fputcfirstargtype) '\n',fpout);
}
