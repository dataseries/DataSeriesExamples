// 
// (c) Copyright 2012, Hewlett-Packard Development Company, LP
// 
//  See the file named COPYING for license details
//
// Simple C implementation
static const char rcsid[] =
"$Id: tricorder.c,v 1.6 2012/01/23 23:12:39 kterence Exp $";

/* count number of triangles in undirected graph */

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/time.h>

#include <valgrind/callgrind.h>

#define P (void)printf
#define T(msg) do { struct timeval tv; \
                    int r = gettimeofday(&tv, NULL);  if (0 != r) return 1; \
                    P("%s %ld.%06ld\n", (msg), tv.tv_sec, tv.tv_usec); \
                  } while (0)

enum { maxN =  5 * 1000 * 1000, maxE = 43 * 1000 * 1000 };

static int32_t nlo[maxN], fni[maxN], nbr[maxE];
static struct { int32_t a, b; } raw[maxE];

int main(int argc, char *argv[]) {
  int32_t i, j, k, nE = 0, a, b, c, d, NIDmax = 0, fnis = 0, t;
  int64_t nT = 0;
  if (NULL != argv[argc]) return 2;  /* suppress unused var warnings */

  CALLGRIND_STOP_INSTRUMENTATION;
  P("output of %s\n", rcsid);

  T("start first pass: parse & compute degree");
  while (2 == scanf("%" SCNd32 " %" SCNd32 "\n", &a, &b)) {
    if (a < b) {
      assert(maxE > nE);
      assert(0 <= a && maxN > a);
      assert(0 <= b && maxN > b);
      raw[nE].a = a;
      raw[nE].b = b;
      nE++;
      assert(INT32_MAX > nlo[b]);
      nlo[a]++;
      if (a > NIDmax)
        NIDmax = a;
    }
  }
  assert(1 < NIDmax);  /* need 3 nodes to make a triangle */
  assert(1 + NIDmax < maxN);
  P("max node ID: %" PRId32 "\n", NIDmax);
  P("# edges: %" PRId32 "\n", nE);

  T("update `first node index': index of first neighbor in nbr[]");
  for (i = 0; i <= NIDmax; i++) {
    fni[i] = fnis;
    fnis += nlo[i];
  }
  fni[1 + NIDmax] = fnis;
  assert(fnis == nE);

  int32_t edges = 0;
  T("second pass: read edges into nbr[]");
  for (i = 0; i < nE; i++) {
    a = raw[i].a;
    b = raw[i].b;
    /* add b to a's lo list */
    nlo[a]--;
    assert(0 <= nlo[a]);
    t = fni[a] + nlo[a];
    assert(0 <= t && maxE > t);
    nbr[t] = b;
    ++edges;
  }

  P("# edges: %" PRId32 "\n", edges);
  T("clear nlo[] for use to compute set intersections");
  for (i = 0; i <= NIDmax; i++)
    nlo[i] = 0;

  /* count triangles:  for every node pair a,b where b is a lower
     neighbor of a, does the set of a's lower neighbors intersect
     with the set of b's lower neighbors?  every node at intersection
     creates a triangle */
  T("start counting triangles");
  CALLGRIND_ZERO_STATS;
  CALLGRIND_START_INSTRUMENTATION;
  uint64_t outer = 0;
  uint64_t ncheck = 0;
  for (a = 0; a <= NIDmax; a++) {
    if (2 <= fni[1+a] - fni[a]) {
      for (i = fni[a]; i < fni[1+a]; i++) {
          ++outer;
        b = nbr[i];
        assert(0 <= b && a < b);
        /* set nlo[c] = 1 for every c that is a neighbor of b */
        for (j = fni[b]; j < fni[1+b]; j++) {
          c = nbr[j];
          nlo[c] = 1;
        }
        /* test nlo[d] for every d that is neighbor of a */
        for (k = fni[a]; k < fni[1+a]; k++) {
            ++ncheck;
          d = nbr[k];
          if (0 != nlo[d])
            nT++;
        }
        /* clear nlo[c] = 0 for every c that is a neighbor of b */
        for (j = fni[b]; j < fni[1+b]; j++) {
          c = nbr[j];
          nlo[c] = 0;
        }
      }
    }
    P("# checks: %" PRId64 " ; outer %" PRId64 "\n", ncheck, outer);
  }
  CALLGRIND_STOP_INSTRUMENTATION;

  P("# checks: %" PRId64 " ; outer %" PRId64 "\n", ncheck, outer);
  P("# triangles: %" PRId64 "\n", nT);
  T("done");
  return 0;
}
