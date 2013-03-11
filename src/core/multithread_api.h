/*
  Copyright (c) 2010, 2013 Gordon Gremme <gremme@zbh.uni-hamburg.de>

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

#ifndef MULTITHREAD_API_H
#define MULTITHREAD_API_H

#include "core/error_api.h"
#include "core/thread_api.h"

/* Multithread module */

extern unsigned int gt_jobs; /* number of parallel threads to be used */

/* Execute <function> (with <data> passed to it) in <gt_jobs> many parallel
   threads, if threading is enabled. Otherwise <function> is executed <gt_jobs>
   many times sequentially. <gt_jobs> is a global <unsigned int> variable. */
int       gt_multithread(GtThreadFunc function, void *data, GtError *err);

#endif