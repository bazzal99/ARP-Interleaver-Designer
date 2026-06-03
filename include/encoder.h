/**
 * @file encoder.h
 * @brief RSC encoder, circular state initialisation, and distance computation.
 *
 * Implements a generic binary Recursive Systematic Convolutional (RSC)
 * encoder parameterised by generator polynomials G1/G2 and number of
 * delays DELAYS (defined in config.h).
 *
 * Two encoding modes are supported:
 *   - Circular / tail-biting (TB): CIRCULAR_ENCODING = 1
 *   - Zero-termination (ZT):       CIRCULAR_ENCODING = 0, TAIL_BITS = 0
 *   - Terminated (ZT + tail bits): CIRCULAR_ENCODING = 0, TAIL_BITS = 1
 */

#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"

/* =========================================================================
 * Global encoder state (populated by Initialize_States)
 * ========================================================================= */

/** Circular (tail-biting) starting states. CIRCULAR_STATES[end] = start. */
extern int CIRCULAR_STATES[32];

/** RSC feedback polynomial (binary coefficients). */
extern int G1[4];

/** RSC feedforward polynomial (binary coefficients). */
extern int G2[4];


/* =========================================================================
 * Initialisation
 * ========================================================================= */

/**
 * Find and store all circular (tail-biting) starting states.
 * Must be called once before any call to Encode() when CIRCULAR_ENCODING=1.
 */
void Initialize_States(void);


/* =========================================================================
 * Encoding
 * ========================================================================= */

/**
 * Encode one frame using the RSC encoder.
 *
 * In circular mode (CIRCULAR_ENCODING=1): runs the trellis twice to find
 * the correct starting state, then produces the parity output.
 * In ZT mode: starts from state 0.
 *
 * @param input   Binary input frame of length FRAME_SIZE.
 * @param parity  Output parity bits of length FRAME_SIZE.
 */
void Encode(int input[], int parity[]);

/**
 * Encode one frame with explicit tail bits (ZT mode, TAIL_BITS=1).
 *
 * @param input        Binary input frame of length FRAME_SIZE.
 * @param parity       Output parity bits of length FRAME_SIZE.
 * @param termination  Output tail bits (length 2*DELAYS).
 */
void Encode_Termination(int input[], int parity[], int termination[]);

/**
 * Encode `size` bits starting from a given state, using arbitrary G1/G2.
 *
 * @param prevstate  Encoder state before encoding (length `delays`).
 * @param input      Input bit array of length `size`.
 * @param size       Number of bits to encode.
 * @param g1         Feedforward polynomial (length delays+1).
 * @param g2         Feedback polynomial (length delays+1).
 * @param parity     Output parity bits (length `size`).
 * @param nextstate  Encoder state after encoding (length `delays`).
 * @param delays     Number of memory elements.
 */
void Conv_Encode_number_of_delays(int prevstate[], int input[], int size,
                                  int g1[], int g2[], int parity[],
                                  int nextstate[], int delays);

/**
 * Encode one bit and return the parity output.
 *
 * @param prevstate  Current encoder state (length `num_delays`).
 * @param input      Input bit (0 or 1).
 * @param g1         Feedforward polynomial.
 * @param g2         Feedback polynomial.
 * @param nextstate  Next encoder state (length `num_delays`).
 * @param num_delays Number of memory elements.
 * @return           Parity bit.
 */
int Encode_1_bit_number_of_delays(int prevstate[], int input, int g1[],
                                  int g2[], int nextstate[], int num_delays);


/* =========================================================================
 * Distance computation
 * ========================================================================= */

/**
 * Compute the Hamming weight of the turbo codeword for input pattern x[].
 *
 * Accounts for systematic bits, both RSC parity streams, and puncturing.
 *
 * @param x                Positions where the input equals 1.
 * @param size             Input weight (number of 1s).
 * @param Original_address ARP interleaver forward map.
 * @return                 Total codeword Hamming weight.
 */
int Turbo_distance(int x[], int size, int Original_address[]);

/**
 * Compute the parity weight of the second RSC encoder for input x[],
 * after applying the puncturing mask of type `type`.
 *
 * @param x     Positions where the input equals 1.
 * @param size  Input weight.
 * @param type  Puncturing type: 0 = PUNCT_PATTERN_1, 1 = PUNCT_PATTERN_2.
 * @return      Punctured parity weight.
 */
int parity_distance(int x[], int size, int type);

/**
 * Fill array `x` with the puncturing mask for a full frame.
 *
 * @param x          Output mask array of length `framesize`.
 * @param framesize  Frame length.
 * @param type       0 = PUNCT_PATTERN_1, 1 = PUNCT_PATTERN_2.
 */
void puncmask_all_frame_FrameSize(int x[], int framesize, int type);

/**
 * Fill array `x` with the puncturing mask for a window of `size` bits
 * starting at position `startfrom`.
 */
void puncmask_frame(int x[], int startfrom, int size, int type);

#endif /* ENCODER_H */
