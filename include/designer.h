/**
 * @file designer.h
 * @brief ARP interleaver design algorithm.
 *
 * The design algorithm searches for an ARP interleaver shift vector S[]
 * that maximises the minimum Hamming distance (mHD) of the resulting
 * turbo code.
 *
 * The algorithm proceeds layer by layer (one entry of S[] at a time):
 *
 *  1. Choose a candidate period P from the set of valid periods.
 *  2. For each sub-layer l = 0 … Q-1:
 *       a. Try candidate shift values S[l] in [0, K).
 *       b. Evaluate the partial interleaver using the RTZ search kernels.
 *       c. Keep only candidates that do not produce a codeword below the threshold.
 *  3. Repeat with an incremented threshold until MAX_INTERLEAVERS are found.
 *
 * The two main entry points are:
 *   - Determine_Best_Interleaver_Design(): full automated search loop.
 *   - Test_Minimum_Distance(): evaluate a given (S, P) pair.
 */

#ifndef DESIGNER_H
#define DESIGNER_H

/* =========================================================================
 * Top-level design entry points
 * ========================================================================= */

/**
 * Run the full interleaver design search.
 *
 * Iterates over valid periods, then layer by layer builds the shift vector
 * S[] that achieves a minimum distance >= threshold.
 *
 * @param fileoutput        Path to write candidate interleavers (e.g. "result.txt").
 * @param finalname         Path to write the best interleaver found (e.g. "final_result.txt").
 * @param fileinput         Path to the precomputed RTZ pattern file (used when type_of_search=0 or 2).
 * @param threshold         Minimum acceptable distance.
 * @param with_shuffle      1 = randomise period and shift search order, 0 = deterministic.
 * @param print             1 = verbose output to stdout, 0 = silent.
 * @param max_size_per_layer Maximum number of valid shift candidates to keep per layer.
 * @param distribute_layers 1 = pre-distribute the RTZ file by layer, 0 = use as-is.
 * @param type_of_search    0 = file-only, 1 = cycle-only, 2 = combined.
 */
void Determine_Best_Interleaver_Design(const char *fileoutput,
                                       const char *finalname,
                                       const char *fileinput,
                                       int threshold, int with_shuffle,
                                       int print, int max_size_per_layer,
                                       int distribute_layers,
                                       int type_of_search);

/**
 * Verify the minimum distance of a given ARP interleaver (S, P).
 * Prints results to stdout.
 *
 * @param s          Shift vector S[], length Q.
 * @param p          Period P.
 * @param threshold  Distance threshold for reporting.
 */
void Test_Minimum_Distance(int s[], int p, int threshold);


/* =========================================================================
 * Internal design helpers (exposed for advanced use)
 * ========================================================================= */

/**
 * Recursive layer-by-layer design.
 * Builds S[layer], S[layer+1], …, S[Q-1] and writes valid results.
 */
void Determine_ALL_Layers_With_Punc(const char *filename,
                                    const char *finalname,
                                    int p, int S[],
                                    int dmin, int multiplicity, int layer,
                                    int threshold, int with_shuffle,
                                    int print, int max_size_layer,
                                    int type_of_search);

/**
 * Find valid shift values for S[layer] and store them in array_s[].
 * Also records the minimum distance and multiplicity for each candidate.
 */
void Add_Layer_With_Shuffle_WIth_Punc(const char *filename, int p, int S[],
                                      int layer, int array_s[],
                                      int minimum_distances[],
                                      int distance_duplicate[],
                                      int *size, int threshold,
                                      int with_shuffle, int max_size_layer,
                                      int type_of_search);

/**
 * Compute mHD and multiplicity for a given (S, P) using the RTZ file.
 */
void Calculate_Min_Distance_And_Multiplicity(const char *filename,
                                             int s[], int p, int threshold,
                                             int *mydmin, int *mymul);

#endif /* DESIGNER_H */
