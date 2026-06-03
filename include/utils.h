/**
 * @file utils.h
 * @brief General-purpose utility functions.
 *
 * Covers integer/binary conversion, array operations, sorting, shuffling,
 * file I/O helpers, duplicate removal, and the precomputed RTZ branch table.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/* =========================================================================
 * Integer / binary conversion
 * ========================================================================= */

void int_to_binary(int num, int size, int binary_arr[]);
int  binary_to_int(int binary_arr[], int size);
int  circular_modulus(int value, int range);


/* =========================================================================
 * Array utilities
 * ========================================================================= */

int  search(int target, int arr[], int n);
int  Number_Of_Ones(int a[], int size);
void Array_XOR(int a[], int b[], int c[], int size);
void Array_AND(int a[], int b[], int c[], int size);
void copy_array(int source[], int destination[], int size);
void duplicateArray(int source[], int destination[], int size);
int  sum(int arr[], int size);


/* =========================================================================
 * Sorting and shuffling
 * ========================================================================= */

void bubbleSort(int arr[], int size);
void swap(int *a, int *b);
void shuffleArray(int arr[], int size);

/**
 * Simultaneously shuffle three arrays with the same permutation.
 * Used to keep (shift_value, min_distance, multiplicity) triples aligned.
 */
void shuffle3Arrays(int arr[], int arr1[], int arr2[], int size);

void generateRandomBinaryArray(int *array, int length);


/* =========================================================================
 * Bit counting
 * ========================================================================= */

/** Count 1-bits in an integer. */
int countOnes(int n);

/** Count the total number of 1-bits across all elements of an array. */
int countBinaryOnes(int arr[], int size);

/** Integer power: base^exponent. */
int power(int base, int exponent);

/** Greatest common divisor (Euclidean algorithm). */
int gcd(int a, int b);

/**
 * Expand a 7-bit binary vector to 14 bits by zero-padding the upper half.
 * Used in the RTZ branch table construction.
 */
void bit7_to_bit14(int bit7[], int bit14[]);


/* =========================================================================
 * File I/O utilities
 * ========================================================================= */

/** Return 1 if the file is empty (or doesn't exist), 0 otherwise. */
int isFileEmpty(const char *filename);

/** Count the number of newline-terminated lines in a file. */
int countLinesInFile(const char *filename);

/** Append a sorted copy of `path` to `filename`, one value per line. */
void print_in_file_sorted(const char *filename, int path[], int pathLen);

/** Write one RTZ entry (weight, positions, distance) to a file. */
void print_line_in_file(FILE *output, int num, int distance, int input[]);

/** Remove duplicate lines from `filename` in-place. */
void removeDuplicateLines(const char *filename);

/**
 * Distribute lines of `filename` into per-distance bucket files
 * output_1.txt … output_dmax.txt, then optionally delete the original.
 */
void distributeFiles(const char *filename, int dmax, int removes);

/**
 * Merge output_1.txt … output_n.txt into `filename` (sorted by distance),
 * then delete the per-bucket files.
 */
void merge_files(const char *filename, int n);

/**
 * Distribute lines of `filename` into per-layer files layer_0.txt … layer_{Q-1}.txt.
 * Used when DISTRIBUTE_LAYERS=1 to speed up file-based RTZ lookup.
 */
void distribute_layers(const char *filename);

/** Determine which ARP sub-layer an input pattern x[] belongs to. */
int give_my_layer(int x[], int size);


/* =========================================================================
 * RTZ branch table
 * ========================================================================= */

/**
 * Precompute the 2^ENCODER_PERIOD-entry RTZ branch table.
 *
 * table[i] = {pattern_index, input_weight, first_parity_int,
 *             second_parity_int, first_parity_weight}
 *
 * Used by saveAllBranchesToFile_WITH_Punc_WITH_SIZE_K() to enumerate
 * all low-weight codewords analytically.
 */
void Create_Table_Possible_Combinations(int table[][5]);

/**
 * Recursively enumerate all low-weight RTZ branches and write them to `filename`.
 * Output format: "iw/pos0,pos1,...,posN;parity_weight\n"
 */
void saveAllBranchesToFile_WITH_Punc_WITH_SIZE_K(const char *filename,
                                                  int table[][5],
                                                  int depth, int dmax);

void saveBranchesToFile_With_Punc_Size_K(int table[][5], int state,
                                          int IW, int OW, int depth,
                                          FILE *file, int path[],
                                          int pathLen, int dmax,
                                          int acc_temp, int lcm);

/**
 * Convert the compact branch-table file format into the expanded
 * per-position format expected by Calculate_Min_Distance_And_Multiplicity.
 */
void Transform_shape_to_shape_FOR_K(const char *filenameread,
                                    const char *outputname, int threshold);

/** Compute the punctured parity weight of a single 7-bit block. */
int output_with_punc(int x, int start, int type);

#endif /* UTILS_H */
