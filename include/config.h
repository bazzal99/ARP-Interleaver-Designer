/**
 * @file config.h
 * @brief Global encoder and interleaver configuration parameters.
 *
 * Edit this file to change the frame size, encoder polynomials,
 * puncturing pattern, and search hyperparameters before compiling.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* =========================================================================
 * Encoder configuration
 * ========================================================================= */

/**
 * Frame length K in bits.
 * Must satisfy K % Q == 0 for ARP interleaver compatibility.
 */
#define FRAME_SIZE  6144

/**
 * Number of memory elements (delays) in the RSC encoder.
 * Determines the number of trellis states: 2^DELAYS.
 */
#define DELAYS  3

/**
 * RSC generator polynomials (binary, MSB = feedback tap).
 * G1 = feedforward, G2 = feedback.
 * Default: G(1, 13/15) — same family as the LTE RSC encoder.
 */
#define G1_POLY  {1, 1, 0, 1}
#define G2_POLY  {1, 0, 1, 1}

/**
 * Period of the RSC encoder's free impulse response.
 * For the default G(1, 13/15) encoder this is 7.
 */
#define ENCODER_PERIOD  7

/**
 * Encoding mode:
 *   1 = Circular / tail-biting (TB)
 *   0 = Zero-termination (ZT)
 */
#define CIRCULAR_ENCODING  1

/**
 * Append tail bits to force the encoder back to state 0 (ZT mode only).
 * Set to 1 only when CIRCULAR_ENCODING == 0.
 */
#define TAIL_BITS  0


/* =========================================================================
 * Puncturing pattern
 * ========================================================================= */

/**
 * Puncturing mask period (number of bits in one puncturing period).
 * The mask repeats every PUNCT_MASK bits.
 */
#define PUNCT_MASK  8

/**
 * Puncturing patterns for the first and second RSC encoders.
 * 1 = keep bit, 0 = puncture.
 * Must have PUNCT_MASK entries each.
 */
#define PUNCT_PATTERN_1   {0, 1, 0, 0, 0, 0, 0, 0}
#define PUNCT_PATTERN_2   {0, 1, 0, 0, 0, 0, 0, 0}


/* =========================================================================
 * ARP interleaver configuration
 * ========================================================================= */

/**
 * ARP interleaver period Q (number of sub-layers).
 * Must divide FRAME_SIZE.
 */
#define ARP_Q  8

/**
 * Distance threshold for the interleaver design search.
 * The algorithm will accept an interleaver only if its estimated mHD
 * meets or exceeds this value.
 */
#define DESIGN_THRESHOLD  13

/**
 * Minimum spread threshold offset.
 * A period p is accepted if Min_Spread(p) >= sqrt(2*K) - SPREAD_OFFSET.
 */
#define SPREAD_OFFSET  2


/* =========================================================================
 * RTZ search hyperparameters (circular / TB mode)
 * ========================================================================= */

/** Girth bounds for weight-2 periodic RTZ search. */
#define IW2_GIRTH_BOUNDS   {2, 4, 6, 8}

/** Half-period budgets for weight-2 periodic RTZ search. */
#define IW2_PERIOD_BUDGETS {10, 10, 7, 4}

/** Maximum input weight tested by the iw=2 kernel. */
#define IW2_MAX_WEIGHT  4

/** Girth bounds for weight-3+ general RTZ search. */
#define IWM_GIRTH_BOUNDS   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

/** Step budgets for weight-3+ general RTZ search. */
#define IWM_STEP_BUDGETS   {-1, -1, -1, 50, 28, 15, 10, 7, 5, 4}

/** Maximum input weight tested by the general RTZ kernel. */
#define IWM_MAX_WEIGHT  9


/* =========================================================================
 * Interleaver design search parameters
 * ========================================================================= */

/**
 * Maximum number of candidate shift values S[layer] to keep per layer.
 * Higher = more thorough search, longer runtime. Recommended: 1–10.
 */
#define MAX_CANDIDATES_PER_LAYER  1

/**
 * Maximum number of valid interleavers to find before stopping.
 */
#define MAX_INTERLEAVERS  1

/**
 * Search strategy:
 *   0 = File-based only (use precomputed RTZ file)
 *   1 = Cycle-based only (live RTZ graph search)
 *   2 = Both combined
 */
#define SEARCH_TYPE  1

/**
 * Shuffle the period and shift candidates randomly.
 *   1 = randomised search (recommended)
 *   0 = deterministic, start from 0
 */
#define WITH_SHUFFLE  1

/**
 * Print search progress to stdout.
 *   1 = verbose, 0 = silent
 */
#define VERBOSE  1

#endif /* CONFIG_H */
