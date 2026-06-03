/**
 * @file rtz_search.h
 * @brief RTZ graph-girth search kernels for ARP interleaver distance estimation.
 *
 * Three kernel families are provided:
 *
 *  IW2 kernel (Min_Girth_One_Layer_RTZ_IW2_Without_Circular):
 *    Exploits the periodic structure of weight-2 RTZ sequences.
 *    Steps by multiples of ENCODER_PERIOD. Symmetry-pruned (ref >= the_ref).
 *    Used for input weights iw = 2, 4, 6, ...
 *
 *  LTE kernel (Min_Girth_One_Layer_LTE):
 *    General unit-step search. Supports both circular and ZT modes
 *    (selected at runtime via CIRCULAR_ENCODING).
 *    Skips positions with undefined interleaver entries (value == -1),
 *    enabling use with partially-initialised ARP interleavers.
 *    Used for input weights iw = 3, 4, 5, ...
 *
 *  Legacy IW2 / RTZ-Multiple kernels (Min_Distance_One_Layer_*):
 *    Earlier versions retained for comparison purposes.
 *    Use the IW2 and LTE kernels for new work.
 *
 * All kernels write candidate minimum-distance codewords to "temp.txt".
 * The caller is responsible for clearing and reading this file.
 */

#ifndef RTZ_SEARCH_H
#define RTZ_SEARCH_H

/* =========================================================================
 * Primary kernels (used by the interleaver design algorithm)
 * ========================================================================= */

/**
 * Search for weight-2 RTZ cycles from position `layer` (ZT/non-circular).
 *
 * @param layer        Starting position.
 * @param pi           ARP forward map  π[i].
 * @param depi         ARP inverse map  π⁻¹[i].
 * @param threshold    Current distance threshold.
 * @param mult         Multiplicity counter (updated in place).
 * @param max_RTZ_size Half-period budget (search depth in multiples of ENCODER_PERIOD).
 * @param mingirth     Maximum girth (cycle length) to explore.
 * @return             0 if a distance < threshold was found, 1 otherwise.
 */
int Min_Girth_One_Layer_RTZ_IW2_Without_Circular(int layer, int pi[], int depi[],
                                                  int threshold, int *mult,
                                                  int max_RTZ_size, int mingirth);

int Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(int layer, int pi[], int depi[],
                                                           int girth, int ref, int the_ref,
                                                           int starting, int path[], int pathLen,
                                                           int threshold, int *mult,
                                                           int max_RTZ_size, int mingirth);

/**
 * General RTZ cycle search from position `layer`.
 * Supports both circular and ZT modes (via CIRCULAR_ENCODING).
 * Skips positions where pi[ref] == -1 or depi[ref] == -1.
 *
 * @param layer        Starting position.
 * @param pi           ARP forward map.
 * @param depi         ARP inverse map.
 * @param threshold    Current distance threshold.
 * @param mult         Multiplicity counter.
 * @param max_RTZ_size Step budget.
 * @param mingirth     Maximum girth.
 * @return             0 if a distance < threshold was found, 1 otherwise.
 */
int Min_Girth_One_Layer_LTE(int layer, int pi[], int depi[],
                             int threshold, int *mult,
                             int max_RTZ_size, int mingirth);

int Min_Girth_One_Ref_With_Layer_LTE(int layer, int pi[], int depi[],
                                      int girth, int ref, int the_ref, int starting,
                                      int path[], int pathLen,
                                      int threshold, int *mult,
                                      int max_RTZ_size, int mingirth);


/* =========================================================================
 * Legacy kernels (retained for reference)
 * ========================================================================= */

int Min_Distance_One_Layer_IW2(int layer, int pi[], int depi[],
                                int threshold, int *mult,
                                int max_RTZ_size, int mingirth);

int Min_Distance_One_Ref_With_Layer_IW2(int layer, int pi[], int depi[],
                                         int girth, int ref, int the_ref, int starting,
                                         int path[], int pathLen,
                                         int threshold, int *mult,
                                         int max_RTZ_size, int mingirth);

int Min_Distance_One_Layer_RTZ_Multiple(int layer, int pi[], int depi[],
                                         int threshold, int *mult,
                                         int max_RTZ_size, int mingirth);

int Min_Distance_One_Ref_With_Layer_RTZ_Multiple(int layer, int pi[], int depi[],
                                                  int girth, int ref, int the_ref,
                                                  int starting, int path[], int pathLen,
                                                  int threshold, int *mult,
                                                  int max_RTZ_size, int mingirth);

#endif /* RTZ_SEARCH_H */
