/**
 * @file designer.c
 * @brief ARP interleaver design algorithm implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../include/config.h"
#include "../include/encoder.h"
#include "../include/interleaver.h"
#include "../include/rtz_search.h"
#include "../include/designer.h"
#include "../include/utils.h"

/* Global counter: number of valid interleavers found in the current round */
static int count_interleaver = 0;


/* =========================================================================
 * Test_Minimum_Distance
 * ========================================================================= */

void Test_Minimum_Distance(int s[], int p, int threshold)
{
    int Original_address[FRAME_SIZE];
    int Interleaved_address[FRAME_SIZE];
    int i, j, d, mult = 0;
    int xlp[1];
    int lcm = (ARP_Q * PUNCT_MASK) / gcd(ARP_Q, PUNCT_MASK);

    for (i = 0; i < FRAME_SIZE; i++) {
        Interleaved_address[i] = -1;
        Original_address[i]    = -1;
    }
    ARPInterleaverInitialization(FRAME_SIZE, p, ARP_Q, s,
                                  Interleaved_address, Original_address);

    /* Quick single-impulse scan */
    for (j = FRAME_SIZE - 1; j > FRAME_SIZE / 2; j--) {
        if (Original_address[j] != -1 &&
            (FRAME_SIZE - j + FRAME_SIZE - Original_address[j]) < FRAME_SIZE) {
            xlp[0] = j;
            d = Turbo_distance(xlp, 1, Original_address);
            if (d < threshold) printf("%d \t distance=%d\n", j, d);
        }
    }

    /* RTZ search over all layers */
    for (i = 0; i < lcm; i++) {
        printf("Layer %d:\n", i);
        for (j = 0; j < IW2_MAX_WEIGHT; j++) {
            int g2[] = IW2_GIRTH_BOUNDS;
            int p2[] = IW2_PERIOD_BUDGETS;
            if (Min_Girth_One_Layer_RTZ_IW2_Without_Circular(i, Original_address,
                    Interleaved_address, threshold, &mult, p2[j], g2[j]) == 0)
                printf("  New best (RTZ-IW2, iw=%d)\n", j * 2 + 2);
        }
        int gM[] = IWM_GIRTH_BOUNDS;
        int pM[] = IWM_STEP_BUDGETS;
        for (j = 3; j <= 5; j++) {
            if (Min_Girth_One_Layer_LTE(i, Original_address, Interleaved_address,
                    threshold, &mult, pM[j], gM[j]) == 0)
                printf("  New best (LTE-Multiple, iw=%d)\n", j);
        }
    }
}


/* =========================================================================
 * Determine_Best_Interleaver_Design
 * ========================================================================= */

void Determine_Best_Interleaver_Design(const char *fileoutput,
                                       const char *finalname,
                                       const char *fileinput,
                                       int threshold, int with_shuffle,
                                       int print, int max_size_per_layer,
                                       int distribute_layers_flag,
                                       int type_of_search)
{
    int s[ARP_Q];
    int results[FRAME_SIZE];
    int count = 0;
    FILE *file;

    s[0] = 0;
    file = fopen(fileoutput, "w"); fclose(file);
    file = fopen(finalname,  "w"); fclose(file);

    if (distribute_layers_flag)
        distribute_layers(fileinput);

    Choose_Period(results, &count, FRAME_SIZE);
    if (with_shuffle) shuffleArray(results, count);

    if (print) {
        for (int i = 0; i < count; i++) printf("%d ", results[i]);
        printf("\n\n");
    }

    for (int t = 0; t < 100000; t++) {
        if (count_interleaver >= MAX_INTERLEAVERS) {
            count_interleaver = 0;
            threshold++;
        }
        file = fopen(finalname, "w"); fclose(file);
        if (print) printf("t = %d\n", t);
        if (with_shuffle) shuffleArray(results, count);

        Determine_ALL_Layers_With_Punc(fileoutput, finalname, results[0],
                                        s, FRAME_SIZE, 0, 0, threshold,
                                        with_shuffle, print,
                                        max_size_per_layer, type_of_search);
    }
}


/* =========================================================================
 * Determine_ALL_Layers_With_Punc (recursive layer builder)
 * ========================================================================= */

void Determine_ALL_Layers_With_Punc(const char *filename,
                                    const char *finalname,
                                    int p, int S[],
                                    int dmin, int multiplicity, int layer,
                                    int threshold, int with_shuffle,
                                    int print, int max_size_layer,
                                    int type_of_search)
{
    int i, j;
    char name[32];
    int array_s[FRAME_SIZE];
    int minimum_distances[FRAME_SIZE];
    int distance_duplicate[FRAME_SIZE];
    int size = 0;
    FILE *file;

    if (count_interleaver > MAX_INTERLEAVERS) return;

    /* All Q layers filled: record this complete interleaver */
    if (layer == ARP_Q) {
        count_interleaver++;
        file = fopen(filename, "a");
        for (i = 0; i < ARP_Q; i++) {
            if (i == ARP_Q - 1)
                fprintf(file, "%d", S[i]);
            else
                fprintf(file, "%d,", S[i]);
        }
        fprintf(file, "     \t    dmin = %d - P=%d  - multiple = %d \n",
                dmin, p, multiplicity);
        fclose(file);
        if (print) printf("Found a valid interleaver.\n");
        return;
    }

    sprintf(name, "layer_%d.txt", layer);
    Add_Layer_With_Shuffle_WIth_Punc(name, p, S, layer,
                                      array_s, minimum_distances, distance_duplicate,
                                      &size, threshold, with_shuffle,
                                      max_size_layer, type_of_search);

    if (print) {
        printf("\nPossible S[%d]: ", layer);
        for (i = 0; i < size; i++) printf("%d ", array_s[i]);
        printf("  (%d candidates)\n", size);
    }

    if (size == 0) return;

    for (i = 0; i < size; i++) {
        S[layer] = array_s[i];
        if (print) {
            for (j = 0; j <= layer; j++) printf("%d ", S[j]);
            printf("\n");
        }
        if (minimum_distances[i] < dmin)
            Determine_ALL_Layers_With_Punc(filename, finalname, p, S,
                minimum_distances[i], distance_duplicate[i], layer + 1,
                threshold, with_shuffle, print, max_size_layer, type_of_search);
        else if (minimum_distances[i] == dmin)
            Determine_ALL_Layers_With_Punc(filename, finalname, p, S,
                minimum_distances[i],
                (type_of_search == 0) ? distance_duplicate[i] + multiplicity
                                      : distance_duplicate[i],
                layer + 1, threshold, with_shuffle, print, max_size_layer, type_of_search);
        else
            Determine_ALL_Layers_With_Punc(filename, finalname, p, S,
                dmin, multiplicity, layer + 1,
                threshold, with_shuffle, print, max_size_layer, type_of_search);
    }
}


/* =========================================================================
 * Add_Layer_With_Shuffle_WIth_Punc
 * ========================================================================= */

void Add_Layer_With_Shuffle_WIth_Punc(const char *filename, int p, int S[],
                                      int layer, int array_s[],
                                      int minimum_distances[],
                                      int distance_duplicate[],
                                      int *size, int threshold,
                                      int with_shuffle, int max_size_layer,
                                      int type_of_search)
{
    int i, j, pp, sl, num, distance, line;
    int dmin, multiplicity;
    char comma;
    int x[threshold], pi_arr[threshold];
    int Interleaved_address[FRAME_SIZE];
    int Original_address[FRAME_SIZE];
    int all_sl[FRAME_SIZE];
    int xlp[1];
    FILE *file;
    int reach = FRAME_SIZE;
    int num_tested = 0;
    int lcm = (ARP_Q * PUNCT_MASK) / gcd(ARP_Q, PUNCT_MASK);

    for (i = 0; i < FRAME_SIZE; i++) {
        Interleaved_address[i] = -1;
        Original_address[i]    = -1;
    }
    for (i = layer; i < ARP_Q; i++) S[i] = 0;

    ARPInterleaverInitialization_Till_Layer(FRAME_SIZE, layer - 1, p, ARP_Q, S,
                                            Interleaved_address, Original_address);
    if (layer == 0) reach = lcm;
    for (i = 0; i < reach; i++) all_sl[i] = i;
    if (with_shuffle) shuffleArray(all_sl, reach);

    for (pp = 0; pp < reach; pp++) {
    choose_s:
        sl = all_sl[pp];
        if (num_tested >= 30 || pp >= reach) break;

        /* Check ARP regularity: no two layers map to the same residue mod Q */
        S[layer] = sl;
        for (j = 0; j < layer; j++) {
            if ((p * layer + sl) % ARP_Q == (p * j + S[j]) % ARP_Q) {
                pp++;
                goto choose_s;
            }
        }

        dmin        = FRAME_SIZE;
        multiplicity = 0;
        ARPInterleaverInitialization_Per_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                               Interleaved_address, Original_address);

        /* Single-impulse pre-filter (ZT mode) */
        if (TAIL_BITS) {
            for (j = FRAME_SIZE - 1; j > FRAME_SIZE / 2; j--) {
                if (Original_address[j] != -1 &&
                    (FRAME_SIZE - j + FRAME_SIZE - Original_address[j]) < 2 * threshold * PUNCT_MASK) {
                    xlp[0] = j;
                    if (Turbo_distance(xlp, 1, Original_address) < threshold) {
                        Arp_DE_Interleave_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                                Interleaved_address, Original_address);
                        pp++;
                        goto choose_s;
                    }
                }
            }
        }

        /* File-based evaluation */
        if (type_of_search == 0 || type_of_search == 2) {
            file = fopen(filename, "r");
            line = 0;
            do {
                line++;
                fscanf(file, "%d%c", &num, &comma);
                for (i = 0; i < num; i++) {
                    fscanf(file, "%d%c", &x[i], &comma);
                    pi_arr[i] = Original_address[x[i]];
                }
                fscanf(file, "%d%c", &distance, &comma);
                distance += parity_distance(pi_arr, num, 1);
                if (distance < dmin) { dmin = distance; multiplicity = 0; }
                if (distance == dmin) multiplicity++;
                if (dmin < threshold) {
                    fclose(file);
                    Arp_DE_Interleave_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                            Interleaved_address, Original_address);
                    pp++;
                    goto choose_s;
                }
            } while (!feof(file));
            fclose(file);
        }

        /* Cycle-based evaluation */
        if (type_of_search == 1 || type_of_search == 2) {
            file = fopen("temp.txt", "w"); fclose(file);
            num_tested++;

            int gM[] = IWM_GIRTH_BOUNDS;
            int pM[] = IWM_STEP_BUDGETS;
            int g2[] = IW2_GIRTH_BOUNDS;
            int p2[] = IW2_PERIOD_BUDGETS;

            for (i = 0; i < lcm; i++) {
                for (j = 0; j < IW2_MAX_WEIGHT; j++) {
                    if (Min_Girth_One_Layer_RTZ_IW2_Without_Circular(i,
                            Original_address, Interleaved_address,
                            threshold, &multiplicity, p2[j], g2[j]) == 0) {
                        Arp_DE_Interleave_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                                Interleaved_address, Original_address);
                        pp++;
                        goto choose_s;
                    }
                }
                for (j = 3; j <= IWM_MAX_WEIGHT; j++) {
                    if (Min_Girth_One_Layer_LTE(i, Original_address, Interleaved_address,
                            threshold, &multiplicity, pM[j], gM[j]) == 0) {
                        Arp_DE_Interleave_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                                Interleaved_address, Original_address);
                        pp++;
                        goto choose_s;
                    }
                }
            }

            removeDuplicateLines("temp.txt");
            multiplicity = countLinesInFile("temp.txt");
            if (dmin > threshold && multiplicity != 0) dmin = threshold;
            file = fopen("temp.txt", "w"); fclose(file);
        }

        Arp_DE_Interleave_Layer(FRAME_SIZE, layer, p, ARP_Q, S,
                                 Interleaved_address, Original_address);
        array_s[*size]            = sl;
        minimum_distances[*size]  = dmin;
        distance_duplicate[*size] = multiplicity;
        (*size)++;
        if (with_shuffle)
            shuffle3Arrays(array_s, minimum_distances, distance_duplicate, *size);
        if (*size >= max_size_layer) break;
    }
}


/* =========================================================================
 * Calculate_Min_Distance_And_Multiplicity
 * ========================================================================= */

void Calculate_Min_Distance_And_Multiplicity(const char *filename,
                                             int s[], int p, int threshold,
                                             int *mydmin, int *mymul)
{
    FILE *file;
    int i, iw, d;
    char comma;
    int x[threshold], y[threshold];
    int dmin        = FRAME_SIZE;
    int multiplicity = 0;

    file = fopen(filename, "r");
    if (!file) { printf("Error opening file.\n"); return; }

    do {
        fscanf(file, "%d%c", &iw, &comma);
        for (i = 0; i < iw; i++) {
            fscanf(file, "%d%c", &x[i], &comma);
            y[i] = inter(x[i], p, s);
        }
        fscanf(file, "%d%c", &d, &comma);
        d += parity_distance(y, iw, 1);

        if (d < dmin && d != 0) { dmin = d; multiplicity = 0; }
        if (d == dmin) multiplicity++;
        if (dmin < threshold) break;
    } while (!feof(file));

    fclose(file);
    *mydmin = dmin;
    *mymul  = multiplicity;
}
