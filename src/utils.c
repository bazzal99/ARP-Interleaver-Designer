/**
 * @file utils.c
 * @brief General-purpose utility function implementations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/config.h"
#include "../include/encoder.h"
#include "../include/interleaver.h"
#include "../include/utils.h"

#define HASH_TABLE_SIZE  1009
#define MAX_LINE_LEN     60

/* =========================================================================
 * Integer / binary conversion
 * ========================================================================= */

void int_to_binary(int num, int size, int binary_arr[])
{
    for (int i = size - 1; i >= 0; i--) {
        binary_arr[i] = num % 2;
        num /= 2;
    }
}

int binary_to_int(int binary_arr[], int size)
{
    int decimal = 0;
    for (int i = 0; i < size; i++)
        decimal += binary_arr[i] * (int)pow(2, size - 1 - i);
    return decimal;
}

int circular_modulus(int value, int range)
{
    return (value % range + range) % range;
}


/* =========================================================================
 * Array utilities
 * ========================================================================= */

int search(int target, int arr[], int n)
{
    for (int i = 0; i < n; i++)
        if (arr[i] == target) return i;
    return -1;
}

int Number_Of_Ones(int a[], int size)
{
    int s = 0;
    for (int i = 0; i < size; i++) s += a[i];
    return s;
}

void Array_XOR(int a[], int b[], int c[], int size)
{
    for (int i = 0; i < size; i++) c[i] = a[i] ^ b[i];
}

void Array_AND(int a[], int b[], int c[], int size)
{
    for (int i = 0; i < size; i++) c[i] = a[i] & b[i];
}

void copy_array(int source[], int destination[], int size)
{
    for (int i = 0; i < size; i++) destination[i] = source[i];
}

void duplicateArray(int source[], int destination[], int size)
{
    for (int i = 0; i < size; i++) destination[i] = source[i];
}

int sum(int arr[], int size)
{
    int s = 0;
    for (int i = 0; i < size; i++) s += arr[i];
    return s;
}


/* =========================================================================
 * Sorting and shuffling
 * ========================================================================= */

void bubbleSort(int arr[], int size)
{
    for (int i = 0; i < size - 1; i++)
        for (int j = 0; j < size - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = tmp;
            }
}

void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

void shuffleArray(int arr[], int size)
{
    for (int i = size - 1; i > 0; i--)
        swap(&arr[i], &arr[rand() % (i + 1)]);
}

void shuffle3Arrays(int arr[], int arr1[], int arr2[], int size)
{
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&arr[i],  &arr[j]);
        swap(&arr1[i], &arr1[j]);
        swap(&arr2[i], &arr2[j]);
    }
}

void generateRandomBinaryArray(int *array, int length)
{
    for (int i = 0; i < length; i++) array[i] = rand() % 2;
}


/* =========================================================================
 * Bit counting
 * ========================================================================= */

int countOnes(int n)
{
    int c = 0;
    while (n) { c += n & 1; n >>= 1; }
    return c;
}

int countBinaryOnes(int arr[], int size)
{
    int c = 0;
    for (int i = 0; i < size; i++) {
        int n = arr[i];
        while (n > 0) { c += n & 1; n >>= 1; }
    }
    return c;
}

int power(int base, int exponent)
{
    int r = 1;
    for (int i = 0; i < exponent; i++) r *= base;
    return r;
}

void bit7_to_bit14(int bit7[], int bit14[])
{
    for (int i = 0; i < 2 * ENCODER_PERIOD; i++)
        bit14[i] = (i < ENCODER_PERIOD) ? bit7[i] : 0;
}


/* =========================================================================
 * File I/O utilities
 * ========================================================================= */

int isFileEmpty(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return (sz == 0);
}

int countLinesInFile(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    int count = 0;
    char c;
    while ((c = fgetc(f)) != EOF) if (c == '\n') count++;
    fclose(f);
    return count;
}

void print_in_file_sorted(const char *filename, int path[], int pathLen)
{
    int newpath[pathLen];
    copy_array(path, newpath, pathLen);
    bubbleSort(newpath, pathLen);
    FILE *f = fopen(filename, "a");
    for (int i = 0; i < pathLen; i++) {
        if (i == pathLen - 1) fprintf(f, "%d", newpath[i]);
        else                  fprintf(f, "%d,", newpath[i]);
    }
    fprintf(f, "\n");
    fclose(f);
}

void print_line_in_file(FILE *output, int num, int distance, int input[])
{
    fprintf(output, "%d/", num);
    for (int i = 0; i < num; i++) {
        if (i != num - 1) fprintf(output, "%d,", input[i]);
        else              fprintf(output, "%d;", input[i]);
    }
    fprintf(output, "%d\n", distance);
}

void distributeFiles(const char *filename, int dmax, int removes)
{
    FILE *input = fopen(filename, "r");
    char line[256];

    FILE *outputs[dmax];
    char outputNames[dmax][32];
    for (int i = 0; i < dmax; i++) {
        sprintf(outputNames[i], "output_%d.txt", i + 1);
        outputs[i] = fopen(outputNames[i], "a");
        if (!outputs[i]) { fprintf(stderr, "Error creating output files.\n"); exit(1); }
    }

    while (fgets(line, sizeof(line), input)) {
        int num = atoi(strrchr(line, ';') + 1);
        if (num >= 1 && num <= dmax)
            fprintf(outputs[num - 1], "%s", line);
    }
    fclose(input);
    if (removes) remove(filename);
    for (int i = 0; i < dmax; i++) fclose(outputs[i]);
}

void merge_files(const char *filename, int n)
{
    FILE *output = fopen(filename, "w");
    if (!output) { fprintf(stderr, "Error creating output file.\n"); exit(1); }

    for (int i = 1; i <= n; i++) {
        char fname[50];
        sprintf(fname, "output_%d.txt", i);
        FILE *input = fopen(fname, "r");
        if (!input) { fprintf(stderr, "Error opening %s\n", fname); exit(1); }
        char buf[1024];
        size_t nr;
        while ((nr = fread(buf, 1, sizeof(buf), input)) > 0)
            fwrite(buf, 1, nr, output);
        fclose(input);
        remove(fname);
    }
    fclose(output);
}

void distribute_layers(const char *filename)
{
    char comma;
    int num, i, x[30], distance;
    FILE *input = fopen(filename, "r");
    FILE *outputs[ARP_Q];
    char outputNames[ARP_Q][32];

    for (i = 0; i < ARP_Q; i++) {
        sprintf(outputNames[i], "layer_%d.txt", i);
        outputs[i] = fopen(outputNames[i], "w");
        if (!outputs[i]) { fprintf(stderr, "Error creating layer files.\n"); exit(1); }
    }

    do {
        fscanf(input, "%d%c", &num, &comma);
        for (i = 0; i < num; i++) fscanf(input, "%d%c", &x[i], &comma);
        fscanf(input, "%d%c", &distance, &comma);

        int lyr = give_my_layer(x, num);
        fprintf(outputs[lyr], "%d/", num);
        for (i = 0; i < num; i++)
            fprintf(outputs[lyr], (i == num - 1) ? "%d;" : "%d,", x[i]);
        fprintf(outputs[lyr], "%d%c", distance, comma);
    } while (!feof(input));

    fclose(input);
    for (i = 0; i < ARP_Q; i++) fclose(outputs[i]);
}

int give_my_layer(int x[], int size)
{
    int max = 0;
    for (int i = 0; i < size; i++) {
        int v = x[i] % ARP_Q;
        if (v > max) max = v;
    }
    return max;
}


/* =========================================================================
 * Duplicate-line removal
 * ========================================================================= */

static unsigned long hash_line(const char *str)
{
    unsigned long h = 5381;
    int c;
    while ((c = *str++)) h = ((h << 5) + h) + c;
    return h % HASH_TABLE_SIZE;
}

struct Node {
    char line[MAX_LINE_LEN];
    struct Node *next;
};

static void ht_insert(struct Node *table[], const char *line)
{
    unsigned long idx = hash_line(line);
    struct Node *node = malloc(sizeof(struct Node));
    if (!node) { perror("malloc"); exit(1); }
    strncpy(node->line, line, MAX_LINE_LEN - 1);
    node->line[MAX_LINE_LEN - 1] = '\0';
    node->next  = table[idx];
    table[idx]  = node;
}

static int ht_find(struct Node *table[], const char *line)
{
    unsigned long idx = hash_line(line);
    struct Node *cur  = table[idx];
    while (cur) {
        if (strcmp(cur->line, line) == 0) return 1;
        cur = cur->next;
    }
    return 0;
}

void removeDuplicateLines(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) { perror("fopen"); return; }

    struct Node *table[HASH_TABLE_SIZE];
    for (int i = 0; i < HASH_TABLE_SIZE; i++) table[i] = NULL;

    FILE *tmp = fopen("_dedup_tmp.txt", "w");
    if (!tmp) { perror("fopen tmp"); fclose(f); return; }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), f)) {
        if (!ht_find(table, line)) {
            ht_insert(table, line);
            fputs(line, tmp);
        }
    }
    fclose(f);
    fclose(tmp);
    if (remove(filename) != 0)              { perror("remove"); return; }
    if (rename("_dedup_tmp.txt", filename) != 0) perror("rename");
}


/* =========================================================================
 * RTZ branch table
 * ========================================================================= */

void Create_Table_Possible_Combinations(int table[][5])
{
    int nextstate[DELAYS];
    int binary[ENCODER_PERIOD], binary14[2 * ENCODER_PERIOD];
    int impulse[2 * ENCODER_PERIOD];
    int starting[DELAYS];
    for (int i = 0; i < DELAYS; i++) starting[i] = 0;

    for (int i = 0; i < (int)pow(2, ENCODER_PERIOD); i++) {
        int_to_binary(i, ENCODER_PERIOD, binary);
        bit7_to_bit14(binary, binary14);
        Conv_Encode_number_of_delays(starting, binary14, 2 * ENCODER_PERIOD,
                                     G1, G2, impulse, nextstate, DELAYS);
        table[i][0] = i;
        table[i][1] = Number_Of_Ones(binary14, 2 * ENCODER_PERIOD);
        table[i][2] = binary_to_int(impulse, ENCODER_PERIOD);
        table[i][3] = binary_to_int(impulse + ENCODER_PERIOD, ENCODER_PERIOD);
        table[i][4] = Number_Of_Ones(impulse, ENCODER_PERIOD);
    }
}

int output_with_punc(int x, int start, int type)
{
    int b[ENCODER_PERIOD], mask[ENCODER_PERIOD];
    int_to_binary(x, ENCODER_PERIOD, b);
    puncmask_frame(mask, start, ENCODER_PERIOD, type);
    Array_AND(b, mask, b, ENCODER_PERIOD);
    return Number_Of_Ones(b, ENCODER_PERIOD);
}

void saveAllBranchesToFile_WITH_Punc_WITH_SIZE_K(const char *filename,
                                                  int table[][5],
                                                  int depth, int dmax)
{
    int path[depth];
    int lcm = (ARP_Q * PUNCT_MASK) / (ARP_Q < PUNCT_MASK ?
                  (ARP_Q * PUNCT_MASK / (ARP_Q + PUNCT_MASK)) : gcd(ARP_Q, PUNCT_MASK));
    FILE *file = fopen(filename, "w"); fclose(file);
    file = fopen(filename, "a");
    saveBranchesToFile_With_Punc_Size_K(table, 0, -1, 0, depth, file, path, 0, dmax, 0, lcm);
    fclose(file);
}

void saveBranchesToFile_With_Punc_Size_K(int table[][5], int state,
                                          int IW, int OW, int depth,
                                          FILE *file, int path[],
                                          int pathLen, int dmax,
                                          int acc_temp, int lcm)
{
    int j;
    depth -= ENCODER_PERIOD;
    if (OW >= dmax || depth <= 0) return;

    if (pathLen == (lcm / ENCODER_PERIOD) &&
        countOnes(IW & ((power(2, lcm % ENCODER_PERIOD) - 1)
                        << (ENCODER_PERIOD - (lcm % ENCODER_PERIOD)))) == 0 &&
        sum(path, pathLen) == 0) return;

    if (IW != -1) {
        path[pathLen] = IW;
        pathLen++;

        OW = (pathLen == 1) ?
             output_with_punc(table[IW][2], pathLen - 1, 0) + table[IW][1] :
             OW + output_with_punc(table[acc_temp][3] ^ table[IW][2],
                                   (pathLen - 1) * ENCODER_PERIOD, 0) + table[IW][1];
        acc_temp ^= IW;

        int check = (pathLen == (lcm / ENCODER_PERIOD) &&
                     countOnes(state & ((power(2, lcm % ENCODER_PERIOD) - 1)
                                        << (ENCODER_PERIOD - (lcm % ENCODER_PERIOD)))) == 0 &&
                     sum(path, pathLen) == 0);
        if (!check) {
            int close_ow = OW + output_with_punc(table[acc_temp][3] ^ table[state][2],
                                                  pathLen * ENCODER_PERIOD, 0) + table[state][1];
            if (close_ow < dmax && state != 0) {
                path[pathLen] = state;
                pathLen++;
                fprintf(file, "%d/", countBinaryOnes(path, pathLen));
                for (j = 0; j < pathLen; j++)
                    fprintf(file, (j == pathLen - 1) ? "%d" : "%d,", path[j]);
                fprintf(file, ";%d \n",
                        OW + output_with_punc(table[acc_temp][3] ^ table[state][2],
                                              (pathLen - 1) * ENCODER_PERIOD, 0) + table[state][1]);
                pathLen--;
            }
        }
    }

    for (j = 0; j < (int)pow(2, ENCODER_PERIOD); j++)
        saveBranchesToFile_With_Punc_Size_K(table, state ^ j, j, OW, depth,
                                             file, path, pathLen, dmax, acc_temp, lcm);
    pathLen--;
}

void Transform_shape_to_shape_FOR_K(const char *filenameread,
                                    const char *outputname, int threshold)
{
    char comma;
    int bin[ENCODER_PERIOD];
    int input[threshold];
    int num, i, x[FRAME_SIZE / ENCODER_PERIOD + 1], size, index, distance;

    FILE *output = fopen(outputname, "w");
    FILE *file   = fopen(filenameread, "r");

    do {
        size = index = 0;
        fscanf(file, "%d%c", &num, &comma);
        if (comma != '/') break;
        do {
            fscanf(file, "%d%c", &x[size], &comma);
            int_to_binary(x[size], ENCODER_PERIOD, bin);
            for (i = 0; i < ENCODER_PERIOD; i++)
                if (bin[i] == 1) input[index++] = size * ENCODER_PERIOD + i;
            size++;
        } while (comma != ';');
        fscanf(file, "%d%c", &distance, &comma);
        print_line_in_file(output, num, distance, input);
    } while (!feof(file));

    fclose(file);
    fclose(output);
    distributeFiles(outputname, threshold, 1);
    merge_files(outputname, threshold);
}
