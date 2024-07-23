#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "freq_search.h"

/* Tables of rPLL output frequencies available on both Tang Nano 9K and
 * Tang Nano 20K and both of them assuming a 27 MHz input clock on both.
 *
 * idiv_sel, fbdiv_sel, and odiv_sel are the rPLL parameters needed to
 * get the frequency.  */

typedef struct {
  double freq; /* MHz */
  int idiv_sel;
  int fbdiv_sel;
  int odiv_sel;
} freq_table;

static freq_table freqs_9k[] = {
  {3.375000, 7, 0, 128},
  {3.857143, 6, 0, 112},
  {4.500000, 5, 0, 96},
  {5.400000, 4, 0, 80},
  {6.000000, 8, 1, 80},
  {6.750000, 3, 0, 64},
  {7.714286, 6, 1, 64},
  {9.000000, 2, 0, 48},
  {10.125000, 7, 2, 48},
  {10.800000, 4, 1, 48},
  {11.571429, 6, 2, 48},
  {12.000000, 8, 3, 48},
  {13.500000, 1, 0, 32},
  {15.000000, 8, 4, 32},
  {15.428571, 6, 3, 32},
  {16.200000, 4, 2, 32},
  {16.875000, 7, 4, 32},
  {18.000000, 2, 1, 32},
  {19.285714, 6, 4, 32},
  {20.250000, 3, 2, 32},
  {21.000000, 8, 6, 32},
  {21.600000, 4, 3, 32},
  {22.500000, 5, 4, 32},
  {23.142857, 6, 5, 32},
  {23.625000, 7, 6, 32},
  {24.000000, 8, 7, 32},
  {27.000000, 0, 0, 16},
  {30.000000, 8, 9, 16},
  {30.375000, 7, 8, 16},
  {30.857143, 6, 7, 16},
  {31.500000, 5, 6, 16},
  {32.400000, 4, 5, 16},
  {33.000000, 8, 10, 16},
  {33.750000, 3, 4, 16},
  {34.714286, 6, 8, 16},
  {36.000000, 2, 3, 16},
  {37.125000, 7, 10, 16},
  {37.800000, 4, 6, 16},
  {38.571429, 6, 9, 16},
  {39.000000, 8, 12, 16},
  {40.500000, 1, 2, 16},
  {42.000000, 8, 13, 16},
  {42.428571, 6, 10, 16},
  {43.200000, 4, 7, 16},
  {43.875000, 7, 12, 16},
  {45.000000, 2, 4, 16},
  {46.285714, 6, 11, 16},
  {47.250000, 3, 6, 16},
  {48.000000, 8, 15, 16},
  {48.600000, 4, 8, 16},
  {49.500000, 5, 10, 16},
  {50.142857, 6, 12, 8},
  {50.625000, 7, 14, 8},
  {51.000000, 8, 16, 8},
  {54.000000, 0, 1, 8},
  {57.000000, 8, 18, 8},
  {57.375000, 7, 16, 8},
  {57.857143, 6, 14, 8},
  {58.500000, 5, 12, 8},
  {59.400000, 4, 10, 8},
  {60.000000, 8, 19, 8},
  {60.750000, 3, 8, 8},
  {61.714286, 6, 15, 8},
  {63.000000, 2, 6, 8},
  {64.125000, 7, 18, 8},
  {64.800000, 4, 11, 8},
  {65.571429, 6, 16, 8},
  {66.000000, 8, 21, 8},
  {67.500000, 1, 4, 8},
  {69.000000, 8, 22, 8},
  {69.428571, 6, 17, 8},
  {70.200000, 4, 12, 8},
  {70.875000, 7, 20, 8},
  {72.000000, 2, 7, 8},
  {73.285714, 6, 18, 8},
  {74.250000, 3, 10, 8},
  {75.000000, 8, 24, 8},
  {75.600000, 4, 13, 8},
  {76.500000, 5, 16, 8},
  {77.142857, 6, 19, 8},
  {77.625000, 7, 22, 8},
  {78.000000, 8, 25, 8},
  {81.000000, 0, 2, 8},
  {84.000000, 8, 27, 8},
  {84.375000, 7, 24, 8},
  {84.857143, 6, 21, 8},
  {85.500000, 5, 18, 8},
  {86.400000, 4, 15, 8},
  {87.000000, 8, 28, 8},
  {87.750000, 3, 12, 8},
  {88.714286, 6, 22, 8},
  {90.000000, 2, 9, 8},
  {91.125000, 7, 26, 8},
  {91.800000, 4, 16, 8},
  {92.571429, 6, 23, 8},
  {93.000000, 8, 30, 8},
  {94.500000, 1, 6, 8},
  {96.000000, 8, 31, 8},
  {96.428571, 6, 24, 8},
  {97.200000, 4, 17, 8},
  {97.875000, 7, 28, 8},
  {99.000000, 2, 10, 8},
  {100.285714, 6, 25, 4},
  {101.250000, 3, 14, 4},
  {102.000000, 8, 33, 4},
  {102.600000, 4, 18, 4},
  {103.500000, 5, 22, 4},
  {104.142857, 6, 26, 4},
  {104.625000, 7, 30, 4},
  {105.000000, 8, 34, 4},
  {108.000000, 0, 3, 4},
  {111.000000, 8, 36, 4},
  {111.375000, 7, 32, 4},
  {111.857143, 6, 28, 4},
  {112.500000, 5, 24, 4},
  {113.400000, 4, 20, 4},
  {114.000000, 8, 37, 4},
  {114.750000, 3, 16, 4},
  {115.714286, 6, 29, 4},
  {117.000000, 2, 12, 4},
  {118.125000, 7, 34, 4},
  {118.800000, 4, 21, 4},
  {119.571429, 6, 30, 4},
  {120.000000, 8, 39, 4},
  {121.500000, 1, 8, 4},
  {123.000000, 8, 40, 4},
  {123.428571, 6, 31, 4},
  {124.200000, 4, 22, 4},
  {124.875000, 7, 36, 4},
  {126.000000, 2, 13, 4},
  {127.285714, 6, 32, 4},
  {128.250000, 3, 18, 4},
  {129.000000, 8, 42, 4},
  {129.600000, 4, 23, 4},
  {130.500000, 5, 28, 4},
  {131.142857, 6, 33, 4},
  {131.625000, 7, 38, 4},
  {132.000000, 8, 43, 4},
  {135.000000, 0, 4, 4},
  {138.000000, 8, 45, 4},
  {138.375000, 7, 40, 4},
  {138.857143, 6, 35, 4},
  {139.500000, 5, 30, 4},
  {140.400000, 4, 25, 4},
  {141.000000, 8, 46, 4},
  {141.750000, 3, 20, 4},
  {142.714286, 6, 36, 4},
  {144.000000, 2, 15, 4},
  {145.125000, 7, 42, 4},
  {145.800000, 4, 26, 4},
  {146.571429, 6, 37, 4},
  {147.000000, 8, 48, 4},
  {148.500000, 1, 10, 4},
  {150.000000, 8, 49, 4},
  {150.428571, 6, 38, 4},
  {151.200000, 4, 27, 4},
  {151.875000, 7, 44, 4},
  {153.000000, 2, 16, 4},
  {154.285714, 6, 39, 4},
  {155.250000, 3, 22, 4},
  {156.000000, 8, 51, 4},
  {156.600000, 4, 28, 4},
  {157.500000, 5, 34, 4},
  {158.142857, 6, 40, 4},
  {158.625000, 7, 46, 4},
  {159.000000, 8, 52, 4},
  {162.000000, 0, 5, 4},
  {165.000000, 8, 54, 4},
  {165.375000, 7, 48, 4},
  {165.857143, 6, 42, 4},
  {166.500000, 5, 36, 4},
  {167.400000, 4, 30, 4},
  {168.000000, 8, 55, 4},
  {168.750000, 3, 24, 4},
  {169.714286, 6, 43, 4},
  {171.000000, 2, 18, 4},
  {172.125000, 7, 50, 4},
  {172.800000, 4, 31, 4},
  {173.571429, 6, 44, 4},
  {174.000000, 8, 57, 4},
  {175.500000, 1, 12, 4},
  {177.000000, 8, 58, 4},
  {177.428571, 6, 45, 4},
  {178.200000, 4, 32, 4},
  {178.875000, 7, 52, 4},
  {180.000000, 2, 19, 4},
  {181.285714, 6, 46, 4},
  {182.250000, 3, 26, 4},
  {183.000000, 8, 60, 4},
  {183.600000, 4, 33, 4},
  {184.500000, 5, 40, 4},
  {185.142857, 6, 47, 4},
  {185.625000, 7, 54, 4},
  {186.000000, 8, 61, 4},
  {189.000000, 0, 6, 4},
  {192.375000, 7, 56, 4},
  {192.857143, 6, 49, 4},
  {193.500000, 5, 42, 4},
  {194.400000, 4, 35, 4},
  {195.750000, 3, 28, 4},
  {196.714286, 6, 50, 4},
  {198.000000, 2, 21, 4},
  {199.125000, 7, 58, 4},
  {199.800000, 4, 36, 4},
  {200.571429, 6, 51, 2},
  {202.500000, 1, 14, 2},
  {204.428571, 6, 52, 2},
  {205.200000, 4, 37, 2},
  {205.875000, 7, 60, 2},
  {207.000000, 2, 22, 2},
  {208.285714, 6, 53, 2},
  {209.250000, 3, 30, 2},
  {210.600000, 4, 38, 2},
  {211.500000, 5, 46, 2},
  {212.142857, 6, 54, 2},
  {212.625000, 7, 62, 2},
  {216.000000, 0, 7, 2},
  {219.857143, 6, 56, 2},
  {220.500000, 5, 48, 2},
  {221.400000, 4, 40, 2},
  {222.750000, 3, 32, 2},
  {223.714286, 6, 57, 2},
  {225.000000, 2, 24, 2},
  {226.800000, 4, 41, 2},
  {227.571429, 6, 58, 2},
  {229.500000, 1, 16, 2},
  {231.428571, 6, 59, 2},
  {232.200000, 4, 42, 2},
  {234.000000, 2, 25, 2},
  {235.285714, 6, 60, 2},
  {236.250000, 3, 34, 2},
  {237.600000, 4, 43, 2},
  {238.500000, 5, 52, 2},
  {239.142857, 6, 61, 2},
  {243.000000, 0, 8, 2},
  {247.500000, 5, 54, 2},
  {248.400000, 4, 45, 2},
  {249.750000, 3, 36, 2},
  {252.000000, 2, 27, 2},
  {253.800000, 4, 46, 2},
  {256.500000, 1, 18, 2},
  {259.200000, 4, 47, 2},
  {261.000000, 2, 28, 2},
  {263.250000, 3, 38, 2},
  {264.600000, 4, 48, 2},
  {265.500000, 5, 58, 2},
  {270.000000, 0, 9, 2},
  {274.500000, 5, 60, 2},
  {275.400000, 4, 50, 2},
  {276.750000, 3, 40, 2},
  {279.000000, 2, 30, 2},
  {280.800000, 4, 51, 2},
  {283.500000, 1, 20, 2},
  {286.200000, 4, 52, 2},
  {288.000000, 2, 31, 2},
  {290.250000, 3, 42, 2},
  {291.600000, 4, 53, 2},
  {297.000000, 0, 10, 2},
  {302.400000, 4, 55, 2},
  {303.750000, 3, 44, 2},
  {306.000000, 2, 33, 2},
  {307.800000, 4, 56, 2},
  {310.500000, 1, 22, 2},
  {313.200000, 4, 57, 2},
  {315.000000, 2, 34, 2},
  {317.250000, 3, 46, 2},
  {318.600000, 4, 58, 2},
  {324.000000, 0, 11, 2},
  {329.400000, 4, 60, 2},
  {330.750000, 3, 48, 2},
  {333.000000, 2, 36, 2},
  {334.800000, 4, 61, 2},
  {337.500000, 1, 24, 2},
  {340.200000, 4, 62, 2},
  {342.000000, 2, 37, 2},
  {344.250000, 3, 50, 2},
  {351.000000, 0, 12, 2},
  {357.750000, 3, 52, 2},
  {360.000000, 2, 39, 2},
  {364.500000, 1, 26, 2},
  {369.000000, 2, 40, 2},
  {371.250000, 3, 54, 2},
  {378.000000, 0, 13, 2},
  {384.750000, 3, 56, 2},
  {387.000000, 2, 42, 2},
  {391.500000, 1, 28, 2},
  {396.000000, 2, 43, 2},
  {398.250000, 3, 58, 2},
  {405.000000, 0, 14, 2},
  {411.750000, 3, 60, 2},
  {414.000000, 2, 45, 2},
  {418.500000, 1, 30, 2},
  {423.000000, 2, 46, 2},
  {425.250000, 3, 62, 2},
  {432.000000, 0, 15, 2},
  {441.000000, 2, 48, 2},
  {445.500000, 1, 32, 2},
  {450.000000, 2, 49, 2},
  {459.000000, 0, 16, 2},
  {468.000000, 2, 51, 2},
  {472.500000, 1, 34, 2},
  {477.000000, 2, 52, 2},
  {486.000000, 0, 17, 2},
  {495.000000, 2, 54, 2},
  {499.500000, 1, 36, 2},
  {504.000000, 2, 55, 2},
  {513.000000, 0, 18, 2},
  {522.000000, 2, 57, 2},
  {526.500000, 1, 38, 2},
  {531.000000, 2, 58, 2},
  {540.000000, 0, 19, 2},
  {549.000000, 2, 60, 2},
  {553.500000, 1, 40, 2},
  {558.000000, 2, 61, 2},
  {567.000000, 0, 20, 2},
  {580.500000, 1, 42, 2},
  {594.000000, 0, 21, 2}
};

static freq_table freqs_20k[] = {
  {4.500000, 5, 0, 112},
  {5.400000, 4, 0, 96},
  {6.000000, 8, 1, 96},
  {6.750000, 3, 0, 80},
  {7.714286, 6, 1, 80},
  {9.000000, 2, 0, 64},
  {10.125000, 7, 2, 64},
  {10.800000, 4, 1, 48},
  {11.571429, 6, 2, 48},
  {12.000000, 8, 3, 48},
  {13.500000, 1, 0, 48},
  {15.000000, 8, 4, 48},
  {15.428571, 6, 3, 48},
  {16.200000, 4, 2, 32},
  {16.875000, 7, 4, 32},
  {18.000000, 2, 1, 32},
  {19.285714, 6, 4, 32},
  {20.250000, 3, 2, 32},
  {21.000000, 8, 6, 32},
  {21.600000, 4, 3, 32},
  {22.500000, 5, 4, 32},
  {23.142857, 6, 5, 32},
  {23.625000, 7, 6, 32},
  {24.000000, 8, 7, 32},
  {27.000000, 0, 0, 32},
  {30.000000, 8, 9, 32},
  {30.375000, 7, 8, 32},
  {30.857143, 6, 7, 32},
  {31.500000, 5, 6, 16},
  {32.400000, 4, 5, 16},
  {33.000000, 8, 10, 16},
  {33.750000, 3, 4, 16},
  {34.714286, 6, 8, 16},
  {36.000000, 2, 3, 16},
  {37.125000, 7, 10, 16},
  {37.800000, 4, 6, 16},
  {38.571429, 6, 9, 16},
  {39.000000, 8, 12, 16},
  {40.500000, 1, 2, 16},
  {42.000000, 8, 13, 16},
  {42.428571, 6, 10, 16},
  {43.200000, 4, 7, 16},
  {43.875000, 7, 12, 16},
  {45.000000, 2, 4, 16},
  {46.285714, 6, 11, 16},
  {47.250000, 3, 6, 16},
  {48.000000, 8, 15, 16},
  {48.600000, 4, 8, 16},
  {49.500000, 5, 10, 16},
  {50.142857, 6, 12, 16},
  {50.625000, 7, 14, 16},
  {51.000000, 8, 16, 16},
  {54.000000, 0, 1, 16},
  {57.000000, 8, 18, 16},
  {57.375000, 7, 16, 16},
  {57.857143, 6, 14, 16},
  {58.500000, 5, 12, 16},
  {59.400000, 4, 10, 16},
  {60.000000, 8, 19, 16},
  {60.750000, 3, 8, 16},
  {61.714286, 6, 15, 16},
  {63.000000, 2, 6, 8},
  {64.125000, 7, 18, 8},
  {64.800000, 4, 11, 8},
  {65.571429, 6, 16, 8},
  {66.000000, 8, 21, 8},
  {67.500000, 1, 4, 8},
  {69.000000, 8, 22, 8},
  {69.428571, 6, 17, 8},
  {70.200000, 4, 12, 8},
  {70.875000, 7, 20, 8},
  {72.000000, 2, 7, 8},
  {73.285714, 6, 18, 8},
  {74.250000, 3, 10, 8},
  {75.000000, 8, 24, 8},
  {75.600000, 4, 13, 8},
  {76.500000, 5, 16, 8},
  {77.142857, 6, 19, 8},
  {77.625000, 7, 22, 8},
  {78.000000, 8, 25, 8},
  {81.000000, 0, 2, 8},
  {84.000000, 8, 27, 8},
  {84.375000, 7, 24, 8},
  {84.857143, 6, 21, 8},
  {85.500000, 5, 18, 8},
  {86.400000, 4, 15, 8},
  {87.000000, 8, 28, 8},
  {87.750000, 3, 12, 8},
  {88.714286, 6, 22, 8},
  {90.000000, 2, 9, 8},
  {91.125000, 7, 26, 8},
  {91.800000, 4, 16, 8},
  {92.571429, 6, 23, 8},
  {93.000000, 8, 30, 8},
  {94.500000, 1, 6, 8},
  {96.000000, 8, 31, 8},
  {96.428571, 6, 24, 8},
  {97.200000, 4, 17, 8},
  {97.875000, 7, 28, 8},
  {99.000000, 2, 10, 8},
  {100.285714, 6, 25, 8},
  {101.250000, 3, 14, 8},
  {102.000000, 8, 33, 8},
  {102.600000, 4, 18, 8},
  {103.500000, 5, 22, 8},
  {104.142857, 6, 26, 8},
  {104.625000, 7, 30, 8},
  {105.000000, 8, 34, 8},
  {108.000000, 0, 3, 8},
  {111.000000, 8, 36, 8},
  {111.375000, 7, 32, 8},
  {111.857143, 6, 28, 8},
  {112.500000, 5, 24, 8},
  {113.400000, 4, 20, 8},
  {114.000000, 8, 37, 8},
  {114.750000, 3, 16, 8},
  {115.714286, 6, 29, 8},
  {117.000000, 2, 12, 8},
  {118.125000, 7, 34, 8},
  {118.800000, 4, 21, 8},
  {119.571429, 6, 30, 8},
  {120.000000, 8, 39, 8},
  {121.500000, 1, 8, 8},
  {123.000000, 8, 40, 8},
  {123.428571, 6, 31, 8},
  {124.200000, 4, 22, 8},
  {124.875000, 7, 36, 8},
  {126.000000, 2, 13, 4},
  {127.285714, 6, 32, 4},
  {128.250000, 3, 18, 4},
  {129.000000, 8, 42, 4},
  {129.600000, 4, 23, 4},
  {130.500000, 5, 28, 4},
  {131.142857, 6, 33, 4},
  {131.625000, 7, 38, 4},
  {132.000000, 8, 43, 4},
  {135.000000, 0, 4, 4},
  {138.000000, 8, 45, 4},
  {138.375000, 7, 40, 4},
  {138.857143, 6, 35, 4},
  {139.500000, 5, 30, 4},
  {140.400000, 4, 25, 4},
  {141.000000, 8, 46, 4},
  {141.750000, 3, 20, 4},
  {142.714286, 6, 36, 4},
  {144.000000, 2, 15, 4},
  {145.125000, 7, 42, 4},
  {145.800000, 4, 26, 4},
  {146.571429, 6, 37, 4},
  {147.000000, 8, 48, 4},
  {148.500000, 1, 10, 4},
  {150.000000, 8, 49, 4},
  {150.428571, 6, 38, 4},
  {151.200000, 4, 27, 4},
  {151.875000, 7, 44, 4},
  {153.000000, 2, 16, 4},
  {154.285714, 6, 39, 4},
  {155.250000, 3, 22, 4},
  {156.000000, 8, 51, 4},
  {156.600000, 4, 28, 4},
  {157.500000, 5, 34, 4},
  {158.142857, 6, 40, 4},
  {158.625000, 7, 46, 4},
  {159.000000, 8, 52, 4},
  {162.000000, 0, 5, 4},
  {165.000000, 8, 54, 4},
  {165.375000, 7, 48, 4},
  {165.857143, 6, 42, 4},
  {166.500000, 5, 36, 4},
  {167.400000, 4, 30, 4},
  {168.000000, 8, 55, 4},
  {168.750000, 3, 24, 4},
  {169.714286, 6, 43, 4},
  {171.000000, 2, 18, 4},
  {172.125000, 7, 50, 4},
  {172.800000, 4, 31, 4},
  {173.571429, 6, 44, 4},
  {174.000000, 8, 57, 4},
  {175.500000, 1, 12, 4},
  {177.000000, 8, 58, 4},
  {177.428571, 6, 45, 4},
  {178.200000, 4, 32, 4},
  {178.875000, 7, 52, 4},
  {180.000000, 2, 19, 4},
  {181.285714, 6, 46, 4},
  {182.250000, 3, 26, 4},
  {183.000000, 8, 60, 4},
  {183.600000, 4, 33, 4},
  {184.500000, 5, 40, 4},
  {185.142857, 6, 47, 4},
  {185.625000, 7, 54, 4},
  {186.000000, 8, 61, 4},
  {189.000000, 0, 6, 4},
  {192.375000, 7, 56, 4},
  {192.857143, 6, 49, 4},
  {193.500000, 5, 42, 4},
  {194.400000, 4, 35, 4},
  {195.750000, 3, 28, 4},
  {196.714286, 6, 50, 4},
  {198.000000, 2, 21, 4},
  {199.125000, 7, 58, 4},
  {199.800000, 4, 36, 4},
  {200.571429, 6, 51, 4},
  {202.500000, 1, 14, 4},
  {204.428571, 6, 52, 4},
  {205.200000, 4, 37, 4},
  {205.875000, 7, 60, 4},
  {207.000000, 2, 22, 4},
  {208.285714, 6, 53, 4},
  {209.250000, 3, 30, 4},
  {210.600000, 4, 38, 4},
  {211.500000, 5, 46, 4},
  {212.142857, 6, 54, 4},
  {212.625000, 7, 62, 4},
  {216.000000, 0, 7, 4},
  {219.857143, 6, 56, 4},
  {220.500000, 5, 48, 4},
  {221.400000, 4, 40, 4},
  {222.750000, 3, 32, 4},
  {223.714286, 6, 57, 4},
  {225.000000, 2, 24, 4},
  {226.800000, 4, 41, 4},
  {227.571429, 6, 58, 4},
  {229.500000, 1, 16, 4},
  {231.428571, 6, 59, 4},
  {232.200000, 4, 42, 4},
  {234.000000, 2, 25, 4},
  {235.285714, 6, 60, 4},
  {236.250000, 3, 34, 4},
  {237.600000, 4, 43, 4},
  {238.500000, 5, 52, 4},
  {239.142857, 6, 61, 4},
  {243.000000, 0, 8, 4},
  {247.500000, 5, 54, 4},
  {248.400000, 4, 45, 4},
  {249.750000, 3, 36, 4},
  {252.000000, 2, 27, 2},
  {253.800000, 4, 46, 2},
  {256.500000, 1, 18, 2},
  {259.200000, 4, 47, 2},
  {261.000000, 2, 28, 2},
  {263.250000, 3, 38, 2},
  {264.600000, 4, 48, 2},
  {265.500000, 5, 58, 2},
  {270.000000, 0, 9, 2},
  {274.500000, 5, 60, 2},
  {275.400000, 4, 50, 2},
  {276.750000, 3, 40, 2},
  {279.000000, 2, 30, 2},
  {280.800000, 4, 51, 2},
  {283.500000, 1, 20, 2},
  {286.200000, 4, 52, 2},
  {288.000000, 2, 31, 2},
  {290.250000, 3, 42, 2},
  {291.600000, 4, 53, 2},
  {297.000000, 0, 10, 2},
  {302.400000, 4, 55, 2},
  {303.750000, 3, 44, 2},
  {306.000000, 2, 33, 2},
  {307.800000, 4, 56, 2},
  {310.500000, 1, 22, 2},
  {313.200000, 4, 57, 2},
  {315.000000, 2, 34, 2},
  {317.250000, 3, 46, 2},
  {318.600000, 4, 58, 2},
  {324.000000, 0, 11, 2},
  {329.400000, 4, 60, 2},
  {330.750000, 3, 48, 2},
  {333.000000, 2, 36, 2},
  {334.800000, 4, 61, 2},
  {337.500000, 1, 24, 2},
  {340.200000, 4, 62, 2},
  {342.000000, 2, 37, 2},
  {344.250000, 3, 50, 2},
  {351.000000, 0, 12, 2},
  {357.750000, 3, 52, 2},
  {360.000000, 2, 39, 2},
  {364.500000, 1, 26, 2},
  {369.000000, 2, 40, 2},
  {371.250000, 3, 54, 2},
  {378.000000, 0, 13, 2},
  {384.750000, 3, 56, 2},
  {387.000000, 2, 42, 2},
  {391.500000, 1, 28, 2},
  {396.000000, 2, 43, 2},
  {398.250000, 3, 58, 2},
  {405.000000, 0, 14, 2},
  {411.750000, 3, 60, 2},
  {414.000000, 2, 45, 2},
  {418.500000, 1, 30, 2},
  {423.000000, 2, 46, 2},
  {425.250000, 3, 62, 2},
  {432.000000, 0, 15, 2},
  {441.000000, 2, 48, 2},
  {445.500000, 1, 32, 2},
  {450.000000, 2, 49, 2},
  {459.000000, 0, 16, 2},
  {468.000000, 2, 51, 2},
  {472.500000, 1, 34, 2},
  {477.000000, 2, 52, 2},
  {486.000000, 0, 17, 2},
  {495.000000, 2, 54, 2},
  {499.500000, 1, 36, 2},
  {504.000000, 2, 55, 2},
  {513.000000, 0, 18, 2},
  {522.000000, 2, 57, 2},
  {526.500000, 1, 38, 2},
  {531.000000, 2, 58, 2},
  {540.000000, 0, 19, 2},
  {549.000000, 2, 60, 2},
  {553.500000, 1, 40, 2},
  {558.000000, 2, 61, 2},
  {567.000000, 0, 20, 2},
  {580.500000, 1, 42, 2},
  {594.000000, 0, 21, 2},
  {607.500000, 1, 44, 2},
  {621.000000, 0, 22, 2}
};
 
static freq_table freqs_both[] = {
  {4.5000, 5, 0, 112},
  {5.4000, 4, 0, 96},
  {6.0000, 8, 1, 96},
  {6.7500, 3, 0, 80},
  {7.7143, 6, 1, 80},
  {9.0000, 2, 0, 64},
  {10.1250, 7, 2, 64},
  {10.8000, 4, 1, 48},
  {11.5714, 6, 2, 48},
  {12.0000, 8, 3, 48},
  {13.5000, 1, 0, 48},
  {15.0000, 8, 4, 48},
  {15.4286, 6, 3, 48},
  {16.2000, 4, 2, 32},
  {16.8750, 7, 4, 32},
  {18.0000, 2, 1, 32},
  {19.2857, 6, 4, 32},
  {20.2500, 3, 2, 32},
  {21.0000, 8, 6, 32},
  {21.6000, 4, 3, 32},
  {22.5000, 5, 4, 32},
  {23.1429, 6, 5, 32},
  {23.6250, 7, 6, 32},
  {24.0000, 8, 7, 32},
  {27.0000, 0, 0, 32},
  {30.0000, 8, 9, 32},
  {30.3750, 7, 8, 32},
  {30.8571, 6, 7, 32},
  {31.5000, 5, 6, 16},
  {32.4000, 4, 5, 16},
  {33.0000, 8, 10, 16},
  {33.7500, 3, 4, 16},
  {34.7143, 6, 8, 16},
  {36.0000, 2, 3, 16},
  {37.1250, 7, 10, 16},
  {37.8000, 4, 6, 16},
  {38.5714, 6, 9, 16},
  {39.0000, 8, 12, 16},
  {40.5000, 1, 2, 16},
  {42.0000, 8, 13, 16},
  {42.4286, 6, 10, 16},
  {43.2000, 4, 7, 16},
  {43.8750, 7, 12, 16},
  {45.0000, 2, 4, 16},
  {46.2857, 6, 11, 16},
  {47.2500, 3, 6, 16},
  {48.0000, 8, 15, 16},
  {48.6000, 4, 8, 16},
  {49.5000, 5, 10, 16},
  {50.1429, 6, 12, 16},
  {50.6250, 7, 14, 16},
  {51.0000, 8, 16, 16},
  {54.0000, 0, 1, 16},
  {57.0000, 8, 18, 16},
  {57.3750, 7, 16, 16},
  {57.8571, 6, 14, 16},
  {58.5000, 5, 12, 16},
  {59.4000, 4, 10, 16},
  {60.0000, 8, 19, 16},
  {60.7500, 3, 8, 16},
  {61.7143, 6, 15, 16},
  {63.0000, 2, 6, 8},
  {64.1250, 7, 18, 8},
  {64.8000, 4, 11, 8},
  {65.5714, 6, 16, 8},
  {66.0000, 8, 21, 8},
  {67.5000, 1, 4, 8},
  {69.0000, 8, 22, 8},
  {69.4286, 6, 17, 8},
  {70.2000, 4, 12, 8},
  {70.8750, 7, 20, 8},
  {72.0000, 2, 7, 8},
  {73.2857, 6, 18, 8},
  {74.2500, 3, 10, 8},
  {75.0000, 8, 24, 8},
  {75.6000, 4, 13, 8},
  {76.5000, 5, 16, 8},
  {77.1429, 6, 19, 8},
  {77.6250, 7, 22, 8},
  {78.0000, 8, 25, 8},
  {81.0000, 0, 2, 8},
  {84.0000, 8, 27, 8},
  {84.3750, 7, 24, 8},
  {84.8571, 6, 21, 8},
  {85.5000, 5, 18, 8},
  {86.4000, 4, 15, 8},
  {87.0000, 8, 28, 8},
  {87.7500, 3, 12, 8},
  {88.7143, 6, 22, 8},
  {90.0000, 2, 9, 8},
  {91.1250, 7, 26, 8},
  {91.8000, 4, 16, 8},
  {92.5714, 6, 23, 8},
  {93.0000, 8, 30, 8},
  {94.5000, 1, 6, 8},
  {96.0000, 8, 31, 8},
  {96.4286, 6, 24, 8},
  {97.2000, 4, 17, 8},
  {97.8750, 7, 28, 8},
  {99.0000, 2, 10, 8},
  {100.2857, 6, 25, 8},
  {101.2500, 3, 14, 8},
  {102.0000, 8, 33, 8},
  {102.6000, 4, 18, 8},
  {103.5000, 5, 22, 8},
  {104.1429, 6, 26, 8},
  {104.6250, 7, 30, 8},
  {105.0000, 8, 34, 8},
  {108.0000, 0, 3, 8},
  {111.0000, 8, 36, 8},
  {111.3750, 7, 32, 8},
  {111.8571, 6, 28, 8},
  {112.5000, 5, 24, 8},
  {113.4000, 4, 20, 8},
  {114.0000, 8, 37, 8},
  {114.7500, 3, 16, 8},
  {115.7143, 6, 29, 8},
  {117.0000, 2, 12, 8},
  {118.1250, 7, 34, 8},
  {118.8000, 4, 21, 8},
  {119.5714, 6, 30, 8},
  {120.0000, 8, 39, 8},
  {121.5000, 1, 8, 8},
  {123.0000, 8, 40, 8},
  {123.4286, 6, 31, 8},
  {124.2000, 4, 22, 8},
  {124.8750, 7, 36, 8},
  {126.0000, 2, 13, 4},
  {127.2857, 6, 32, 4},
  {128.2500, 3, 18, 4},
  {129.0000, 8, 42, 4},
  {129.6000, 4, 23, 4},
  {130.5000, 5, 28, 4},
  {131.1429, 6, 33, 4},
  {131.6250, 7, 38, 4},
  {132.0000, 8, 43, 4},
  {135.0000, 0, 4, 4},
  {138.0000, 8, 45, 4},
  {138.3750, 7, 40, 4},
  {138.8571, 6, 35, 4},
  {139.5000, 5, 30, 4},
  {140.4000, 4, 25, 4},
  {141.0000, 8, 46, 4},
  {141.7500, 3, 20, 4},
  {142.7143, 6, 36, 4},
  {144.0000, 2, 15, 4},
  {145.1250, 7, 42, 4},
  {145.8000, 4, 26, 4},
  {146.5714, 6, 37, 4},
  {147.0000, 8, 48, 4},
  {148.5000, 1, 10, 4},
  {150.0000, 8, 49, 4},
  {150.4286, 6, 38, 4},
  {151.2000, 4, 27, 4},
  {151.8750, 7, 44, 4},
  {153.0000, 2, 16, 4},
  {154.2857, 6, 39, 4},
  {155.2500, 3, 22, 4},
  {156.0000, 8, 51, 4},
  {156.6000, 4, 28, 4},
  {157.5000, 5, 34, 4},
  {158.1429, 6, 40, 4},
  {158.6250, 7, 46, 4},
  {159.0000, 8, 52, 4},
  {162.0000, 0, 5, 4},
  {165.0000, 8, 54, 4},
  {165.3750, 7, 48, 4},
  {165.8571, 6, 42, 4},
  {166.5000, 5, 36, 4},
  {167.4000, 4, 30, 4},
  {168.0000, 8, 55, 4},
  {168.7500, 3, 24, 4},
  {169.7143, 6, 43, 4},
  {171.0000, 2, 18, 4},
  {172.1250, 7, 50, 4},
  {172.8000, 4, 31, 4},
  {173.5714, 6, 44, 4},
  {174.0000, 8, 57, 4},
  {175.5000, 1, 12, 4},
  {177.0000, 8, 58, 4},
  {177.4286, 6, 45, 4},
  {178.2000, 4, 32, 4},
  {178.8750, 7, 52, 4},
  {180.0000, 2, 19, 4},
  {181.2857, 6, 46, 4},
  {182.2500, 3, 26, 4},
  {183.0000, 8, 60, 4},
  {183.6000, 4, 33, 4},
  {184.5000, 5, 40, 4},
  {185.1429, 6, 47, 4},
  {185.6250, 7, 54, 4},
  {186.0000, 8, 61, 4},
  {189.0000, 0, 6, 4},
  {192.3750, 7, 56, 4},
  {192.8571, 6, 49, 4},
  {193.5000, 5, 42, 4},
  {194.4000, 4, 35, 4},
  {195.7500, 3, 28, 4},
  {196.7143, 6, 50, 4},
  {198.0000, 2, 21, 4},
  {199.1250, 7, 58, 4},
  {199.8000, 4, 36, 4},
  {200.5714, 6, 51, 4},
  {202.5000, 1, 14, 4},
  {204.4286, 6, 52, 4},
  {205.2000, 4, 37, 4},
  {205.8750, 7, 60, 4},
  {207.0000, 2, 22, 4},
  {208.2857, 6, 53, 4},
  {209.2500, 3, 30, 4},
  {210.6000, 4, 38, 4},
  {211.5000, 5, 46, 4},
  {212.1429, 6, 54, 4},
  {212.6250, 7, 62, 4},
  {216.0000, 0, 7, 4},
  {219.8571, 6, 56, 4},
  {220.5000, 5, 48, 4},
  {221.4000, 4, 40, 4},
  {222.7500, 3, 32, 4},
  {223.7143, 6, 57, 4},
  {225.0000, 2, 24, 4},
  {226.8000, 4, 41, 4},
  {227.5714, 6, 58, 4},
  {229.5000, 1, 16, 4},
  {231.4286, 6, 59, 4},
  {232.2000, 4, 42, 4},
  {234.0000, 2, 25, 4},
  {235.2857, 6, 60, 4},
  {236.2500, 3, 34, 4},
  {237.6000, 4, 43, 4},
  {238.5000, 5, 52, 4},
  {239.1429, 6, 61, 4},
  {243.0000, 0, 8, 4},
  {247.5000, 5, 54, 4},
  {248.4000, 4, 45, 4},
  {249.7500, 3, 36, 4},
  {252.0000, 2, 27, 2},
  {253.8000, 4, 46, 2},
  {256.5000, 1, 18, 2},
  {259.2000, 4, 47, 2},
  {261.0000, 2, 28, 2},
  {263.2500, 3, 38, 2},
  {264.6000, 4, 48, 2},
  {265.5000, 5, 58, 2},
  {270.0000, 0, 9, 2},
  {274.5000, 5, 60, 2},
  {275.4000, 4, 50, 2},
  {276.7500, 3, 40, 2},
  {279.0000, 2, 30, 2},
  {280.8000, 4, 51, 2},
  {283.5000, 1, 20, 2},
  {286.2000, 4, 52, 2},
  {288.0000, 2, 31, 2},
  {290.2500, 3, 42, 2},
  {291.6000, 4, 53, 2},
  {297.0000, 0, 10, 2},
  {302.4000, 4, 55, 2},
  {303.7500, 3, 44, 2},
  {306.0000, 2, 33, 2},
  {307.8000, 4, 56, 2},
  {310.5000, 1, 22, 2},
  {313.2000, 4, 57, 2},
  {315.0000, 2, 34, 2},
  {317.2500, 3, 46, 2},
  {318.6000, 4, 58, 2},
  {324.0000, 0, 11, 2},
  {329.4000, 4, 60, 2},
  {330.7500, 3, 48, 2},
  {333.0000, 2, 36, 2},
  {334.8000, 4, 61, 2},
  {337.5000, 1, 24, 2},
  {340.2000, 4, 62, 2},
  {342.0000, 2, 37, 2},
  {344.2500, 3, 50, 2},
  {351.0000, 0, 12, 2},
  {357.7500, 3, 52, 2},
  {360.0000, 2, 39, 2},
  {364.5000, 1, 26, 2},
  {369.0000, 2, 40, 2},
  {371.2500, 3, 54, 2},
  {378.0000, 0, 13, 2},
  {384.7500, 3, 56, 2},
  {387.0000, 2, 42, 2},
  {391.5000, 1, 28, 2},
  {396.0000, 2, 43, 2},
  {398.2500, 3, 58, 2},
  {405.0000, 0, 14, 2},
  {411.7500, 3, 60, 2},
  {414.0000, 2, 45, 2},
  {418.5000, 1, 30, 2},
  {423.0000, 2, 46, 2},
  {425.2500, 3, 62, 2},
  {432.0000, 0, 15, 2},
  {441.0000, 2, 48, 2},
  {445.5000, 1, 32, 2},
  {450.0000, 2, 49, 2},
  {459.0000, 0, 16, 2},
  {468.0000, 2, 51, 2},
  {472.5000, 1, 34, 2},
  {477.0000, 2, 52, 2},
  {486.0000, 0, 17, 2},
  {495.0000, 2, 54, 2},
  {499.5000, 1, 36, 2},
  {504.0000, 2, 55, 2},
  {513.0000, 0, 18, 2},
  {522.0000, 2, 57, 2},
  {526.5000, 1, 38, 2},
  {531.0000, 2, 58, 2},
  {540.0000, 0, 19, 2},
  {549.0000, 2, 60, 2},
  {553.5000, 1, 40, 2},
  {558.0000, 2, 61, 2},
  {567.0000, 0, 20, 2},
  {580.5000, 1, 42, 2},
  {594.0000, 0, 21, 2}
};

static freq_table *tables[] = {freqs_9k, freqs_20k, freqs_both};

void freq_search(const unsigned int board, const double target_freq, double *actual_freq, int *idiv_sel,
		 int *fbdiv_sel, int *odiv_sel)
{
  int i, index_min;
  double diff, min = 99999.0;
  freq_table *table = tables[board];

  for (i = 0; i < sizeof(freqs_both)/sizeof(freqs_both[0]); i++) {
    diff = fabs(target_freq - table[i].freq);
    if (diff < min) {
      min = diff;;
      index_min = i;
    }
  }

  *actual_freq = table[index_min].freq;
  *idiv_sel = table[index_min].idiv_sel;
  *fbdiv_sel = table[index_min].fbdiv_sel;
  *odiv_sel = table[index_min].odiv_sel;

  return;
}

#ifdef STANDALONE

int main(int argc, char **argv)
{
  double freq, target;
  int idiv_sel, fbdiv_sel, odiv_sel;
  unsigned int table_index;

  if (argc != 3) {
    fprintf(stderr, "usage: freq_search target_freq_mhz BOARD_9K|BOARD_20K|BOARD_BOTH\n");
    exit(EXIT_FAILURE);
  }

  target = atof(argv[1]);

  if (strncmp(argv[2], "BOARD_9K", strlen("BOARD_9K")) == 0)
    table_index = BOARD_9K;
  else if (strncmp(argv[2], "BOARD_20K", strlen("BOARD_29K")) == 0)
    table_index = BOARD_20K;
  else if (strncmp(argv[2], "BOARD_BOTH", strlen("BOARD_BOTH")) == 0)
    table_index = BOARD_BOTH;
  else {
    fprintf(stderr, "Error board must be one of BOARD_9K, BOARD_20K, or BOARD_BOTH\n");
    exit(EXIT_FAILURE);
  }
  
  freq_search(table_index, target, &freq, &idiv_sel, &fbdiv_sel, &odiv_sel);
  printf("freq: %8.4lf idev_sel: %d fbdiv_sel: %d, odiv_sel: %d\n",
	 freq, idiv_sel, fbdiv_sel, odiv_sel);
  
  return EXIT_SUCCESS;
}

#endif
