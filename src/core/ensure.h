/*
  Copyright (c) 2007-2009 Gordon Gremme <gordon@gremme.org>
  Copyright (c)      2013 Dirk Willrodt <willrodt@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

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

#ifndef ENSURE_H
#define ENSURE_H

#include <stdbool.h>
#include "core/error.h"

/* the ensure macro used for unit tests */
#define gt_ensure(expr)                                                  \
  do {                                                                   \
    if (!had_err) {                                                      \
      if (!(expr)) {                                                     \
        gt_error_set(err, "gt_ensure(%s) failed: function %s, file %s, " \
                     "line %d.\nThis is probably a bug, please report "  \
                "at https://github.com/genometools/genometools/issues.", \
                     #expr, __func__, __FILE__, __LINE__);               \
        had_err = -1;                                                    \
      }                                                                  \
    }                                                                    \
  } while (false)
#endif
