/**
 * @file interleaver.c
 * @brief ARP interleaver construction and period selection.
 */

#include <math.h>
#include <stdlib.h>
#include "config.h"
#include "interleaver.h"

/* =========================================================================
 * ARP interleaver initialisation
 * ========================================================================= */

void ARPInterleaverInitialization(int Size, int Period, int PeriodARP,
                                  int Shift[],
                                  int InterleavedAddress[],
                                  int OriginalAddress[])
{
    for (int i = 0; i < Size; i++) {
        OriginalAddress[i]                  = (int)((i * Period + Shift[i % PeriodARP]) % Size);
        InterleavedAddress[OriginalAddress[i]] = i;
    }
}

void ARPInterleaverInitialization_Per_Layer(int Size, int layer, int Period,
                                            int Q, int Shift[],
                                            int InterleavedAddress[],
                                            int OriginalAddress[])
{
    for (int i = layer; i < Size; i += Q) {
        OriginalAddress[i]                  = (int)((i * Period + Shift[i % Q]) % Size);
        InterleavedAddress[OriginalAddress[i]] = i;
    }
}

void ARPInterleaverInitialization_Till_Layer(int Size, int layer, int Period,
                                             int Q, int Shift[],
                                             int InterleavedAddress[],
                                             int OriginalAddress[])
{
    for (int j = 0; j <= layer; j++) {
        for (int i = j; i < Size; i += Q) {
            OriginalAddress[i]                  = (int)((i * Period + Shift[i % Q]) % Size);
            InterleavedAddress[OriginalAddress[i]] = i;
        }
    }
}

void Arp_DE_Interleave_Layer(int Size, int layer, int Period, int Q,
                             int Shift[],
                             int InterleavedAddress[],
                             int OriginalAddress[])
{
    for (int i = layer; i < FRAME_SIZE; i += Q) {
        OriginalAddress[i]             = (int)((i * Period + Shift[i % Q]) % Size);
        InterleavedAddress[OriginalAddress[i]] = -1;
        OriginalAddress[i]             = -1;
    }
}

int inter(int i, int p, int s[])
{
    return (p * i + s[i % ARP_Q]) % FRAME_SIZE;
}


/* =========================================================================
 * Period selection
 * ========================================================================= */

int gcd(int a, int b)
{
    return (b == 0) ? a : gcd(b, a % b);
}

int isRelativePrime(int a, int b)
{
    return gcd(a, b) == 1;
}

void saveRelativePrimes(int number, int *result, int *count)
{
    for (int i = 1; i <= number; i++)
        if (isRelativePrime(number, i))
            result[(*count)++] = i;
}

int Min_Spread(int p)
{
    int upper = (int)sqrt(2.0 * FRAME_SIZE);
    int interleaved[FRAME_SIZE];
    int smin = FRAME_SIZE;

    for (int i = 0; i < FRAME_SIZE; i++)
        interleaved[(p * i) % FRAME_SIZE] = i;

    for (int i = 0; i < FRAME_SIZE; i++) {
        for (int j = 1; j < upper; j++) {
            if (i != j) {
                int x = abs(j) + abs(interleaved[i] - interleaved[(i + j) % FRAME_SIZE]);
                if (x < smin) smin = x;
            }
        }
    }
    return smin;
}

void Choose_Period(int periods[], int *size, int framesize)
{
    int result[framesize];
    int count = 0;
    saveRelativePrimes(framesize, result, &count);

    for (int p = 0; p < count; p++) {
        if (Min_Spread(result[p]) >= ((int)sqrt(2.0 * framesize) - SPREAD_OFFSET)) {
            periods[(*size)++] = result[p];
        }
    }
}
