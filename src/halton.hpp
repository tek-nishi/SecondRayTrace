
#pragma once

#include "defines.hpp"
#include <cmath>
#include <algorithm>


//
// Halton 列生成
//

namespace {


// Halton列で使う素数
const int primes[100] = {
  2,   3,   5,   7,  11,  13,  17,  19,  23,  29,   /*  10 */
  31,  37,  41,  43,  47,  53,  59,  61,  67,  71,  /*  20 */
  73,  79,  83,  89,  97, 101, 103, 107, 109, 113,  /*  30 */
  127, 131, 137, 139, 149, 151, 157, 163, 167, 173, /*  40 */ 
  179, 181, 191, 193, 197, 199, 211, 223, 227, 229, /*  50 */
  233, 239, 241, 251, 257, 263, 269, 271, 277, 281, /*  60 */
  283, 293, 307, 311, 313, 317, 331, 337, 347, 349, /*  70 */
  353, 359, 367, 373, 379, 383, 389, 397, 401, 409, /*  80 */
  419, 421, 431, 433, 439, 443, 449, 457, 461, 463, /*  90 */
  467, 479, 487, 491, 499, 503, 509, 521, 523, 541  /* 100 */
};


/*
 * Function: faure_permutation
 *
 *     Generates the sequence of permutation up to n dimension
 *     using Faure's permutation method.
 *
 * Parameters:
 *
 *      n - Maximum dimension to generate.
 *
 * Returns:
 *
 *      The sequence of permutation.
 *
 * Reference:
 *
 *     - Faure Henri,
 *       Good permutations for extreme discrepancy,
 *       J. Number Theory 42, no. 1, 47--56, 1992
 *
 * Note:
 *
 *     Permutations up to n = 8.
 *
 *     - p2 = (0, 1)
 *     - p3 = (0, 1, 2)
 *     - p4 = (0, 2, 1, 3)
 *     - p5 = (0, 3, 2, 1, 4)
 *     - p6 = (0, 2, 4, 1, 3, 5)
 *     - p7 = (0, 2, 5, 3, 1, 4, 6)
 *     - p8 = (0, 4, 2, 6, 1, 5, 3, 7)
 *
 */
std::vector<std::vector<int> > faurePermutation(const int n) {
  assert(n > 1);

	/* Allocate memory for permutation table. */
  std::vector<std::vector<int> > p(n + 1);
	for (int i = 1; i < n + 1; i++) {
    p[i].resize(i + 1);
	}

	/* start with identity mapping in p_{2} = (0,1). */
	p[1][0] = 0;
	p[1][1] = 1;

	for (int i = 3; i < n + 1; i++) {
		if (i % 2 != 0) {	/* odd */ 
			/* first (i - 1) / 2 index */
			for (int j = 0; j < (i - 1) / 2; j++) {
				/*
				 * if p_{i-1}(j) >= (i - 1) / 2
				 *     p_{i}(j) = p_{i-1}(j) + 1
				 * else
				 *     p_{i}(j) = p_{i-1}(j)
				 */
				if (2 * p[i - 2][j] >= i - 1) { 
					p[i - 1][j] = p[i - 2][j] + 1;
				} else {
					p[i - 1][j] = p[i - 2][j];
				}
			}

			/* insert the value c into the center index */
			int c = (int)((i - 1) / 2);
			p[i - 1][c] = c;

			/* last (i - 1)/2 + 1 index */
			for (int j = (i - 1) / 2 + 1; j < i; j++) {
				/*
				 * if p_{i-1}(j-1) >= (i - 1) / 2
				 *     p_{i}(j) = p_{i-1}(j-1) + 1
				 * else
				 *     p_{i}(j) = p_{i-1}(j-1)
				 */
				if (2 * p[i - 2][j - 1] >= i - 1) { 
					p[i - 1][j] = p[i - 2][j - 1] + 1;
				} else {
					p[i - 1][j] = p[i - 2][j - 1];
				}
			}
		} else {		/* even */
			/* Generate first i/2 values taking
			 * p_{i}(j) = 2 p_{i/2}(j)
			 */
			for (int j = 0; j < i / 2; j++) {
				p[i - 1][j] = 2 * p[i / 2 - 1][j];
			}

			/* Replicate the sequence above adding
			 * +1 for each elements and append to the last.
			 */
			for (int j = i / 2; j < i; j++) {
				p[i - 1][j] = p[i - 1][j - i / 2] + 1;
			}
		}
	}

	return p;
}


class Halton {
  int offset_;
  const std::vector<std::vector<int> >& perm_table_;

  
public:
  explicit Halton(const std::vector<std::vector<int> >& perm_table, const int offest = 0) :
    offset_(0),
    perm_table_(perm_table)
  {}


  void offset(const int value) {
    offset_ = value;
  }
  
  
  Real operator()(const int dim) {
    assert(dim < 100);

    return vdC(offset_, primes[dim]);
  }

  
  Real scrambled(const int dim) {
    assert(dim < 100);
           
    /* dim'th prime number. */
    return (dim) ? generalizedVdC(offset_, primes[dim], perm_table_)
                 : vdC(offset_, primes[dim]);
  }

  
private:
  Real vdC(int i, const int base) {
    Real h = 0.0;
    Real f, factor;

    f = factor = 1.0 / base;

    while (i > 0) {
      h += (i % base) * factor;
      i /= base;
      factor *= f;
    }
    
    return h;
  }
  
  Real generalizedVdC(int i, const int base,
                      const std::vector<std::vector<int> >& p) {
    Real h = 0.0;
    Real f, factor;
	
    f = factor = 1.0 / base;

    while (i > 0) {
      /* Lookup permutation table. */
      int perm = p[base - 1][i % base];

      h += perm * factor;
      i /= base;
      factor *= f;
    }
  
    return h;
  }
  
};

}
