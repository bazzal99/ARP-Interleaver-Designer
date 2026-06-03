/**
 * @file rtz_search.c
 * @brief RTZ graph-girth search kernel implementations.
 */

#include <stdio.h>
#include "config.h"
#include "encoder.h"
#include "rtz_search.h"
#include "utils.h"

/* =========================================================================
 * Shared helper: write a candidate to temp.txt
 * ========================================================================= */

static void record_to_temp(int path[], int pathLen, int d, int threshold, int *mult)
{
    if (d == threshold) {
        (*mult)++;
        print_in_file_sorted("temp.txt", path, pathLen);
    }
}


/* =========================================================================
 * IW2 kernel — ZT (non-circular) mode
 * ========================================================================= */

int Min_Girth_One_Layer_RTZ_IW2_Without_Circular(int layer, int pi[], int depi[],
                                                  int threshold, int *mult,
                                                  int max_RTZ_size, int mingirth)
{
    int path[20];
    if (layer == -1) return 1;
    return Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(
               layer, pi, depi, 0, layer, layer, 0,
               path, 0, threshold, mult, max_RTZ_size, mingirth);
}

int Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(int layer, int pi[], int depi[],
                                                           int girth, int ref, int the_ref,
                                                           int starting, int path[], int pathLen,
                                                           int threshold, int *mult,
                                                           int max_RTZ_size, int mingirth)
{
    int i, d, x = 1;

    /* Symmetry pruning: only explore ref >= the_ref */
    if (starting == 0 && ref < the_ref) return 1;

    path[pathLen] = (starting == 1) ? depi[ref] : ref;
    pathLen++;

    if (girth > mingirth) return 1;

    if (ref == the_ref && starting == 0 && girth != 0 && girth <= mingirth) {
        d = Turbo_distance(path, pathLen - 1, pi);
        record_to_temp(path, pathLen - 1, d, threshold, mult);
        return (d < threshold) ? 0 : 1;
    }

    for (i = ENCODER_PERIOD; i <= max_RTZ_size * ENCODER_PERIOD; i += ENCODER_PERIOD) {
        if (x == 0) return 0;
        if (starting == 0) {
            int cm1 = circular_modulus(ref - i, FRAME_SIZE);
            int cm2 = circular_modulus(ref + i, FRAME_SIZE);
            if (cm1 != -1 && girth != 0)
                x &= Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(
                         layer, pi, depi, girth + 1, pi[cm1], the_ref, 1,
                         path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1)
                x &= Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(
                         layer, pi, depi, girth + 1, pi[cm2], the_ref, 1,
                         path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        } else {
            int cm1 = circular_modulus(ref - i, FRAME_SIZE);
            int cm2 = circular_modulus(ref + i, FRAME_SIZE);
            if (cm1 != -1)
                x &= Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(
                         layer, pi, depi, girth + 1, depi[cm1], the_ref, 0,
                         path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1)
                x &= Min_Girth_One_Ref_With_Layer_RTZ_IW2_Without_Circular(
                         layer, pi, depi, girth + 1, depi[cm2], the_ref, 0,
                         path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        }
    }
    return x;
}


/* =========================================================================
 * LTE (general) kernel — supports both ZT and circular modes,
 * and partially-initialised ARP interleavers (entries == -1 are skipped)
 * ========================================================================= */

int Min_Girth_One_Layer_LTE(int layer, int pi[], int depi[],
                             int threshold, int *mult,
                             int max_RTZ_size, int mingirth)
{
    int path[20];
    if (layer == -1) return 1;
    return Min_Girth_One_Ref_With_Layer_LTE(
               layer, pi, depi, 0, layer, layer, 0,
               path, 0, threshold, mult, max_RTZ_size, mingirth);
}

int Min_Girth_One_Ref_With_Layer_LTE(int layer, int pi[], int depi[],
                                      int girth, int ref, int the_ref, int starting,
                                      int path[], int pathLen,
                                      int threshold, int *mult,
                                      int max_RTZ_size, int mingirth)
{
    int i, d, x = 1;

    if (starting == 0 && ref < the_ref)               return 1;
    if (ref < 0 || ref >= FRAME_SIZE)                  return 1;
    if (pi[ref]   == -1 && starting == 0)              return 1;
    if (depi[ref] == -1 && starting == 1)              return 1;

    path[pathLen] = (starting == 1) ? depi[ref] : ref;
    pathLen++;

    if (girth > mingirth) return 1;
    if (path[pathLen - 1] != the_ref && search(path[pathLen - 1], path, pathLen - 1) != -1)
        return 1;

    if (path[pathLen - 1] == the_ref && girth != 0 && girth <= mingirth) {
        d = Turbo_distance(path, pathLen - 1, pi);
        record_to_temp(path, pathLen - 1, d, threshold, mult);
        return (d < threshold) ? 0 : 1;
    }

    for (i = 1; i <= max_RTZ_size; i++) {
        if (x == 0) return 0;
        if (CIRCULAR_ENCODING) {
            if (starting == 0) {
                int cm1 = circular_modulus(ref - i, FRAME_SIZE);
                int cm2 = circular_modulus(ref + i, FRAME_SIZE);
                if (cm1 != -1)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             pi[cm1], the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
                if (cm2 != -1)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             pi[cm2], the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            } else {
                int cm1 = circular_modulus(ref - i, FRAME_SIZE);
                int cm2 = circular_modulus(ref + i, FRAME_SIZE);
                if (cm1 != -1)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             depi[cm1], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
                if (cm2 != -1)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             depi[cm2], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            }
        } else {
            /* ZT mode: hard boundary checks */
            if (starting == 0) {
                if (ref - i >= 0 && girth != 0)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             pi[ref - i], the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
                if (ref + i < FRAME_SIZE)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             pi[ref + i], the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            } else {
                if (ref - i >= 0)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             depi[ref - i], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
                if (ref + i < FRAME_SIZE)
                    x &= Min_Girth_One_Ref_With_Layer_LTE(layer, pi, depi, girth + 1,
                             depi[ref + i], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            }
        }
    }
    return x;
}


/* =========================================================================
 * Legacy IW2 kernel (circular mode)
 * ========================================================================= */

int Min_Distance_One_Layer_IW2(int layer, int pi[], int depi[],
                                int threshold, int *mult,
                                int max_RTZ_size, int mingirth)
{
    int path[20];
    return Min_Distance_One_Ref_With_Layer_IW2(layer, pi, depi, 0, layer, layer, 0,
                                                path, 0, threshold, mult, max_RTZ_size, mingirth);
}

int Min_Distance_One_Ref_With_Layer_IW2(int layer, int pi[], int depi[],
                                         int girth, int ref, int the_ref, int starting,
                                         int path[], int pathLen,
                                         int threshold, int *mult,
                                         int max_RTZ_size, int mingirth)
{
    int i, d, x = 1;

    path[pathLen] = (starting == 1) ? depi[ref] : ref;
    pathLen++;

    if (girth > mingirth || ref == -1) return 1;

    if (ref == the_ref && starting == 0 && girth != 0 && girth <= mingirth) {
        d = Turbo_distance(path, pathLen - 1, pi);
        if (d == threshold) (*mult)++;
        if (d < threshold) {
            printf("\nNew best: ref=%d  d=%d\n", ref, d);
            return 0;
        }
        return 1;
    }

    for (i = ENCODER_PERIOD; i <= max_RTZ_size * ENCODER_PERIOD; i += ENCODER_PERIOD) {
        if (x == 0) return 0;
        int cm1 = circular_modulus(ref - i, FRAME_SIZE);
        int cm2 = circular_modulus(ref + i, FRAME_SIZE);
        if (starting == 0) {
            if (cm1 != -1) x &= Min_Distance_One_Ref_With_Layer_IW2(layer, pi, depi, girth + 1, pi[cm1],   the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1) x &= Min_Distance_One_Ref_With_Layer_IW2(layer, pi, depi, girth + 1, pi[cm2],   the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        } else {
            if (cm1 != -1) x &= Min_Distance_One_Ref_With_Layer_IW2(layer, pi, depi, girth + 1, depi[cm1], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1) x &= Min_Distance_One_Ref_With_Layer_IW2(layer, pi, depi, girth + 1, depi[cm2], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        }
    }
    return x;
}


/* =========================================================================
 * Legacy RTZ-Multiple kernel
 * ========================================================================= */

int Min_Distance_One_Layer_RTZ_Multiple(int layer, int pi[], int depi[],
                                         int threshold, int *mult,
                                         int max_RTZ_size, int mingirth)
{
    int path[20];
    return Min_Distance_One_Ref_With_Layer_RTZ_Multiple(layer, pi, depi, 0, layer, layer, 0,
                                                         path, 0, threshold, mult, max_RTZ_size, mingirth);
}

int Min_Distance_One_Ref_With_Layer_RTZ_Multiple(int layer, int pi[], int depi[],
                                                  int girth, int ref, int the_ref,
                                                  int starting, int path[], int pathLen,
                                                  int threshold, int *mult,
                                                  int max_RTZ_size, int mingirth)
{
    int i, d, x = 1;

    path[pathLen] = (starting == 1) ? depi[ref] : ref;
    pathLen++;

    if (girth > mingirth || ref == -1) return 1;
    if (path[pathLen - 1] != the_ref && search(path[pathLen - 1], path, pathLen - 1) != -1)
        return 1;

    if (path[pathLen - 1] == the_ref && girth != 0 && girth <= mingirth) {
        d = Turbo_distance(path, pathLen - 1, pi);
        if (d == threshold) (*mult)++;
        return (d < threshold) ? 0 : 1;
    }

    for (i = 1; i <= max_RTZ_size; i++) {
        if (x == 0) return 0;
        int cm1 = circular_modulus(ref - i, FRAME_SIZE);
        int cm2 = circular_modulus(ref + i, FRAME_SIZE);
        if (starting == 0) {
            if (cm1 != -1) x &= Min_Distance_One_Ref_With_Layer_RTZ_Multiple(layer, pi, depi, girth + 1, pi[cm1],   the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1) x &= Min_Distance_One_Ref_With_Layer_RTZ_Multiple(layer, pi, depi, girth + 1, pi[cm2],   the_ref, 1, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        } else {
            if (cm1 != -1) x &= Min_Distance_One_Ref_With_Layer_RTZ_Multiple(layer, pi, depi, girth + 1, depi[cm1], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
            if (cm2 != -1) x &= Min_Distance_One_Ref_With_Layer_RTZ_Multiple(layer, pi, depi, girth + 1, depi[cm2], the_ref, 0, path, pathLen, threshold, mult, max_RTZ_size, mingirth);
        }
    }
    return x;
}
