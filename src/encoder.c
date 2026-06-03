/**
 * @file encoder.c
 * @brief RSC encoder, circular state initialisation, and distance computation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "encoder.h"
#include "utils.h"

/* =========================================================================
 * Global encoder tables
 * ========================================================================= */

int CIRCULAR_STATES[32];
int G1[] = G1_POLY;
int G2[] = G2_POLY;

/** Zero-terminated starting state (all zeros). */
static int STARTING_STATE[] = {0, 0, 0, 0, 0, 0};

/** Puncturing patterns (initialised from config.h macros). */
static int PUNCTURED[]    = PUNCT_PATTERN_1;
static int PUNCTURED_II[] = PUNCT_PATTERN_2;


/* =========================================================================
 * Circular state initialisation
 * ========================================================================= */

void Initialize_States(void)
{
    int i, j, BoolCircular;
    int d[FRAME_SIZE];
    int myparity[FRAME_SIZE];
    int prevstate[DELAYS];
    int nextstate[DELAYS];
    int nextstateint;

    /* Sentinel: all states unresolved except state 0 */
    CIRCULAR_STATES[0] = 0;
    for (i = 1; i < (int)pow(2, DELAYS); i++)
        CIRCULAR_STATES[i] = 9999;

    /*
     * Find all circular starting states by random probing.
     * For a random input d[], encode twice from state 0:
     *   pass 1: 0 -> mid_state
     *   pass 2: mid_state -> end_state
     * Then CIRCULAR_STATES[end_state] = mid_state.
     * Repeat until all 2^DELAYS states are covered.
     */
    do {
        BoolCircular = 1;
        generateRandomBinaryArray(d, FRAME_SIZE);

        int_to_binary(0, DELAYS, prevstate);
        Conv_Encode_number_of_delays(prevstate, d, FRAME_SIZE,
                                     G1, G2, myparity, nextstate, DELAYS);
        nextstateint = binary_to_int(nextstate, DELAYS);

        Conv_Encode_number_of_delays(nextstate, d, FRAME_SIZE,
                                     G1, G2, myparity, nextstate, DELAYS);
        CIRCULAR_STATES[binary_to_int(nextstate, DELAYS)] = nextstateint;

        for (j = 1; j < (int)pow(2, DELAYS); j++)
            if (CIRCULAR_STATES[j] == 9999) { BoolCircular = 0; break; }

    } while (BoolCircular == 0);
}


/* =========================================================================
 * Encoding
 * ========================================================================= */

int Encode_1_bit_number_of_delays(int prevstate[], int input, int g1[],
                                   int g2[], int nextstate[], int num_delays)
{
    int i, parity, x = 0;

    /* Compute feedback: x = input XOR sum(G2[i] * state[i-1]) */
    for (i = num_delays; i > 0; i--)
        x ^= g2[i] & prevstate[i - 1];
    x ^= input;

    /* Compute parity output */
    parity = g1[0] ? x : 0;
    nextstate[0] = x;
    for (i = 1; i < num_delays; i++)
        nextstate[i] = prevstate[i - 1];
    for (i = 1; i < num_delays + 1; i++)
        if (g1[i]) parity ^= prevstate[i - 1];

    return parity;
}

void Conv_Encode_number_of_delays(int prevstate[], int input[], int size,
                                   int g1[], int g2[], int parity[],
                                   int nextstate[], int delays)
{
    int i;
    int theprevstate[delays];
    copy_array(prevstate, theprevstate, delays);
    for (i = 0; i < size; i++) {
        parity[i] = Encode_1_bit_number_of_delays(theprevstate, input[i],
                                                   g1, g2, nextstate, delays);
        copy_array(nextstate, theprevstate, delays);
    }
}

void Encode(int input[], int parity[])
{
    int prevstate[DELAYS], nextstate[DELAYS];
    int nextstateint, startstate;

    if (!CIRCULAR_ENCODING) {
        Conv_Encode_number_of_delays(STARTING_STATE, input, FRAME_SIZE,
                                     G1, G2, parity, nextstate, DELAYS);
        return;
    }

    /* Circular mode: two passes */
    int_to_binary(0, DELAYS, prevstate);
    Conv_Encode_number_of_delays(prevstate, input, FRAME_SIZE,
                                 G1, G2, parity, nextstate, DELAYS);
    nextstateint = binary_to_int(nextstate, DELAYS);

    startstate = CIRCULAR_STATES[nextstateint];
    int_to_binary(startstate, DELAYS, prevstate);
    Conv_Encode_number_of_delays(prevstate, input, FRAME_SIZE,
                                 G1, G2, parity, nextstate, DELAYS);
}

void Encode_Termination(int input[], int parity[], int termination[])
{
    int j, in;
    int prevstate[DELAYS], nextstate[DELAYS];

    Conv_Encode_number_of_delays(STARTING_STATE, input, FRAME_SIZE,
                                 G1, G2, parity, nextstate, DELAYS);
    copy_array(nextstate, prevstate, DELAYS);

    for (j = 0; j < DELAYS; j++) {
        in = 0;
        for (int i = 0; i < DELAYS; i++) in ^= G2[i + 1] & nextstate[i];
        termination[j]         = in;
        termination[j + DELAYS] = Encode_1_bit_number_of_delays(
                                      prevstate, in, G1, G2, nextstate, DELAYS);
        copy_array(nextstate, prevstate, DELAYS);
    }
}


/* =========================================================================
 * Puncturing masks
 * ========================================================================= */

void puncmask_all_frame_FrameSize(int x[], int framesize, int type)
{
    for (int i = 0; i < framesize; i++)
        x[i] = (type == 0) ? PUNCTURED[i % PUNCT_MASK]
                            : PUNCTURED_II[i % PUNCT_MASK];
}

void puncmask_frame(int x[], int startfrom, int size, int type)
{
    for (int i = 0; i < size; i++)
        x[i] = (type == 0) ? PUNCTURED[(i + startfrom) % PUNCT_MASK]
                            : PUNCTURED_II[(i + startfrom) % PUNCT_MASK];
}


/* =========================================================================
 * Distance computation
 * ========================================================================= */

int Turbo_distance(int x[], int size, int Original_address[])
{
    int interleaver[FRAME_SIZE];
    int impulse_response[FRAME_SIZE];
    int mask[FRAME_SIZE];
    int termination[2 * DELAYS];
    int i, d = size;

    /* --- First RSC encoder (systematic + parity stream 1) --- */
    for (i = 0; i < FRAME_SIZE; i++) interleaver[i] = 0;
    for (i = 0; i < size; i++)       interleaver[x[i]] = 1;

    if (TAIL_BITS) {
        Encode_Termination(interleaver, impulse_response, termination);
        d += Number_Of_Ones(termination, 2 * DELAYS);
    } else {
        Encode(interleaver, impulse_response);
    }
    puncmask_all_frame_FrameSize(mask, FRAME_SIZE, 0);
    Array_AND(impulse_response, mask, impulse_response, FRAME_SIZE);
    d += Number_Of_Ones(impulse_response, FRAME_SIZE);

    /* --- Second RSC encoder (interleaved input, parity stream 2) --- */
    for (i = 0; i < FRAME_SIZE; i++) interleaver[i] = 0;
    for (i = 0; i < size; i++)       interleaver[Original_address[x[i]]] = 1;

    if (TAIL_BITS) {
        Encode_Termination(interleaver, impulse_response, termination);
        d += Number_Of_Ones(termination, 2 * DELAYS);
    } else {
        Encode(interleaver, impulse_response);
    }
    puncmask_all_frame_FrameSize(mask, FRAME_SIZE, 1);
    Array_AND(impulse_response, mask, impulse_response, FRAME_SIZE);
    d += Number_Of_Ones(impulse_response, FRAME_SIZE);

    return d;
}

int parity_distance(int x[], int size, int type)
{
    int interleaver[FRAME_SIZE];
    int impulse_response[FRAME_SIZE];
    int mask[FRAME_SIZE];
    int termination[2 * DELAYS];
    int d = 0;

    for (int i = 0; i < FRAME_SIZE; i++) interleaver[i] = 0;
    for (int i = 0; i < size; i++)       interleaver[x[i]] = 1;

    if (TAIL_BITS) {
        Encode_Termination(interleaver, impulse_response, termination);
        d += Number_Of_Ones(termination, 2 * DELAYS);
    } else {
        Encode(interleaver, impulse_response);
    }
    puncmask_all_frame_FrameSize(mask, FRAME_SIZE, type);
    Array_AND(impulse_response, mask, impulse_response, FRAME_SIZE);
    d += Number_Of_Ones(impulse_response, FRAME_SIZE);

    return d;
}
