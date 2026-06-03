/**
 * @file main.c
 * @brief Entry point for the ARP interleaver design tool.
 *
 * Edit config.h to change the encoder and interleaver parameters,
 * then compile with `make` and run `./arp_designer`.
 *
 * Output files:
 *   result.txt        — all interleavers meeting the distance threshold
 *   final_result.txt  — the best interleaver found
 *
 * Reference:
 *   M. Bazzal, J. Nadal, S. Weithoffer, C. Abdel Nour, C. Douillard,
 *   "Distance-centric joint interleaver and structural code design for
 *   concatenated convolutional codes,"
 *   IEEE Open Journal of the Communications Society, 2025.
 *
 * Author:  Mohammad Bazzal, IMT Atlantique, Lab-STICC, Brest, France
 * License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "../include/config.h"
#include "../include/encoder.h"
#include "../include/interleaver.h"
#include "../include/designer.h"
#include "../include/utils.h"

int main(void)
{
    srand(time(NULL));

    /* Initialise circular (tail-biting) starting states if needed */
    if (CIRCULAR_ENCODING)
        Initialize_States();

    /* ---------------------------------------------------------------
     * Precomputed RTZ file (used when SEARCH_TYPE = 0 or 2)
     *
     * The file "All_RTZ.txt" must be generated once using:
     *   saveAllBranchesToFile_WITH_Punc_WITH_SIZE_K(...)
     *   Transform_shape_to_shape_FOR_K(...)
     * before running the interleaver design search.
     *
     * If using SEARCH_TYPE = 1 (cycle-based only), this file is
     * not required.
     * --------------------------------------------------------------- */
    const char *rtz_file    = "All_RTZ.txt";
    const char *result_file = "result.txt";
    const char *final_file  = "final_result.txt";

    /* Run the design search */
    Determine_Best_Interleaver_Design(
        result_file,
        final_file,
        rtz_file,
        DESIGN_THRESHOLD,
        WITH_SHUFFLE,
        VERBOSE,
        MAX_CANDIDATES_PER_LAYER,
        0,              /* distribute_layers: set to 1 to pre-sort the RTZ file */
        SEARCH_TYPE
    );

    return 0;
}
