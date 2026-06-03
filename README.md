# ARP Interleaver Designer

A distance-driven design tool for **Almost Regular Permutation (ARP)** interleavers for parallel concatenated convolutional (turbo) codes.

The tool searches for an ARP shift vector **S[]** and period **P** that maximise the minimum Hamming distance (mHD) of the resulting turbo code, using a decoder-free RTZ-based estimation method.

**Code:** https://github.com/bazzal99/Decoder-Free-Turbo-mHD_Estimator

---

## Background

The ARP interleaver is defined by:

```
π(i) = (P · i + S[i mod Q]) mod K
```

where K is the frame size, P is the period, Q is the number of sub-layers, and S[] is the shift vector. Choosing S[] and P to maximise the mHD is the central challenge in turbo code interleaver design.

This tool solves that problem using a **layer-by-layer search** driven by the same decoder-free RTZ estimation technique introduced for LTE turbo code analysis. The algorithm is computationally efficient and applicable to arbitrary RSC encoders and puncturing patterns.

**Reference:**

> M. Bazzal, J. Nadal, S. Weithoffer, C. Abdel Nour, C. Douillard,  
> *"Distance-centric joint interleaver and structural code design for concatenated convolutional codes,"*  
> IEEE Open Journal of the Communications Society, 2025.

---

## Repository structure

```
arp-interleaver-designer/
├── include/
│   ├── config.h        — all tunable parameters (edit this first)
│   ├── encoder.h       — RSC encoder and distance computation API
│   ├── interleaver.h   — ARP interleaver construction API
│   ├── rtz_search.h    — RTZ graph-girth search kernel API
│   ├── designer.h      — interleaver design algorithm API
│   └── utils.h         — utility functions API
├── src/
│   ├── main.c          — entry point
│   ├── encoder.c       — RSC encoder, circular states, Turbo_distance
│   ├── interleaver.c   — ARP initialisation, period selection
│   ├── rtz_search.c    — IW2 and LTE-Multiple RTZ kernels
│   ├── designer.c      — layer-by-layer design loop
│   └── utils.c         — sorting, file I/O, branch table, deduplication
├── examples/
│   └── example_result.txt  — sample output format
├── Makefile
├── LICENSE
└── README.md
```

### Module responsibilities

| File | Responsibility |
|------|---------------|
| `config.h` | Single place for all tunable parameters |
| `encoder.c` | RSC encoding (ZT, tail-biting), circular state initialisation, codeword distance |
| `interleaver.c` | ARP forward/inverse maps, layer-by-layer initialisation, period selection |
| `rtz_search.c` | IW2 periodic kernel + LTE general kernel (write hits to `temp.txt`) |
| `designer.c` | Recursive layer builder, period loop, shift candidate evaluation |
| `utils.c` | Sorting, shuffling, file I/O, hash-based deduplication, RTZ branch table |

---

## Configuration

All parameters are in **`include/config.h`**. The most important ones:

```c
#define FRAME_SIZE        6144   /* Frame length K                        */
#define DELAYS            3      /* RSC memory (trellis states = 2^DELAYS)*/
#define G1_POLY           {1,1,0,1}  /* RSC feedforward polynomial G1    */
#define G2_POLY           {1,0,1,1}  /* RSC feedback polynomial G2       */
#define CIRCULAR_ENCODING 1      /* 1 = tail-biting, 0 = zero-termination*/
#define ARP_Q             8      /* Number of ARP sub-layers             */
#define DESIGN_THRESHOLD  13     /* Minimum acceptable mHD               */
#define SEARCH_TYPE       1      /* 0=file, 1=cycle, 2=both              */
#define MAX_CANDIDATES_PER_LAYER 1  /* Search breadth per layer          */
```

---

## Build

Requires a C99 compiler and the math library:

```bash
make
```

Or manually:

```bash
gcc -O2 -std=c99 -Iinclude -o arp_designer \
    src/main.c src/encoder.c src/interleaver.c \
    src/rtz_search.c src/designer.c src/utils.c -lm
```

---

## Usage

1. Edit `include/config.h` to match your encoder and target frame size.
2. Build: `make`
3. Run: `./arp_designer`

### Search modes (`SEARCH_TYPE`)

| Mode | Description |
|------|-------------|
| `0` — File-only | Evaluates candidates against a precomputed RTZ pattern file (`All_RTZ.txt`). Fastest once the file exists. |
| `1` — Cycle-only | Runs the RTZ graph search live for each candidate. No precomputed file needed. |
| `2` — Combined | Uses both; more thorough. |

For mode 0 or 2, generate `All_RTZ.txt` first by calling `saveAllBranchesToFile_WITH_Punc_WITH_SIZE_K()` and `Transform_shape_to_shape_FOR_K()` from `utils.h`.

### Output files

| File | Content |
|------|---------|
| `result.txt` | All valid interleavers found: `S[0],...,S[Q-1]  dmin=<d>  P=<p>  multiple=<m>` |
| `final_result.txt` | Best interleaver of the current round |

### Example output (`result.txt`)

```
1,3239,1417,4936,1460,52,5820,1203     dmin = 9  - P=733   - multiple = 2
3,4845,5926,5304,4918,792,5739,2121    dmin = 10 - P=3323  - multiple = 20
3,2033,3025,769,6101,2775,3947,5675    dmin = 11 - P=3019  - multiple = 10
7,1345,397,5727,2133,903,3643,4861     dmin = 12 - P=3635  - multiple = 122
```

---

## Algorithm overview

The design proceeds layer by layer:

1. **Period selection:** filter all P coprime with K by the minimum spread criterion.
2. **For each layer l = 0 … Q-1:**
   - Try candidate shift values S[l] using the ARP regularity constraint.
   - Evaluate the partial interleaver using RTZ search kernels.
   - Reject any S[l] that produces a codeword below the threshold.
3. **Increment threshold** after each successful interleaver and repeat.

Two RTZ kernels are used:
- **IW2 kernel:** period-7 steps, detects weight-2 periodic RTZ sequences.
- **LTE-Multiple kernel:** unit steps, detects weight-3+ sequences. Handles partially-initialised ARP interleavers by skipping undefined positions (value = -1).

---

## License

MIT License. See [LICENSE](LICENSE).

---

## Citation

```bibtex
@article{bazzal2025distance,
  author  = {Bazzal, Mohammad and Nadal, Jeremy and Weithoffer, Stefan
             and {Abdel Nour}, Charbel and Douillard, Catherine},
  title   = {Distance-centric joint interleaver and structural code design
             for concatenated convolutional codes},
  journal = {IEEE Open Journal of the Communications Society},
  year    = {2025}
}
```
