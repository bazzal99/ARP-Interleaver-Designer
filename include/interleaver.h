/**
 * @file interleaver.h
 * @brief ARP (Almost Regular Permutation) interleaver construction.
 *
 * The ARP interleaver is defined by:
 *   π(i) = (P * i + S[i mod Q]) mod K
 *
 * where P is the period, Q is the number of sub-layers, S[] is the
 * shift vector (one entry per sub-layer), and K = FRAME_SIZE.
 *
 * Three initialisation modes are provided to support the layer-by-layer
 * interleaver design algorithm:
 *   - Full:      initialise all K positions at once.
 *   - Till layer: initialise sub-layers 0 … layer (inclusive).
 *   - Per layer:  initialise only sub-layer `layer`.
 *
 * A de-initialisation function (Arp_DE_Interleave_Layer) is provided
 * to undo a single sub-layer, used to backtrack during the design search.
 */

#ifndef INTERLEAVER_H
#define INTERLEAVER_H

/* =========================================================================
 * ARP interleaver initialisation
 * ========================================================================= */

/**
 * Fully initialise an ARP interleaver for all K positions.
 *
 * @param Size              Frame length K.
 * @param Period            ARP period P (must be coprime with K).
 * @param PeriodARP         Sub-layer period Q.
 * @param Shift             Shift vector S[], length Q.
 * @param InterleavedAddress Output: inverse map π⁻¹[i] = j such that π(j)=i.
 * @param OriginalAddress    Output: forward map π[i] = (P*i + S[i%Q]) mod K.
 */
void ARPInterleaverInitialization(int Size, int Period, int PeriodARP,
                                  int Shift[],
                                  int InterleavedAddress[],
                                  int OriginalAddress[]);

/**
 * Initialise sub-layers 0 through `layer` of the ARP interleaver.
 * Positions belonging to sub-layers > layer remain at -1 (undefined).
 *
 * Used during the design search to evaluate partial interleavers.
 */
void ARPInterleaverInitialization_Till_Layer(int Size, int layer, int Period,
                                             int Q, int Shift[],
                                             int InterleavedAddress[],
                                             int OriginalAddress[]);

/**
 * Initialise only sub-layer `layer` of the ARP interleaver.
 * All other positions are left unchanged.
 *
 * Used to add one sub-layer at a time during the design search.
 */
void ARPInterleaverInitialization_Per_Layer(int Size, int layer, int Period,
                                            int Q, int Shift[],
                                            int InterleavedAddress[],
                                            int OriginalAddress[]);

/**
 * Undo (de-initialise) sub-layer `layer`, setting its positions back to -1.
 *
 * Used to backtrack when a candidate shift value S[layer] is rejected.
 */
void Arp_DE_Interleave_Layer(int Size, int layer, int Period, int Q,
                             int Shift[],
                             int InterleavedAddress[],
                             int OriginalAddress[]);


/* =========================================================================
 * Period selection
 * ========================================================================= */

/**
 * Find all candidate ARP periods for a given frame size.
 *
 * A period p is accepted if:
 *   - gcd(p, K) == 1  (coprimality)
 *   - Min_Spread(p) >= sqrt(2*K) - SPREAD_OFFSET  (spread criterion)
 *
 * @param periods   Output array of valid periods.
 * @param size      Output: number of valid periods found.
 * @param framesize Frame length K.
 */
void Choose_Period(int periods[], int *size, int framesize);

/**
 * Compute the minimum spread of a linear permutation π(i) = P*i mod K.
 *
 * The spread is defined as:
 *   smin = min_{i,j} (|j| + |π(i) - π(i+j)|)
 *
 * @param p  Period P.
 * @return   Minimum spread value.
 */
int Min_Spread(int p);

/** Return 1 if gcd(a, b) == 1, 0 otherwise. */
int isRelativePrime(int a, int b);

/** Compute gcd(a, b) using Euclid's algorithm. */
int gcd(int a, int b);

/** Fill `result` with all integers coprime to `number` in [1, number]. */
void saveRelativePrimes(int number, int *result, int *count);

/** Evaluate the ARP interleaver at position i: π(i) = (p*i + S[i%Q]) % K. */
int inter(int i, int p, int s[]);

#endif /* INTERLEAVER_H */
