/*
Copyright 2024 VersaSlot Authors, KontonGu (Jianfeng Gu), Hao Wang, et. al.
@Techinical University of Munich, CAPS Cloud Team

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "main.h"
#include "ap_axi_sdata.h"
#include "ap_fixed.h"
#include "ap_int.h"
#include <iostream>

#define IMAGE_MAX_M 1920
#define IMAGE_MAX_N 1080

typedef float DTYPE;
typedef float QTYPE;

#define DZERO 0.00f
#define QZERO 0

void padImage(volatile DTYPE *src, volatile DTYPE *dst, int srcm, int srcn,
              int dstm, int dstn) {
#pragma HLS inline
  bool image;
  for (int i = 0; i < dstn; i++) {
#pragma HLS PIPELINE
    for (int j = 0; j < dstm; j++) {
#pragma HLS PIPELINE
      image = (i < srcn) && (j < srcm);
      if (image)
        dst[i * dstm + j] = src[i * srcm + j];
      else
        dst[i * dstm + j] = 0;
    }
  }
}

void extractBlockD(volatile DTYPE *src, volatile DTYPE *dst, int dstm,
                   int dstn) {
#pragma HLS inline
  DTYPE buffer[8][8];
  int cnt = 0;
  for (int i = 0; i < dstn / 8; i++) {
#pragma HLS PIPELINE
    for (int j = 0; j < dstm / 8; j++) {
      for (int m = 0; m < 8; m++) {
        for (int n = 0; n < 8; n++) {
          buffer[m][n] = src[(8 * i + m) * dstm + (8 * j + n)];
        }
      }
      for (int m = 0; m < 8; m++) {
#pragma HLS PIPELINE
        for (int n = 0; n < 8; n++) {
          dst[cnt] = buffer[m][n];
          cnt += 1;
        }
      }
    }
  }
}

int bundle1(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  padImage(buf1, buf2, srcm, srcn, dstm, dstn);
  extractBlockD(buf2, buf1, dstm, dstn);

  return 0;
}

int matmul_fast(DTYPE srcA[8][8], DTYPE srcB[8][8], DTYPE dst[8][8]) {
#pragma HLS inline
  DTYPE tmp = DZERO;
  for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE off
    for (int j = 0; j < 8; j++) {
#pragma HLS PIPELINE off
      tmp = 0.0;
      for (int k = 0; k < 8; k++) {
        tmp += srcA[i][k] * srcB[k][j];
      }
      dst[i][j] = tmp;
    }
  }
  return 0;
}

int dct(volatile DTYPE *buf1, volatile DTYPE *buf2,
        volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
        volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
        volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
        volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
        volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
        int srcm, int srcn, int dstm, int dstn,
        volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float dctMat[8][8] = {{0.353553, 0.353553, 0.353553, 0.353553, 0.353553,
                         0.353553, 0.353553, 0.353553},
                        {0.490393, 0.41574, 0.277797, 0.097565, -0.0975196,
                         -0.277759, -0.415714, -0.490384},
                        {0.461942, 0.191358, -0.191315, -0.461924, -0.46196,
                         -0.191401, 0.191272, 0.461907},
                        {0.41574, -0.0975196, -0.490384, -0.277836, 0.27772,
                         0.490411, 0.0976559, -0.415662},
                        {0.353562, -0.353529, -0.353594, 0.353496, 0.353627,
                         -0.353463, -0.35366, 0.353431},
                        {0.277797, -0.490384, 0.0974742, 0.415791, -0.415662,
                         -0.0977013, 0.490429, -0.277605},
                        {0.191358, -0.46196, 0.461907, -0.191229, -0.191486,
                         0.462013, -0.461853, 0.191101},
                        {0.097565, -0.277836, 0.415791, -0.49042, 0.490357,
                         -0.415611, 0.277566, -0.097247}};

  float dctMatT[8][8] = {{0.353553, 0.490393, 0.461942, 0.41574, 0.353562,
                          0.277797, 0.191358, 0.097565},
                         {0.353553, 0.41574, 0.191358, -0.0975196, -0.353529,
                          -0.490384, -0.46196, -0.277836},
                         {0.353553, 0.277797, -0.191315, -0.490384, -0.353594,
                          0.0974742, 0.461907, 0.415791},
                         {0.353553, 0.097565, -0.461924, -0.277836, 0.353496,
                          0.415791, -0.191229, -0.49042},
                         {0.353553, -0.0975196, -0.46196, 0.27772, 0.353627,
                          -0.415662, -0.191486, 0.490357},
                         {0.353553, -0.277759, -0.191401, 0.490411, -0.353463,
                          -0.0977013, 0.462013, -0.415611},
                         {0.353553, -0.415714, 0.191272, 0.0976559, -0.35366,
                          0.490429, -0.461853, 0.277566},
                         {0.353553, -0.490384, 0.461907, -0.415662, 0.353431,
                          -0.277605, 0.191101, -0.097247}};

  DTYPE srcBlk[8][8];
  DTYPE tmpBlk[8][8];
  DTYPE dstBlk[8][8];

#pragma HLS ARRAY_PARTITION variable = dctMat cyclic factor = 2 dim = 2
#pragma HLS ARRAY_PARTITION variable = srcBlk cyclic factor = 2 dim = 1
#pragma HLS ARRAY_PARTITION variable = tmpBlk cyclic factor = 2 dim = 1
#pragma HLS ARRAY_PARTITION variable = tmpBlk cyclic factor = 2 dim = 2
#pragma HLS ARRAY_PARTITION variable = dctMatT cyclic factor = 2 dim = 1
#pragma HLS ARRAY_PARTITION variable = dstBlk cyclic factor = 2 dim = 1

  for (int m = 0; m < dstm * dstn / 64; m++) {
#pragma HLS UNROLL
    for (int n = 0; n < 64; n++) {
#pragma HLS UNROLL
      int i = n / 8;
      int j = n % 8;
      srcBlk[i][j] = buf1[m * 64 + n];
    }

    matmul_fast(dctMat, srcBlk, tmpBlk);
    matmul_fast(tmpBlk, dctMatT, dstBlk);

    for (int i = 0; i < 8; i++) {
#pragma HLS UNROLL
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        buf2[m * 64 + i * 8 + j] = dstBlk[i][j];
      }
    }
  }

  return 0;
}

int quantize(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float stdQuantizationMatrixInv[8][8] = {
      {0.063, 0.091, 0.100, 0.063, 0.042, 0.025, 0.020, 0.016},
      {0.083, 0.082, 0.071, 0.053, 0.038, 0.017, 0.017, 0.018},
      {0.071, 0.077, 0.063, 0.042, 0.025, 0.018, 0.014, 0.018},
      {0.071, 0.059, 0.045, 0.034, 0.020, 0.011, 0.013, 0.016},
      {0.056, 0.045, 0.027, 0.018, 0.015, 0.009, 0.010, 0.013},
      {0.042, 0.029, 0.018, 0.016, 0.012, 0.010, 0.009, 0.011},
      {0.020, 0.016, 0.013, 0.011, 0.010, 0.008, 0.008, 0.010},
      {0.014, 0.011, 0.011, 0.010, 0.009, 0.010, 0.010, 0.010}};

  float srcBlk[8][8];
  float dstBlk[8][8];

#pragma HLS ARRAY_PARTITION variable = stdQuantizationMatrixInv type = complete
#pragma HLS ARRAY_PARTITION variable = srcBlk type = complete
#pragma HLS ARRAY_PARTITION variable = dstBlk type = complete

  for (int m = 0; m < dstm * dstn / 64; m++) {
#pragma HLS PIPELINE off
    for (int n = 0; n < 64; n++) {
#pragma HLS UNROLL
      int i = n / 8;
      int j = n % 8;
      srcBlk[i][j] = buf1[m * 64 + n];
    }

    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        float tmpD = srcBlk[i][j] * stdQuantizationMatrixInv[i][j];
        dstBlk[i][j] = tmpD;
      }
    }

    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        buf2[m * 64 + i * 8 + j] = dstBlk[i][j];
      }
    }
  }

  return 0;
}

int dequantize(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float stdQuantizationMatrix[8][8] = {
      {16.00f, 11.00f, 10.00f, 16.00f, 24.00f, 40.00f, 51.00f, 61.00f},
      {12.00f, 12.00f, 14.00f, 19.00f, 26.00f, 58.00f, 60.00f, 55.00f},
      {14.00f, 13.00f, 16.00f, 24.00f, 40.00f, 57.00f, 69.00f, 56.00f},
      {14.00f, 17.00f, 22.00f, 29.00f, 51.00f, 87.00f, 80.00f, 62.00f},
      {18.00f, 22.00f, 37.00f, 56.00f, 68.00f, 109.00f, 103.00f, 77.00f},
      {24.00f, 35.00f, 55.00f, 64.00f, 81.00f, 104.00f, 113.00f, 92.00f},
      {49.00f, 64.00f, 78.00f, 87.00f, 103.00f, 121.00f, 120.00f, 101.00f},
      {72.00f, 92.00f, 95.00f, 98.00f, 112.00f, 100.00f, 103.00f, 99.00f}};

  float srcBlk[8][8];
  float dstBlk[8][8];

#pragma HLS ARRAY_PARTITION variable = stdQuantizationMatrix type = complete
#pragma HLS ARRAY_PARTITION variable = srcBlk type = complete
#pragma HLS ARRAY_PARTITION variable = dstBlk type = complete

  for (int m = 0; m < dstm * dstn / 64; m++) {
#pragma HLS PIPELINE off
    for (int n = 0; n < 64; n++) {
#pragma HLS UNROLL
      int i = n / 8;
      int j = n % 8;
      srcBlk[i][j] = buf1[m * 64 + n];
    }

    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        float tmpD = srcBlk[i][j] * stdQuantizationMatrix[i][j];
        dstBlk[i][j] = tmpD;
      }
    }

    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        buf2[m * 64 + i * 8 + j] = dstBlk[i][j];
      }
    }
  }

  return 0;
}

int idct(volatile DTYPE *buf1, volatile DTYPE *buf2,
         volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
         volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
         volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
         volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
         volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
         int srcm, int srcn, int dstm, int dstn,
         volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float dctMat[8][8] = {{0.353553, 0.353553, 0.353553, 0.353553, 0.353553,
                         0.353553, 0.353553, 0.353553},
                        {0.490393, 0.41574, 0.277797, 0.097565, -0.0975196,
                         -0.277759, -0.415714, -0.490384},
                        {0.461942, 0.191358, -0.191315, -0.461924, -0.46196,
                         -0.191401, 0.191272, 0.461907},
                        {0.41574, -0.0975196, -0.490384, -0.277836, 0.27772,
                         0.490411, 0.0976559, -0.415662},
                        {0.353562, -0.353529, -0.353594, 0.353496, 0.353627,
                         -0.353463, -0.35366, 0.353431},
                        {0.277797, -0.490384, 0.0974742, 0.415791, -0.415662,
                         -0.0977013, 0.490429, -0.277605},
                        {0.191358, -0.46196, 0.461907, -0.191229, -0.191486,
                         0.462013, -0.461853, 0.191101},
                        {0.097565, -0.277836, 0.415791, -0.49042, 0.490357,
                         -0.415611, 0.277566, -0.097247}};

  float dctMatT[8][8] = {{0.353553, 0.490393, 0.461942, 0.41574, 0.353562,
                          0.277797, 0.191358, 0.097565},
                         {0.353553, 0.41574, 0.191358, -0.0975196, -0.353529,
                          -0.490384, -0.46196, -0.277836},
                         {0.353553, 0.277797, -0.191315, -0.490384, -0.353594,
                          0.0974742, 0.461907, 0.415791},
                         {0.353553, 0.097565, -0.461924, -0.277836, 0.353496,
                          0.415791, -0.191229, -0.49042},
                         {0.353553, -0.0975196, -0.46196, 0.27772, 0.353627,
                          -0.415662, -0.191486, 0.490357},
                         {0.353553, -0.277759, -0.191401, 0.490411, -0.353463,
                          -0.0977013, 0.462013, -0.415611},
                         {0.353553, -0.415714, 0.191272, 0.0976559, -0.35366,
                          0.490429, -0.461853, 0.277566},
                         {0.353553, -0.490384, 0.461907, -0.415662, 0.353431,
                          -0.277605, 0.191101, -0.097247}};

  float srcBlk[8][8];
  float tmpBlk[8][8];
  float dstBlk[8][8];

#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = dctMat
#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = srcBlk
#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = tmpBlk
#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = tmpBlk
#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = dctMatT
#pragma HLS ARRAY_PARTITION dim = 2 type = complete variable = dstBlk

  for (int m = 0; m < dstm * dstn / 64; m++) {
#pragma HLS UNROLL
    for (int n = 0; n < 64; n++) {
#pragma HLS UNROLL
      int i = n / 8;
      int j = n % 8;
      srcBlk[i][j] = buf1[m * 64 + n];
    }

    matmul_fast(dctMatT, srcBlk, tmpBlk);
    matmul_fast(tmpBlk, dctMat, dstBlk);

    for (int i = 0; i < 8; i++) {
#pragma HLS UNROLL
      for (int j = 0; j < 8; j++) {
#pragma HLS UNROLL
        buf2[m * 64 + i * 8 + j] = dstBlk[i][j];
      }
    }
  }

  return 0;
}

// Output size = srcm * srcn
int reconstructBlockD(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  DTYPE buffer[64];
#pragma HLS ARRAY_PARTITION dim = 1 type = complete variable = buffer
  DTYPE buffer_resize[8][8];
#pragma HLS ARRAY_PARTITION variable = buffer_resize dim = 1 type = complete

  for (int i = 0; i < dstn; i += 8) {
#pragma HLS UNROLL
    for (int j = 0; j < dstm; j += 8) {
#pragma HLS UNROLL
      int index = 0;
      for (int m = 0; m < 8; m++) {
#pragma HLS UNROLL
        for (int n = 0; n < 8; n++) {
#pragma HLS UNROLL
          if (i + m < dstn && j + n < dstm) {
            buffer[index] = buf1[(i + m) * dstm + (j + n)];
          } else {
            buffer[index] = 0;
          }
          index++;
        }
      }

      int cnt = 0;
      for (int m = 0; m < 8; m++) {
#pragma HLS UNROLL
        for (int n = 0; n < 8; n++) {
#pragma HLS UNROLL
          buffer_resize[m][n] = buffer[cnt];
          cnt++;
        }
      }

      for (int m = 0; m < 8; m++) {
#pragma HLS UNROLL
        for (int n = 0; n < 8; n++) {
#pragma HLS UNROLL
          if (i + m < srcn && j + n < srcm) {
            buf2[(i + m) * srcm + (j + n)] = buffer_resize[m][n];
          }
        }
      }
    }
  }

  return 0;
}

int bundle1_dct_quantize_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {

#pragma HLS DATAFLOW

#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf3 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf4 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf5 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf6 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  bundle1(buf1, buf2, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
          dummy_data5, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
          dummy_data5, srcm, srcn, dstm, dstn, dummy_data);
  dct(buf3, buf4, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
      dummy_data6, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
      dummy_data6, srcm, srcn, dstm, dstn, dummy_data);
  quantize(buf5, buf6, dummy_data7, dummy_data7, dummy_data7, dummy_data7,
           dummy_data7, dummy_data7, dummy_data7, dummy_data7, dummy_data7,
           dummy_data7, srcm, srcn, dstm, dstn, dummy_data);

  return 0;
}

int bundle1_dct_quantize_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf3 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf4 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf5 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf6 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  bundle1(buf1, buf2, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
          dummy_data5, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
          dummy_data5, srcm, srcn, dstm, dstn, dummy_data);
  dct(buf2, buf1, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
      dummy_data6, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
      dummy_data6, srcm, srcn, dstm, dstn, dummy_data);
  quantize(buf1, buf2, dummy_data7, dummy_data7, dummy_data7, dummy_data7,
           dummy_data7, dummy_data7, dummy_data7, dummy_data7, dummy_data7,
           dummy_data7, srcm, srcn, dstm, dstn, dummy_data);

  return 0;
}

int dequantize_idct_recon_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {

#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf3 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf4 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf5 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf6 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  dequantize(buf1, buf2, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
             dummy_data5, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
             dummy_data5, srcm, srcn, dstm, dstn, dummy_data);
  idct(buf3, buf4, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
       dummy_data6, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
       dummy_data6, srcm, srcn, dstm, dstn, dummy_data);
  reconstructBlockD(buf5, buf6, dummy_data7, dummy_data7, dummy_data7,
                    dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                    dummy_data7, dummy_data7, dummy_data7, srcm, srcn, dstm,
                    dstn, dummy_data);

  return 0;
}

int dequantize_idct_recon_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data) {

#pragma HLS INTERFACE m_axi depth = 2073600 port = buf1 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf2 offset =               \
    slave bundle = data1
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf3 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf4 offset =               \
    slave bundle = data2
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf5 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi depth = 2073600 port = buf6 offset =               \
    slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = srcm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = srcn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstm bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dstn bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  dequantize(buf1, buf2, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
             dummy_data5, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
             dummy_data5, srcm, srcn, dstm, dstn, dummy_data);
  idct(buf2, buf1, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
       dummy_data6, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
       dummy_data6, srcm, srcn, dstm, dstn, dummy_data);
  reconstructBlockD(buf1, buf2, dummy_data7, dummy_data7, dummy_data7,
                    dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                    dummy_data7, dummy_data7, dummy_data7, srcm, srcn, dstm,
                    dstn, dummy_data);

  return 0;
}

int conv_I(float input[3][227][227], float conv_core[96][3][11][11],
           float conv_core_weights[96][3][11][11], float conv_core_bias[96],
           float output[96][55][55]) {

  float buff_cov_core[3][11][11];
#pragma HLS ARRAY_PARTITION variable = buff_cov_core dim = 1 factor = 4 type = \
    cyclic
  float buff_cov_image[3][11][11];
#pragma HLS ARRAY_PARTITION variable = buff_cov_image dim = 1 factor =         \
    4 type = cyclic
  float buff_conv_core_weights[3][11][11];
#pragma HLS ARRAY_PARTITION variable = buff_conv_core_weights dim = 1 factor = \
    4 type = cyclic
  float sum;

  for (int co = 0; co < 96; co++) {
#pragma HLS PIPELINE off
    for (int ci = 0; ci < 3; ci++) {
#pragma HLS PIPELINE off
      for (int i = 0; i < 11; i++) {
#pragma HLS PIPELINE off
        for (int j = 0; j < 11; j++) {
#pragma HLS UNROLL
          buff_cov_core[ci][i][j] = conv_core[co][ci][i][j];
          buff_conv_core_weights[ci][i][j] = conv_core_weights[co][ci][i][j];
        }
      }
    }
    for (int h = 0; h < 55; h++) {
#pragma HLS PIPELINE off
      for (int w = 0; w < 55; w++) {
#pragma HLS PIPELINE off
        for (int ci = 0; ci < 3; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 11; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 11; j++) {
#pragma HLS UNROLL
              buff_cov_image[ci][i][j] = input[ci][4 * h + i][4 * w + j];
            }
          }
        }
        sum = 0.00f;
        for (int ci = 0; ci < 3; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 11; i++) {
#pragma HLS UNROLL
            for (int j = 0; j < 11; j++) {
#pragma HLS UNROLL
              sum += buff_cov_image[ci][i][j] * buff_cov_core[ci][i][j] *
                     buff_conv_core_weights[ci][i][j];
            }
          }
        }
        sum = std::max(sum + conv_core_bias[co], 0.0f);
        output[co][h][w] = sum;
      }
    }
  }
  return 0;
}

int MaxPool_I(float input[96][55][55], float output[96][27][27]) {

  float max_val;

  for (int co = 0; co < 96; co++) {
#pragma HLS PIPELINE off
    for (int h = 0; h < 27; h++) {
#pragma HLS PIPELINE off
      for (int w = 0; w < 27; w++) {
#pragma HLS PIPELINE off
        for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
          for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
            max_val = std::max(max_val, input[co][h * 2 + i][w * 2 + j]);
          }
        }
        output[co][h][w] = max_val;
      }
    }
  }
  return 0;
}

void pad_image_I(float input[96][27][27], float output[96][31][31]) {
#pragma HLS inline
  for (int m = 0; m < 96; m++) {
#pragma HLS PIPELINE off
    for (int i = 0; i < 31; ++i) {
#pragma HLS PIPELINE off
      for (int j = 0; j < 31; ++j) {
        output[m][i][j] = 0.0f;
      }
    }

    for (int i = 0; i < 27; ++i) {
#pragma HLS PIPELINE off
      for (int j = 0; j < 27; ++j) {
        output[m][i + 2][j + 2] = input[m][i][j];
      }
    }
  }
}

int conv_II(float input[96][27][27], float conv_core[256][96][5][5],
            float conv_core_weights[256][96][5][5], float conv_core_bias[256],
            float output[256][27][27]) {

  float buff_cov_core[96][5][5];
#pragma HLS ARRAY_PARTITION variable = buff_cov_core dim = 1 factor = 4 type = \
    cyclic
  float buff_cov_image[96][5][5];
#pragma HLS ARRAY_PARTITION variable = buff_cov_image dim = 1 factor =         \
    4 type = cyclic
  float buff_conv_core_weights[96][5][5];
#pragma HLS ARRAY_PARTITION variable = buff_conv_core_weights dim = 1 factor = \
    4 type = cyclic
  float sum;

  float input_padding[96][27 + 4][27 + 4];
#pragma HLS ARRAY_PARTITION variable = input_padding dim = 1 factor = 4 type = \
    cyclic

  for (int co = 0; co < 256; co++) {
#pragma HLS PIPELINE off
    pad_image_I(input, input_padding);
    for (int ci = 0; ci < 96; ci++) {
#pragma HLS PIPELINE off
      for (int i = 0; i < 5; i++) {
#pragma HLS PIPELINE off
        for (int j = 0; j < 5; j++) {
#pragma HLS PIPELINE off
          buff_cov_core[ci][i][j] = conv_core[co][ci][i][j];
          buff_conv_core_weights[ci][i][j] = conv_core_weights[co][ci][i][j];
        }
      }
    }
    for (int h = 0; h < 27; h++) { // Padding = 4
#pragma HLS PIPELINE off
      for (int w = 0; w < 27; w++) { // Padding = 4
#pragma HLS PIPELINE off
        for (int ci = 0; ci < 96; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 5; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 5; j++) {
#pragma HLS PIPELINE off
              buff_cov_image[ci][i][j] =
                  input_padding[ci][1 * h + i][1 * w + j];
            }
          }
        }
        sum = 0.00f;
        for (int ci = 0; ci < 96; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 5; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 5; j++) {
#pragma HLS PIPELINE off
              sum += buff_cov_image[ci][i][j] * buff_cov_core[ci][i][j] *
                     buff_conv_core_weights[ci][i][j];
            }
          }
        }
        sum = std::max(sum + conv_core_bias[co], 0.0f);
        output[co][h][w] = sum;
      }
    }
  }
  return 0;
}

int MaxPool_II(float input[256][27][27], float output[256][13][13]) {

  float max_val;

  for (int co = 0; co < 256; co++) {
#pragma HLS PIPELINE off
    for (int h = 0; h < 13; h++) {
#pragma HLS PIPELINE off
      for (int w = 0; w < 13; w++) {
#pragma HLS PIPELINE off
        for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
          for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
            max_val = std::max(max_val, input[co][h * 2 + i][w * 2 + j]);
          }
        }
        output[co][h][w] = max_val;
      }
    }
  }
  return 0;
}

void pad_image_II(float input[256][13][13], float output[256][15][15]) {
#pragma HLS inline
  for (int m = 0; m < 256; m++) {
#pragma HLS PIPELINE off
    for (int i = 0; i < 15; ++i) {
#pragma HLS PIPELINE off
      for (int j = 0; j < 15; ++j) {
#pragma HLS UNROLL
        output[m][i][j] = 0.0f;
      }
    }

    for (int i = 0; i < 13; ++i) {
#pragma HLS PIPELINE off
      for (int j = 0; j < 13; ++j) {
#pragma HLS UNROLL
        output[m][i + 1][j + 1] = input[m][i][j];
      }
    }
  }
}

int conv_III(
    float input[256][13][13], float conv_core[384][256][3][3],
    float conv_core_weights[384][256][3][3], float conv_core_bias[384],
    float output[384][13][13], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
    volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 43264 port = input offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core offset =           \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core_weights offset =   \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 384 port = conv_core_bias offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 64896 port = output offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float buff_cov_core[256][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_cov_core dim = 1 factor = 4 type = \
    cyclic
  float buff_cov_image[256][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_cov_image dim = 1 factor =         \
    4 type = cyclic
  float buff_conv_core_weights[256][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_conv_core_weights dim = 1 factor = \
    4 type = cyclic
  float sum;

  float input_padding[256][13 + 2][13 + 2];
#pragma HLS ARRAY_PARTITION variable = input_padding dim = 1 factor = 4 type = \
    cyclic

  for (int co = 0; co < 384; co++) {
#pragma HLS PIPELINE off
    pad_image_II(input, input_padding);
    for (int ci = 0; ci < 256; ci++) {
#pragma HLS PIPELINE off
      for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
        for (int j = 0; j < 3; j++) {
#pragma HLS PIPELINE off
          buff_cov_core[ci][i][j] = conv_core[co][ci][i][j];
          buff_conv_core_weights[ci][i][j] = conv_core_weights[co][ci][i][j];
        }
      }
    }
    for (int h = 0; h < 13; h++) { // Padding = 4
#pragma HLS PIPELINE off
      for (int w = 0; w < 13; w++) { // Padding = 4
#pragma HLS PIPELINE off
        for (int ci = 0; ci < 256; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS PIPELINE off
              buff_cov_image[ci][i][j] =
                  input_padding[ci][1 * h + i][1 * w + j];
            }
          }
        }
        sum = 0.00f;
        for (int ci = 0; ci < 256; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS PIPELINE off
              sum += buff_cov_image[ci][i][j] * buff_cov_core[ci][i][j] *
                     buff_conv_core_weights[ci][i][j];
            }
          }
        }
        sum = std::max(sum + conv_core_bias[co], 0.0f);
        output[co][h][w] = sum;
      }
    }
  }
  return 0;
}

void pad_image_III(float input[384][13][13], float output[384][15][15]) {
#pragma HLS inline
  for (int m = 0; m < 384; m++) {
#pragma HLS PIPELINE off
    for (int i = 0; i < 15; ++i) {
      for (int j = 0; j < 15; ++j) {
        output[m][i][j] = 0.0f;
      }
    }

    for (int i = 0; i < 13; ++i) {
      for (int j = 0; j < 13; ++j) {
        output[m][i + 1][j + 1] = input[m][i][j];
      }
    }
  }
}

int conv_IV(float input[384][13][13], float conv_core[384][384][3][3],
            float conv_core_weights[384][384][3][3], float conv_core_bias[384],
            float output[384][13][13], volatile ap_uint<32> dummy_data11,
            volatile ap_uint<32> dummy_data22,
            volatile ap_uint<32> dummy_data33,
            volatile ap_uint<32> dummy_data44,
            volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 64896 port = input offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 1327104 port = conv_core offset =          \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 1327104 port = conv_core_weights offset =  \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 384 port = conv_core_bias offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 64896 port = output offset =               \
    slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  float buff_cov_core[384][3][3];
#pragma HLS ARRAY_PARTITION dim = 1 factor = 4 type = cyclic variable =        \
    buff_cov_core
  float buff_cov_image[384][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_cov_image dim = 1 factor =         \
    4 type = cyclic
  float buff_conv_core_weights[384][3][3];
#pragma HLS ARRAY_PARTITION dim = 1 factor = 4 type = cyclic variable =        \
    buff_conv_core_weights

  float sum;

  float input_padding[384][13 + 2][13 + 2];

  for (int co = 0; co < 384; co++) {
#pragma HLS PIPELINE off
    pad_image_III(input, input_padding);
    for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
      for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
        for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
          buff_cov_core[ci][i][j] = conv_core[co][ci][i][j];
          buff_conv_core_weights[ci][i][j] = conv_core_weights[co][ci][i][j];
        }
      }
    }
    for (int h = 0; h < 13; h++) { // Padding = 4
#pragma HLS PIPELINE off
      for (int w = 0; w < 13; w++) { // Padding = 4
#pragma HLS PIPELINE off
        for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
              buff_cov_image[ci][i][j] =
                  input_padding[ci][1 * h + i][1 * w + j];
            }
          }
        }
        sum = 0.00f;
        for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
              sum += buff_cov_image[ci][i][j] * buff_cov_core[ci][i][j] *
                     buff_conv_core_weights[ci][i][j];
            }
          }
        }
        sum = std::max(sum + conv_core_bias[co], 0.0f);
        output[co][h][w] = sum;
      }
    }
  }
  return 0;
}

int conv_V(float input[384][13][13], float conv_core[256][384][3][3],
           float conv_core_weights[256][384][3][3], float conv_core_bias[256],
           float output[256][13][13]) {

  float buff_cov_core[384][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_cov_core dim = 1 factor = 4 type = \
    cyclic
  float buff_cov_image[384][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_cov_image dim = 1 factor =         \
    4 type = cyclic
  float buff_conv_core_weights[384][3][3];
#pragma HLS ARRAY_PARTITION variable = buff_conv_core_weights dim = 1 factor = \
    4 type = cyclic

  float sum;

  float input_padding[384][13 + 2][13 + 2];
#pragma HLS ARRAY_PARTITION variable = input_padding dim = 1 factor = 4 type = \
    cyclic

  for (int co = 0; co < 256; co++) {
#pragma HLS PIPELINE off
    pad_image_III(input, input_padding);
    for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
      for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
        for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
          buff_cov_core[ci][i][j] = conv_core[co][ci][i][j];
          buff_conv_core_weights[ci][i][j] = conv_core_weights[co][ci][i][j];
        }
      }
    }
    for (int h = 0; h < 13; h++) { // Padding = 4
#pragma HLS PIPELINE off
      for (int w = 0; w < 13; w++) { // Padding = 4
#pragma HLS PIPELINE off
        for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
              buff_cov_image[ci][i][j] =
                  input_padding[ci][1 * h + i][1 * w + j];
            }
          }
        }
        sum = 0.00f;
        for (int ci = 0; ci < 384; ci++) {
#pragma HLS PIPELINE off
          for (int i = 0; i < 3; i++) {
#pragma HLS PIPELINE off
            for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
              sum += buff_cov_image[ci][i][j] * buff_cov_core[ci][i][j] *
                     buff_conv_core_weights[ci][i][j];
            }
          }
        }
        sum = std::max(sum + conv_core_bias[co], 0.0f);
        output[co][h][w] = sum;
      }
    }
  }
  return 0;
}

int MaxPool_III(float input[256][13][13], float output[256][6][6]) {

  float max_val;

  for (int co = 0; co < 256; co++) {
#pragma HLS PIPELINE off
    for (int h = 0; h < 6; h++) {
#pragma HLS PIPELINE off
      for (int w = 0; w < 6; w++) {
#pragma HLS UNROLL
        for (int i = 0; i < 3; i++) {
#pragma HLS UNROLL
          for (int j = 0; j < 3; j++) {
#pragma HLS UNROLL
            max_val = std::max(max_val, input[co][h * 2 + i][w * 2 + j]);
          }
        }
        output[co][h][w] = max_val;
      }
    }
  }
  return 0;
}

int full_connection_I(float input[256][6][6], float weights[4096][256 * 6 * 6],
                      float bias[4096], float output[4096]) {

  float flattened_input[256 * 6 * 6];
#pragma HLS ARRAY_PARTITION dim = 1 factor = 256 type = cyclic variable =      \
    flattened_input

  int index = 0;
  for (int c = 0; c < 256; ++c) {
#pragma HLS PIPELINE
    for (int i = 0; i < 6; ++i) {
#pragma HLS UNROLL
      for (int j = 0; j < 6; ++j) {
#pragma HLS UNROLL
        flattened_input[index++] = input[c][i][j];
      }
    }
  }

  for (int i = 0; i < 4096; ++i) {
    float sum = 0.0f;
    for (int j = 0; j < 256 * 6 * 6; ++j) {
      sum += weights[i][j] * flattened_input[j];
    }
    output[i] = std::max(sum + bias[i], 0.0f);
  }

  return 0;
}

// NO DROPOUT

int full_connection_II(float input[4096], float weights[4096][4096],
                       float bias[4096], float output[4096]) {

  for (int i = 0; i < 4096; ++i) {
    float sum = 0.0f;
    for (int j = 0; j < 4096; ++j) {
#pragma HLS PIPELINE
      sum += weights[i][j] * input[j];
    }
    output[i] = std::max(sum + bias[i], 0.0f);
  }

  return 0;
}

// NO DROPOUT

int full_connection_III(float input[4096], float weights[1000][4096],
                        float bias[1000], float output[1000]) {

  for (int i = 0; i < 1000; ++i) {
    float sum = 0.0f;
    for (int j = 0; j < 4096; ++j) {
#pragma HLS PIPELINE
      sum += weights[i][j] * input[j];
    }
    output[i] = std::max(sum + bias[i], 0.0f);
  }

  return 0;
}

// bundle:

int conv_pool_I(
    float input[3][227][227], float conv_core[96][3][11][11],
    float conv_core_weights[96][3][11][11], float conv_core_bias[96],
    float output1[96][55][55], float output2[96][27][27],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 154587 port = input offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 34848 port = conv_core offset =            \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 34848 port = conv_core_weights offset =    \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 96 port = conv_core_bias offset =          \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 290400 port = output1 offset =             \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 69984 port = output2 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  conv_I(input, conv_core, conv_core_weights, conv_core_bias, output1);
  MaxPool_I(output1, output2);

  return 0;
}

int conv_pool_II(
    float input[96][27][27], float conv_core[256][96][5][5],
    float conv_core_weights[256][96][5][5], float conv_core_bias[256],
    float output1[256][27][27], float output2[256][13][13],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 69984 port = input offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 614400 port = conv_core offset =           \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 614400 port = conv_core_weights offset =   \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 256 port = conv_core_bias offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 186624 port = output1 offset =             \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 43264 port = output2 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  conv_II(input, conv_core, conv_core_weights, conv_core_bias, output1);
  MaxPool_II(output1, output2);

  return 0;
}

int conv_pool_V(float input[384][13][13], float conv_core[256][384][3][3],
                float conv_core_weights[256][384][3][3],
                float conv_core_bias[256], float output1[256][13][13],
                float output2[256][6][6], volatile ap_uint<32> dummy_data11,
                volatile ap_uint<32> dummy_data22,
                volatile ap_uint<32> dummy_data33,
                volatile ap_uint<32> dummy_data44,
                volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 64896 port = input offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core offset =           \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core_weights offset =   \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 256 port = conv_core_bias offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 43264 port = output1 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 9216 port = output2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  conv_V(input, conv_core, conv_core_weights, conv_core_bias, output1);
  MaxPool_III(output1, output2);

  return 0;
}

int fc_bundle(float input[256][6][6], float weights_I[4096][256 * 6 * 6],
              float bias_I[4096], float weights_II[4096][4096],
              float bias_II[4096], float weights_III[1000][4096],
              float bias_III[1000], float output_I[4096], float output_II[4096],
              float output_III[1000], volatile ap_uint<32> dummy_data11,
              volatile ap_uint<32> dummy_data22,
              volatile ap_uint<32> dummy_data33,
              volatile ap_uint<32> dummy_data44,
              volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 9216 port = input offset = slave bundle =  \
    data
#pragma HLS INTERFACE m_axi depth = 37748736 port = weights_I offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096 port = bias_I offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 16777216 port = weights_II offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096 port = bias_II offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096000 port = weights_III offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 1000 port = bias_III offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 1000 port = output_III offset =            \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096 port = output_II offset =             \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096 port = output_I offset =              \
    slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  full_connection_I(input, weights_I, bias_I, output_I);
  full_connection_II(output_I, weights_II, bias_II, output_II);
  full_connection_III(output_II, weights_III, bias_III, output_III);

  return 0;
}

// Bundle Alex Net
int AN_CPN123_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 从 buf1 中解析出 input1
  float(*input1)[227][227] =
      (float(*)[227][227])(buf1); // buf1 第 0 个位置开始存储 input1
  float(*conv_core_I)[3][11][11] =
      (float(*)[3][11][11])(buf1 + (3 * 227 * 227)); // 偏移到 input1 之后
  float *conv_core_bias_I =
      (float *)(buf1 + (3 * 227 * 227) +
                (96 * 3 * 11 * 11)); // 偏移到 conv_core_I 之后
  float(*buffer1)[55][55] =
      (float(*)[55][55])(buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) +
                         96); // 偏移到 conv_core_bias_I 之后

  // 从 buf2 中解析出 output1
  float(*output1)[27][27] =
      (float(*)[27][27])(buf2); // buf2 第 0 个位置开始存储 output1

  // 调用第一个子函数
  conv_pool_I(input1, conv_core_I, conv_core_I, conv_core_bias_I, buffer1,
              output1, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
              dummy_data5, dummy_data5, dummy_data11, dummy_data22,
              dummy_data33, dummy_data44, dummy_data55);

  //     // 从 buf3 中解析出 output1 和 conv_core_II
  // float (*output1_II)[27][27] = (float (*)[27][27])(buf3);  // buf3 第 0
  // 个位置是 output1 float (*conv_core_II)[96][5][5] = (float
  // (*)[96][5][5])(buf3 + (256 * 27 * 27));  // 偏移到 output1 之后 float
  // *conv_core_bias_II = (float *)(buf3 + (256 * 27 * 27) + (256 * 96 * 5 *
  // 5));  // 偏移到 conv_core_II 之后

  // // 从 buf4 中解析出 output2
  // float (*output2)[13][13] = (float (*)[13][13])(buf4);  // buf4 第 0
  // 个位置是 output2

  // // 调用第二个子函数
  // conv_pool_II(output1_II, conv_core_II, conv_core_II, conv_core_bias_II,
  // output1_II, output2,
  //              dummy_data6, dummy_data6, dummy_data6, dummy_data6,
  //              dummy_data6, dummy_data6, dummy_data11, dummy_data22,
  //              dummy_data33, dummy_data44, dummy_data55);

  // // 从 buf5 中解析出 output2, conv_core_III 和 conv_core_weights
  // float (*output2_III)[13][13] = (float (*)[13][13])(buf5);  // buf5 第 0
  // 个位置是 output2 float (*conv_core_III)[256][3][3] = (float
  // (*)[256][3][3])(buf5 + (256 * 13 * 13));  // 偏移到 output2 之后 float
  // (*conv_core_weights_III)[256][3][3] = (float (*)[256][3][3])(buf5 + (256 *
  // 13 * 13) + (384 * 256 * 3 * 3));  // 偏移到 conv_core_III 之后 float
  // *conv_core_bias_III = (float *)(buf5 + (256 * 13 * 13) + (384 * 256 * 3 *
  // 3) + (384 * 256 * 3 * 3));  // 偏移到 conv_core_weights_III 之后

  // // 从 buf6 中解析出 output3
  // float (*output3)[13][13] = (float (*)[13][13])(buf6);  // buf6 第 0
  // 个位置是 output3

  // // 调用第三个子函数
  // conv_III(output2_III, conv_core_III, conv_core_weights_III,
  // conv_core_bias_III, output3,
  //         dummy_data7, dummy_data7,dummy_data7, dummy_data7, dummy_data7,
  //         dummy_data7, dummy_data7, dummy_data7, dummy_data22, dummy_data33,
  //         dummy_data44, dummy_data55);

  return 0;
}

int AN_CPN123_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl
  // 解析 buf1 中的所有输入参数
  float(*input1)[227][227] =
      (float(*)[227][227])(buf1); // buf1 第 0 个位置存储 input1
  float(*conv_core_I)[3][11][11] =
      (float(*)[3][11][11])(buf1 + (3 * 227 * 227)); // 偏移到 input1 之后
  float *conv_core_bias_I =
      (float *)(buf1 + (3 * 227 * 227) +
                (96 * 3 * 11 * 11)); // 偏移到 conv_core_I 之后

  float(*conv_core_II)[96][5][5] =
      (float(*)[96][5][5])(buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) +
                           96); // 偏移到 conv_core_bias_I 之后
  float *conv_core_bias_II =
      (float *)(buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) + 96 +
                (256 * 96 * 5 * 5)); // 偏移到 conv_core_II 之后

  float(*conv_core_III)[256][3][3] = (float(*)[256][3][3])(
      buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) + 96 + (256 * 96 * 5 * 5) +
      256); // 偏移到 conv_core_bias_II 之后
  float(*conv_core_weights)[256][3][3] = (float(*)[256][3][3])(
      buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) + 96 + (256 * 96 * 5 * 5) +
      256 + (384 * 256 * 3 * 3)); // 偏移到 conv_core_III 之后
  float *conv_core_bias_III =
      (float *)(buf1 + (3 * 227 * 227) + (96 * 3 * 11 * 11) + 96 +
                (256 * 96 * 5 * 5) + 256 + (384 * 256 * 3 * 3) +
                (384 * 256 * 3 * 3)); // 偏移到 conv_core_weights 之后

  // 解析 buf2 中的所有输出参数
  float(*output1)[27][27] =
      (float(*)[27][27])(buf2); // buf2 第 0 个位置存储 output1
  float(*output2)[13][13] =
      (float(*)[13][13])(buf2 + (256 * 27 * 27)); // 偏移到 output1 之后
  float(*buffer1)[55][55] = (float(*)[55][55])(
      buf2 + (256 * 27 * 27) + (256 * 13 * 13)); // 偏移到 output2 之后
  float(*output3)[13][13] =
      (float(*)[13][13])(buf2 + (256 * 27 * 27) + (256 * 13 * 13) +
                         (256 * 55 * 55)); // 偏移到 buffer1 之后

  // 调用第一个子函数
  conv_pool_I(input1, conv_core_I, conv_core_I, conv_core_bias_I, buffer1,
              output1, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
              dummy_data5, dummy_data5, dummy_data11, dummy_data22,
              dummy_data33, dummy_data44, dummy_data55);

  // 调用第二个子函数
  conv_pool_II(output1, conv_core_II, conv_core_II, conv_core_bias_II, output1,
               output2, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
               dummy_data5, dummy_data5, dummy_data11, dummy_data22,
               dummy_data33, dummy_data44, dummy_data55);

  // 调用第三个子函数
  conv_III(output2, conv_core_III, conv_core_weights, conv_core_bias_III,
           output3, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
           dummy_data5, dummy_data5, dummy_data5, dummy_data11, dummy_data22,
           dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int AN_CPN123(float input1[3][227][227], float conv_core_I[96][3][11][11],
              float conv_core_III[384][256][3][3], float conv_core_bias_I[96],
              float conv_core_II[256][96][5][5], float conv_core_bias_III[384],
              float conv_core_bias_II[256], float output1[256][27][27],
              float output2[256][13][13], float buffer1[256][55][55],
              float output3[384][13][13], volatile ap_uint<64> *dummy_data1,
              volatile ap_uint<32> dummy_data11,
              volatile ap_uint<32> dummy_data22,
              volatile ap_uint<32> dummy_data33,
              volatile ap_uint<32> dummy_data44,
              volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 154587 port = input1 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 34848 port = conv_core_I offset =          \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core_III offset =       \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 96 port = conv_core_bias_I offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 614400 port = conv_core_II offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 384 port = conv_core_bias_III offset =     \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 256 port = conv_core_bias_II offset =      \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 186624 port = output1 offset =             \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 43264 port = output2 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 290400 port = buffer1 offset =             \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 64896 port = output3 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // Call conv_pool_I
  conv_pool_I(input1, conv_core_I, conv_core_I, conv_core_bias_I, buffer1,
              output1, dummy_data1, dummy_data1, dummy_data1, dummy_data1,
              dummy_data1, dummy_data1, dummy_data11, dummy_data22,
              dummy_data33, dummy_data44, dummy_data55);

  // Call conv_pool_II
  conv_pool_II(output1, conv_core_II, conv_core_II, conv_core_bias_II, output1,
               output2, dummy_data1, dummy_data1, dummy_data1, dummy_data1,
               dummy_data1, dummy_data1, dummy_data11, dummy_data22,
               dummy_data33, dummy_data44, dummy_data55);

  // Call conv_III
  conv_III(output2, conv_core_III, conv_core_III, conv_core_bias_III, output3,
           dummy_data1, dummy_data1, dummy_data1, dummy_data1, dummy_data1,
           dummy_data1, dummy_data1, dummy_data11, dummy_data22, dummy_data33,
           dummy_data44, dummy_data55);

  return 0;
}

int AN_CPN45FCB_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析 buf1: conv_IV 所需的输入和参数
  float(*input_IV)[13][13] = (float(*)[13][13])(buf1);
  float(*conv_core_IV)[384][3][3] = (float(*)[384][3][3])(buf1 + 384 * 13 * 13);
  float(*conv_core_weights_IV)[384][3][3] =
      (float(*)[384][3][3])(buf1 + 384 * 13 * 13 + 384 * 384 * 3 * 3);
  float *conv_core_bias_IV =
      (float *)(buf1 + 384 * 13 * 13 + 2 * (384 * 384 * 3 * 3));
  float(*output_IV)[13][13] = (float(*)[13][13])(buf2);

  // // 解析 buf3: conv_pool_V 所需的输入和参数
  // float (*conv_core_V)[384][3][3] = (float (*)[384][3][3])(buf3);
  // float (*conv_core_weights_V)[384][3][3] = (float (*)[384][3][3])(buf3 + 256
  // * 13 * 13); float *conv_core_bias_V = (float *)(buf3 + 2 * (256 * 13 *
  // 13)); float (*output_V1)[13][13] = (float (*)[13][13])(buf4);  // output1
  // float (*output_V2)[6][6] = (float (*)[6][6])(buf4 + 256 * 13 * 13);  //
  // output2

  // // 解析 buf5 和 buf6: fc_bundle 所需的输入和参数
  // float (*weights_I)[256 * 6 * 6] = (float (*)[256 * 6 * 6])(buf5);
  // float *bias_I = (float *)(buf5 + 4096 * 256 * 6 * 6);
  // float (*weights_II)[4096] = (float (*)[4096])(buf6);
  // float *bias_II = (float *)(buf6 + 4096 * 4096);
  // float (*weights_III)[4096] = (float (*)[4096])(buf6 + 2 * (4096 * 4096));
  // float *bias_III = (float *)(buf6 + 2 * (4096 * 4096) + 1000 * 4096);

  // float (*output_fc_I) = (float *)(buf6 + 2 * (4096 * 4096) + 1000 * 4096 +
  // 4096);  // output_I float (*output_fc_II) = (float *)(buf6 + 2 * (4096 *
  // 4096) + 1000 * 4096 + 4096 + 4096);  // output_II float (*output_fc_III) =
  // (float *)(buf6 + 2 * (4096 * 4096) + 1000 * 4096 + 4096 + 4096 + 1000);  //
  // output_III

  // 调用 conv_IV
  conv_IV(input_IV, conv_core_IV, conv_core_weights_IV, conv_core_bias_IV,
          output_IV, dummy_data11, dummy_data22, dummy_data33, dummy_data44,
          dummy_data55);

  // // 调用 conv_pool_V
  // conv_pool_V(output_IV, conv_core_V, conv_core_weights_V, conv_core_bias_V,
  // output_V1, output_V2,
  //             dummy_data11, dummy_data22, dummy_data33, dummy_data44,
  //             dummy_data55);

  // // 调用 fc_bundle
  // fc_bundle(output_V2, weights_I, bias_I, weights_II, bias_II, weights_III,
  // bias_III,
  //           output_fc_I, output_fc_II, output_fc_III,
  //           dummy_data11, dummy_data22, dummy_data33, dummy_data44,
  //           dummy_data55);

  return 0;
}

int AN_CPN45FCB_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl
  // 从 buf1 中解析出 buffer1, conv_core_IV, conv_core_V, weights_I, weights_II,
  // weights_III
  float(*buffer1)[13][13] = (float(*)[13][13])(buf1);
  float(*conv_core_IV)[384][3][3] =
      (float(*)[384][3][3])(buf1 + (384 * 13 * 13));
  float *conv_core_bias_III =
      (float *)(buf1 + (384 * 13 * 13) + (384 * 384 * 3 * 3));
  float(*conv_core_V)[384][3][3] =
      (float(*)[384][3][3])(buf1 + (384 * 13 * 13) + (384 * 384 * 3 * 3) + 384);
  float(*weights_I)[256 * 6 * 6] = (float(*)[256 * 6 * 6])(
      buf1 + (384 * 13 * 13) + (384 * 384 * 3 * 3) + (256 * 6 * 6));
  float(*weights_II)[4096] =
      (float(*)[4096])(buf1 + (384 * 13 * 13) + (384 * 384 * 3 * 3) +
                       (256 * 6 * 6) + (4096 * 256 * 6 * 6));
  float(*weights_III)[4096] =
      (float(*)[4096])(buf1 + (384 * 13 * 13) + (384 * 384 * 3 * 3) +
                       (256 * 6 * 6) + (4096 * 256 * 6 * 6) + (16777216));

  // 从 buf2 中解析出 buffer2, buffer3, bias_I, bias_III
  float(*buffer2)[6][6] = (float(*)[6][6])(buf2);
  float(*buffer3)[13][13] = (float(*)[13][13])(buf2 + (256 * 6 * 6));
  float *bias_I = (float *)(buf2 + (256 * 6 * 6) + (256 * 13 * 13));
  float *bias_III = (float *)(buf2 + (256 * 6 * 6) + (256 * 13 * 13) + 4096);

  // 调用 conv_IV
  conv_IV(buffer1, conv_core_IV, conv_core_IV, conv_core_bias_III, buffer1,
          dummy_data11, dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用 conv_V
  conv_pool_V(buffer1, conv_core_V, conv_core_V, conv_core_bias_III, buffer3,
              buffer2, dummy_data11, dummy_data22, dummy_data33, dummy_data44,
              dummy_data55);

  // 调用 fc_bundle
  fc_bundle(buffer2, weights_I, bias_I, weights_II, bias_I, weights_III,
            bias_III, bias_I, bias_I, bias_III, dummy_data11, dummy_data22,
            dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int AN_CPN45FCB(float buffer1[384][13][13], float conv_core_IV[384][384][3][3],
                float conv_core_bias_III[384],
                float conv_core_V[256][384][3][3], float buffer2[256][6][6],
                float buffer3[256][13][13], float weights_I[4096][256 * 6 * 6],
                float bias_I[4096], float weights_II[4096][4096],
                float weights_III[1000][4096], float bias_III[1000],
                volatile ap_uint<64> *dummy_data1,
                volatile ap_uint<32> dummy_data11,
                volatile ap_uint<32> dummy_data22,
                volatile ap_uint<32> dummy_data33,
                volatile ap_uint<32> dummy_data44,
                volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi depth = 64896 port = buffer1 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 1327104 port = conv_core_IV offset =       \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 384 port = conv_core_bias_III offset =     \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 884736 port = conv_core_V offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 9216 port = buffer2 offset =               \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 43264 port = buffer3 offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 37748736 port = weights_I offset =         \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096 port = bias_I offset = slave bundle = \
    data
#pragma HLS INTERFACE m_axi depth = 16777216 port = weights_II offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 4096000 port = weights_III offset =        \
    slave bundle = data
#pragma HLS INTERFACE m_axi depth = 1000 port = bias_III offset =              \
    slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // Call conv_IV
  conv_IV(buffer1, conv_core_IV, conv_core_IV, conv_core_bias_III, buffer1,
          dummy_data11, dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // Call conv_V
  conv_pool_V(buffer1, conv_core_V, conv_core_V, conv_core_bias_III, buffer3,
              buffer2, dummy_data11, dummy_data22, dummy_data33, dummy_data44,
              dummy_data55);

  // Call fc_bundle
  fc_bundle(buffer2, weights_I, bias_I, weights_II, bias_I, weights_III,
            bias_III, bias_I, bias_I, bias_III, dummy_data11, dummy_data22,
            dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

// 定义优化后的定点类型
typedef ap_fixed<16, 6> optimized_pixel_t;

int read_input_module(
    frames_t frames[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame1_a[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame2_a[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame3_a[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame3_b[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame4_a[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame5_a[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<32> dummy_data11,
    volatile ap_uint<32> dummy_data22, volatile ap_uint<32> dummy_data33,
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = frames offset = slave bundle = data
#pragma HLS interface m_axi port = frame1_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame2_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame3_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame3_b offset = slave bundle = data
#pragma HLS interface m_axi port = frame4_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame5_a offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  static frames_t buf;
  for (int r = 0; r < MAX_HEIGHT; r++) {
    for (int c = 0; c < MAX_WIDTH; c++) {
      buf = frames[r][c];
      frame1_a[r][c] = (optimized_pixel_t)(buf(7, 0));
      frame2_a[r][c] = (optimized_pixel_t)(buf(15, 8));
      frame3_a[r][c] = (optimized_pixel_t)(buf(23, 16));
      frame3_b[r][c] = (optimized_pixel_t)(buf(23, 16));
      frame4_a[r][c] = (optimized_pixel_t)(buf(31, 24));
      frame5_a[r][c] = (optimized_pixel_t)(buf(31, 24));
    }
  }
  return 0;
}

int gradient_xy_calc_module(
    optimized_pixel_t frame[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<32> dummy_data11,
    volatile ap_uint<32> dummy_data22, volatile ap_uint<32> dummy_data33,
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = frame offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_x offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_y offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  static optimized_pixel_t buf[5][MAX_WIDTH];
#pragma HLS array_partition variable = buf complete dim = 1

  optimized_pixel_t smallbuf[5];
#pragma HLS array_partition variable = smallbuf complete dim = 0

  const int GRAD_WEIGHTS[] = {1, -8, 0, 8, -1};

GRAD_XY_OUTER:
  for (int r = 0; r < MAX_HEIGHT + 2; r++) {
  GRAD_XY_INNER:
    for (int c = 0; c < MAX_WIDTH + 2; c++) {
#pragma HLS pipeline II = 2

      // 缓存值，减少重复计算
      for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
        smallbuf[i] = buf[i + 1][c];
      }

      smallbuf[4] = (r < MAX_HEIGHT && c < MAX_WIDTH)
                        ? (optimized_pixel_t)(frame[r][c])
                        : (optimized_pixel_t)0;

      if (r < MAX_HEIGHT && c < MAX_WIDTH) {
        for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
          buf[i][c] = smallbuf[i];
        }
        buf[4][c] = smallbuf[4];
      }

      // 计算梯度
      optimized_pixel_t x_grad = 0;
      optimized_pixel_t y_grad = 0;

      if (r >= 4 && r < MAX_HEIGHT && c >= 4 && c < MAX_WIDTH) {
        for (int i = 0; i < 5; i++) {
#pragma HLS UNROLL
          x_grad += smallbuf[2] * GRAD_WEIGHTS[i];
          y_grad += smallbuf[i] * GRAD_WEIGHTS[2];
        }
        gradient_x[r - 2][c - 2] = x_grad / 12;
        gradient_y[r - 2][c - 2] = y_grad / 12;
      } else if (r >= 2 && c >= 2) {
        gradient_x[r - 2][c - 2] = 0;
        gradient_y[r - 2][c - 2] = 0;
      }
    }
  }
  return 0;
}

int gradient_z_calc_module(
    optimized_pixel_t frame1[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame2[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame3[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame4[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t frame5[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = frame1 offset = slave bundle = data
#pragma HLS interface m_axi port = frame2 offset = slave bundle = data
#pragma HLS interface m_axi port = frame3 offset = slave bundle = data
#pragma HLS interface m_axi port = frame4 offset = slave bundle = data
#pragma HLS interface m_axi port = frame5 offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_z offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  const int GRAD_WEIGHTS[] = {1, -8, 0, 8, -1};

GRAD_Z_OUTER:
  for (int r = 0; r < MAX_HEIGHT; r++) {
  GRAD_Z_INNER:
    for (int c = 0; c < MAX_WIDTH; c++) {
      gradient_z[r][c] = ((optimized_pixel_t)(frame1[r][c] * GRAD_WEIGHTS[0] +
                                              frame2[r][c] * GRAD_WEIGHTS[1] +
                                              frame3[r][c] * GRAD_WEIGHTS[2] +
                                              frame4[r][c] * GRAD_WEIGHTS[3] +
                                              frame5[r][c] * GRAD_WEIGHTS[4])) /
                         12;
    }
  }
  return 0;
}

int gradient_weight_y_module(
    optimized_pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH],
    optimized_pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH],
    gradient_t filt_grad[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = gradient_x offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_y offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_z offset = slave bundle = data
#pragma HLS interface m_axi port = filt_grad offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  // 用二维数组模拟行缓冲
  static gradient_t buf[7][MAX_WIDTH];
#pragma HLS array_partition variable = buf complete dim = 1

  const optimized_pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903,
                                           0.1869, 0.133, 0.0755};

GRAD_WEIGHT_Y_OUTER:
  for (int r = 0; r < MAX_HEIGHT + 3; r++) {
  GRAD_WEIGHT_Y_INNER:
    for (int c = 0; c < MAX_WIDTH; c++) {

      gradient_t tmp;
      if (r < MAX_HEIGHT) {
        // 行缓冲上移
        for (int i = 0; i < 6; i++) {
          buf[i][c] = buf[i + 1][c];
        }
        tmp.x = gradient_x[r][c];
        tmp.y = gradient_y[r][c];
        tmp.z = gradient_z[r][c];
        buf[6][c] = tmp;
      } else {
        // 如果超出最大高度，填0
        for (int i = 0; i < 6; i++) {
          buf[i][c] = buf[i + 1][c];
        }
        tmp.x = 0;
        tmp.y = 0;
        tmp.z = 0;
        buf[6][c] = tmp;
      }

      gradient_t acc;
      acc.x = 0;
      acc.y = 0;
      acc.z = 0;

      if (r >= 6 && r < MAX_HEIGHT) {
        // 应用滤波器
        for (int i = 0; i < 7; i++) {
          acc.x += buf[i][c].x * GRAD_FILTER[i];
          acc.y += buf[i][c].y * GRAD_FILTER[i];
          acc.z += buf[i][c].z * GRAD_FILTER[i];
        }
        filt_grad[r - 3][c] = acc;
      } else if (r >= 3) {
        filt_grad[r - 3][c] = acc;
      }
    }
  }
  return 0;
}

int gradient_weight_x_module(
    gradient_t y_filt[MAX_HEIGHT][MAX_WIDTH],
    gradient_t filt_grad[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = y_filt offset = slave bundle = data
#pragma HLS interface m_axi port = filt_grad offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  // 用一维数组模拟列缓冲
  static gradient_t buf[7];
#pragma HLS array_partition variable = buf complete dim = 1

  const optimized_pixel_t GRAD_FILTER[] = {0.0755, 0.133, 0.1869, 0.2903,
                                           0.1869, 0.133, 0.0755};

GRAD_WEIGHT_X_OUTER:
  for (int r = 0; r < MAX_HEIGHT; r++) {
  GRAD_WEIGHT_X_INNER:
    for (int c = 0; c < MAX_WIDTH + 3; c++) {

      gradient_t tmp;
      // 窗口左移
      for (int i = 0; i < 6; i++) {
        buf[i] = buf[i + 1];
      }

      if (c < MAX_WIDTH) {
        tmp = y_filt[r][c];
        buf[6] = tmp;
      } else {
        tmp.x = 0;
        tmp.y = 0;
        tmp.z = 0;
        buf[6] = tmp;
      }

      gradient_t acc;
      acc.x = 0;
      acc.y = 0;
      acc.z = 0;

      if (c >= 6 && c < MAX_WIDTH) {
        // 应用滤波器
        for (int i = 0; i < 7; i++) {
          acc.x += buf[i].x * GRAD_FILTER[i];
          acc.y += buf[i].y * GRAD_FILTER[i];
          acc.z += buf[i].z * GRAD_FILTER[i];
        }
        filt_grad[r][c - 3] = acc;
      } else if (c >= 3) {
        filt_grad[r][c - 3] = acc;
      }
    }
  }
  return 0;
}

int flow_calc_module(
    ap_uint<64> tensors_part1[MAX_HEIGHT][MAX_WIDTH], // t1, t2
    ap_uint<64> tensors_part2[MAX_HEIGHT][MAX_WIDTH], // t3, t4
    ap_uint<64> tensors_part3[MAX_HEIGHT][MAX_WIDTH], // t5, t6
    velocity_t outputs[MAX_HEIGHT][MAX_WIDTH],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = tensors_part1 offset = slave bundle = data
#pragma HLS interface m_axi port = tensors_part2 offset = slave bundle = data
#pragma HLS interface m_axi port = tensors_part3 offset = slave bundle = data
#pragma HLS interface m_axi port = outputs offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  static optimized_pixel_t buf[2]; // 临时缓冲区

FLOW_OUTER:
  for (int r = 0; r < MAX_HEIGHT; r++) {
  FLOW_INNER:
    for (int c = 0; c < MAX_WIDTH; c++) {

      // 提取各个部分的数据
      ap_uint<64> tensor_part1 = tensors_part1[r][c];
      ap_uint<64> tensor_part2 = tensors_part2[r][c];
      ap_uint<64> tensor_part3 = tensors_part3[r][c];

      // 将64位整数分解为多个float或固定点数
      calc_pixel_t t1 = (calc_pixel_t)(tensor_part1.range(31, 0));  // 提取t1
      calc_pixel_t t2 = (calc_pixel_t)(tensor_part1.range(63, 32)); // 提取t2
      calc_pixel_t t3 = (calc_pixel_t)(tensor_part2.range(31, 0));  // 提取t3
      calc_pixel_t t4 = (calc_pixel_t)(tensor_part2.range(63, 32)); // 提取t4
      calc_pixel_t t5 = (calc_pixel_t)(tensor_part3.range(31, 0));  // 提取t5
      calc_pixel_t t6 = (calc_pixel_t)(tensor_part3.range(63, 32)); // 提取t6

      if (r >= 2 && r < MAX_HEIGHT - 2 && c >= 2 && c < MAX_WIDTH - 2) {
        calc_pixel_t denom = t1 * t2 - t4 * t4;
        calc_pixel_t numer0 = t6 * t4 - t5 * t2;
        calc_pixel_t numer1 = t5 * t4 - t6 * t1;

        if (denom != 0) {
          buf[0] = numer0 / denom;
          buf[1] = numer1 / denom;
        } else {
          buf[0] = 0;
          buf[1] = 0;
        }
      } else {
        buf[0] = buf[1] = 0;
      }

      outputs[r][c].x = (vel_pixel_t)buf[0];
      outputs[r][c].y = (vel_pixel_t)buf[1];
    }
  }
  return 0;
}

int optical_flow_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析 buf1 和 buf2 给第一个和第二个子函数

  // 调用 read_input_module
  frames_t(*frames)[MAX_WIDTH] = (frames_t(*)[MAX_WIDTH])(buf1);
  optimized_pixel_t(*frame1_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2);
  optimized_pixel_t(*frame2_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 2 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 3 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame4_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 4 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame5_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 5 * MAX_HEIGHT * MAX_WIDTH);

  read_input_module(frames, frame1_a, frame2_a, frame3_a, frame3_b, frame4_a,
                    frame5_a, dummy_data5, dummy_data5, dummy_data5,
                    dummy_data5, dummy_data5, dummy_data11, dummy_data22,
                    dummy_data33, dummy_data44, dummy_data55);

  // 调用 gradient_xy_calc_module
  optimized_pixel_t(*gradient_x)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1);
  optimized_pixel_t(*gradient_y)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + MAX_HEIGHT * MAX_WIDTH);

  gradient_xy_calc_module(frame3_a, gradient_x, gradient_y, dummy_data5,
                          dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                          dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                          dummy_data11, dummy_data22, dummy_data33,
                          dummy_data44, dummy_data55);

  // 解析 buf3 和 buf4 给第三个和第四个子函数

  // 调用 gradient_z_calc_module
  optimized_pixel_t(*frame1_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3);
  optimized_pixel_t(*frame2_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3 + MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_b_2)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3 + 2 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame4_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3 + 3 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame5_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3 + 4 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*gradient_z)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf3 + 5 * MAX_HEIGHT * MAX_WIDTH);

  gradient_z_calc_module(frame1_b, frame2_b, frame3_b_2, frame4_b, frame5_b,
                         gradient_z, dummy_data6, dummy_data6, dummy_data6,
                         dummy_data6, dummy_data6, dummy_data6, dummy_data11,
                         dummy_data22, dummy_data33, dummy_data44,
                         dummy_data55);

  // 调用 gradient_weight_y_module
  gradient_t(*filt_grad)[MAX_WIDTH] = (gradient_t(*)[MAX_WIDTH])(buf4);

  gradient_weight_y_module(gradient_x, gradient_y, gradient_z, filt_grad,
                           dummy_data6, dummy_data6, dummy_data6, dummy_data6,
                           dummy_data6, dummy_data6, dummy_data6, dummy_data6,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);

  // 解析 buf5 和 buf6 给最后两个子函数

  // 调用 gradient_weight_x_module
  gradient_t(*y_filt)[MAX_WIDTH] = (gradient_t(*)[MAX_WIDTH])(buf5);

  gradient_weight_x_module(y_filt, filt_grad, dummy_data7, dummy_data7,
                           dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                           dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);

  // 调用 flow_calc_module
  ap_uint<64>(*tensors_part1)[MAX_WIDTH] = (ap_uint<64>(*)[MAX_WIDTH])(buf6);
  ap_uint<64>(*tensors_part2)[MAX_WIDTH] =
      (ap_uint<64>(*)[MAX_WIDTH])(buf6 + MAX_HEIGHT * MAX_WIDTH);
  ap_uint<64>(*tensors_part3)[MAX_WIDTH] =
      (ap_uint<64>(*)[MAX_WIDTH])(buf6 + 2 * MAX_HEIGHT * MAX_WIDTH);
  velocity_t(*outputs)[MAX_WIDTH] =
      (velocity_t(*)[MAX_WIDTH])(buf6 + 3 * MAX_HEIGHT * MAX_WIDTH);

  flow_calc_module(tensors_part1, tensors_part2, tensors_part3, outputs,
                   dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                   dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                   dummy_data11, dummy_data22, dummy_data33, dummy_data44,
                   dummy_data55);

  return 0;
}

int optical_flow_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 从 buf1 和 buf2 解析数据，并传递给所有六个子函数

  // 调用第一个子函数
  frames_t(*frames)[MAX_WIDTH] = (frames_t(*)[MAX_WIDTH])(buf1);
  optimized_pixel_t(*frame1_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2);
  optimized_pixel_t(*frame2_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 2 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 3 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame4_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 4 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame5_a)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + 5 * MAX_HEIGHT * MAX_WIDTH);

  read_input_module(frames, frame1_a, frame2_a, frame3_a, frame3_b, frame4_a,
                    frame5_a, dummy_data5, dummy_data5, dummy_data5,
                    dummy_data5, dummy_data5, dummy_data11, dummy_data22,
                    dummy_data33, dummy_data44, dummy_data55);

  // 调用第二个子函数
  optimized_pixel_t(*gradient_x)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1);
  optimized_pixel_t(*gradient_y)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf2 + MAX_HEIGHT * MAX_WIDTH);

  gradient_xy_calc_module(frame3_a, gradient_x, gradient_y, dummy_data5,
                          dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                          dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                          dummy_data11, dummy_data22, dummy_data33,
                          dummy_data44, dummy_data55);

  // 调用第三个子函数
  optimized_pixel_t(*frame1_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1);
  optimized_pixel_t(*frame2_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1 + MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame3_b_2)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1 + 2 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame4_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1 + 3 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*frame5_b)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1 + 4 * MAX_HEIGHT * MAX_WIDTH);
  optimized_pixel_t(*gradient_z)[MAX_WIDTH] =
      (optimized_pixel_t(*)[MAX_WIDTH])(buf1 + 5 * MAX_HEIGHT * MAX_WIDTH);

  gradient_z_calc_module(frame1_b, frame2_b, frame3_b_2, frame4_b, frame5_b,
                         gradient_z, dummy_data5, dummy_data5, dummy_data5,
                         dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                         dummy_data22, dummy_data33, dummy_data44,
                         dummy_data55);

  // 调用第四个子函数
  gradient_t(*filt_grad)[MAX_WIDTH] = (gradient_t(*)[MAX_WIDTH])(buf2);

  gradient_weight_y_module(gradient_x, gradient_y, gradient_z, filt_grad,
                           dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                           dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);

  // 调用第五个子函数
  gradient_t(*y_filt)[MAX_WIDTH] = (gradient_t(*)[MAX_WIDTH])(buf1);

  gradient_weight_x_module(y_filt, filt_grad, dummy_data5, dummy_data5,
                           dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                           dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);

  // 调用第六个子函数
  ap_uint<64>(*tensors_part1)[MAX_WIDTH] = (ap_uint<64>(*)[MAX_WIDTH])(buf2);
  ap_uint<64>(*tensors_part2)[MAX_WIDTH] =
      (ap_uint<64>(*)[MAX_WIDTH])(buf2 + MAX_HEIGHT * MAX_WIDTH);
  ap_uint<64>(*tensors_part3)[MAX_WIDTH] =
      (ap_uint<64>(*)[MAX_WIDTH])(buf2 + 2 * MAX_HEIGHT * MAX_WIDTH);
  velocity_t(*outputs)[MAX_WIDTH] =
      (velocity_t(*)[MAX_WIDTH])(buf2 + 3 * MAX_HEIGHT * MAX_WIDTH);

  flow_calc_module(tensors_part1, tensors_part2, tensors_part3, outputs,
                   dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                   dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                   dummy_data11, dummy_data22, dummy_data33, dummy_data44,
                   dummy_data55);

  return 0;
}

void optical_flow(frames_t frames[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t frame1_a[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t frame2_a[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t frame3_a[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t frame4_a[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t frame5_a[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t gradient_x[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t gradient_y[MAX_HEIGHT][MAX_WIDTH],
                  optimized_pixel_t gradient_z[MAX_HEIGHT][MAX_WIDTH],
                  gradient_t x_filt[MAX_HEIGHT][MAX_WIDTH],
                  velocity_t velocity_output[MAX_HEIGHT][MAX_WIDTH],
                  volatile ap_uint<64> *dummy_data1,
                  volatile ap_uint<32> dummy_data11,
                  volatile ap_uint<32> dummy_data22,
                  volatile ap_uint<32> dummy_data33,
                  volatile ap_uint<32> dummy_data44,
                  volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = frames offset = slave bundle = data
#pragma HLS interface m_axi port = frame1_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame2_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame3_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame4_a offset = slave bundle = data
#pragma HLS interface m_axi port = frame5_a offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_x offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_y offset = slave bundle = data
#pragma HLS interface m_axi port = gradient_z offset = slave bundle = data
#pragma HLS interface m_axi port = x_filt offset = slave bundle = data
#pragma HLS interface m_axi port = velocity_output offset = slave bundle = data
#pragma HLS interface m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl
  // 调用 read_input_module
  read_input_module(frames, frame1_a, frame2_a, frame3_a, frame3_a, frame4_a,
                    frame5_a, dummy_data1, dummy_data1, dummy_data1,
                    dummy_data1, dummy_data1, dummy_data11, dummy_data22,
                    dummy_data33, dummy_data44, dummy_data55);

  // 调用 gradient_xy_calc_module
  gradient_xy_calc_module(frame3_a, gradient_x, gradient_y, dummy_data1,
                          dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                          dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                          dummy_data11, dummy_data22, dummy_data33,
                          dummy_data44, dummy_data55);

  // 调用 gradient_z_calc_module
  gradient_z_calc_module(
      frame1_a, frame2_a, frame3_a, frame4_a, frame5_a, gradient_z, dummy_data1,
      dummy_data1, dummy_data1, dummy_data1, dummy_data1, dummy_data1,
      dummy_data11, dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用 gradient_weight_y_module
  gradient_weight_y_module(gradient_x, gradient_y, gradient_z, x_filt,
                           dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                           dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);

  // 调用 gradient_weight_x_module
  gradient_weight_x_module(x_filt, x_filt, dummy_data1, dummy_data1,
                           dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                           dummy_data1, dummy_data1, dummy_data1, dummy_data1,
                           dummy_data11, dummy_data22, dummy_data33,
                           dummy_data44, dummy_data55);
}

int check_clockwise(Triangle_2D triangle_2d) {
  int cw;
  cw = (triangle_2d.x2 - triangle_2d.x0) * (triangle_2d.y1 - triangle_2d.y0) -
       (triangle_2d.y2 - triangle_2d.y0) * (triangle_2d.x1 - triangle_2d.x0);
  return cw;
}

void clockwise_vertices(Triangle_2D *triangle_2d) {
  bit8 tmp_x, tmp_y;
  tmp_x = triangle_2d->x0;
  tmp_y = triangle_2d->y0;
  triangle_2d->x0 = triangle_2d->x1;
  triangle_2d->y0 = triangle_2d->y1;
  triangle_2d->x1 = tmp_x;
  triangle_2d->y1 = tmp_y;
}

bit1 pixel_in_triangle(bit8 x, bit8 y, Triangle_2D triangle_2d) {
  int pi0, pi1, pi2;
  pi0 = (x - triangle_2d.x0) * (triangle_2d.y1 - triangle_2d.y0) -
        (y - triangle_2d.y0) * (triangle_2d.x1 - triangle_2d.x0);
  pi1 = (x - triangle_2d.x1) * (triangle_2d.y2 - triangle_2d.y1) -
        (y - triangle_2d.y1) * (triangle_2d.x2 - triangle_2d.x1);
  pi2 = (x - triangle_2d.x2) * (triangle_2d.y0 - triangle_2d.y2) -
        (y - triangle_2d.y2) * (triangle_2d.x0 - triangle_2d.x2);
  return (pi0 >= 0 && pi1 >= 0 && pi2 >= 0);
}

bit8 find_min(bit8 in0, bit8 in1, bit8 in2) {
  if (in0 < in1) {
    if (in0 < in2)
      return in0;
    else
      return in2;
  } else {
    if (in1 < in2)
      return in1;
    else
      return in2;
  }
}

bit8 find_max(bit8 in0, bit8 in1, bit8 in2) {
  if (in0 > in1) {
    if (in0 > in2)
      return in0;
    else
      return in2;
  } else {
    if (in1 > in2)
      return in1;
    else
      return in2;
  }
}

int bundle1_module(
    bit32 input[3 * NUM_3D_TRI], CandidatePixel fragment2[NUM_3D_TRI][500],
    bit16 size_fragment[NUM_3D_TRI], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
    volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
    volatile ap_uint<64> *dummy_data8, volatile ap_uint<64> *dummy_data9,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {

#pragma HLS interface m_axi port = input offset = slave bundle = data
#pragma HLS interface m_axi port = fragment2 offset = slave bundle = data
#pragma HLS interface m_axi port = size_fragment offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  Triangle_3D triangle_3d;
  Triangle_2D triangle_2d;
  Triangle_2D triangle_2d_same;
  bit8 max_min[5];
  bit16 max_index;
  bit2 flag;

  for (bit16 i = 0; i < NUM_3D_TRI; i++) {
#pragma HLS PIPELINE
    bit32 input_lo = input[3 * i];
    bit32 input_mi = input[3 * i + 1];
    bit32 input_hi = input[3 * i + 2];

    triangle_3d.x0 = input_lo(7, 0);
    triangle_3d.y0 = input_lo(15, 8);
    triangle_3d.z0 = input_lo(23, 16);
    triangle_3d.x1 = input_lo(31, 24);
    triangle_3d.y1 = input_mi(7, 0);
    triangle_3d.z1 = input_mi(15, 8);
    triangle_3d.x2 = input_mi(23, 16);
    triangle_3d.y2 = input_mi(31, 24);
    triangle_3d.z2 = input_hi(7, 0);

    triangle_2d.x0 = triangle_3d.x0;
    triangle_2d.y0 = triangle_3d.y0;
    triangle_2d.x1 = triangle_3d.x1;
    triangle_2d.y1 = triangle_3d.y1;
    triangle_2d.x2 = triangle_3d.x2;
    triangle_2d.y2 = triangle_3d.y2;
    triangle_2d.z =
        triangle_3d.z0 / 3 + triangle_3d.z1 / 3 + triangle_3d.z2 / 3;

    if (check_clockwise(triangle_2d) == 0) {
      flag = 1;
    } else {
      flag = 0;
    }
    if (check_clockwise(triangle_2d) < 0)
      clockwise_vertices(&(triangle_2d));

    triangle_2d_same.x0 = triangle_2d.x0;
    triangle_2d_same.y0 = triangle_2d.y0;
    triangle_2d_same.x1 = triangle_2d.x1;
    triangle_2d_same.y1 = triangle_2d.y1;
    triangle_2d_same.x2 = triangle_2d.x2;
    triangle_2d_same.y2 = triangle_2d.y2;
    triangle_2d_same.z = triangle_2d.z;

    max_min[0] = find_min(triangle_2d.x0, triangle_2d.x1, triangle_2d.x2);
    max_min[1] = find_max(triangle_2d.x0, triangle_2d.x1, triangle_2d.x2);
    max_min[2] = find_min(triangle_2d.y0, triangle_2d.y1, triangle_2d.y2);
    max_min[3] = find_max(triangle_2d.y0, triangle_2d.y1, triangle_2d.y2);
    max_min[4] = max_min[1] - max_min[0];

    max_index = (max_min[1] - max_min[0]) * (max_min[3] - max_min[2]);

    if (flag) {
      size_fragment[i] = 0;
      continue;
    }
    bit8 color = 100;
    bit16 j = 0;

  RAST2:
    for (bit16 k = 0; k < max_index; k++) {
#pragma HLS PIPELINE II = 1
      bit8 x = max_min[0] + k % max_min[4];
      bit8 y = max_min[2] + k / max_min[4];

      if (pixel_in_triangle(x, y, triangle_2d_same)) {
        fragment2[i][j].x = x;
        fragment2[i][j].y = y;
        fragment2[i][j].z = triangle_2d_same.z;
        fragment2[i][j].color = color;
        j++;
      }
    }
    size_fragment[i] = j;
  }
  return 0;
}

int zculling_module(
    CandidatePixel fragments[NUM_3D_TRI][500], bit16 size[NUM_3D_TRI],
    Pixel pixels[NUM_3D_TRI][500], bit16 size_pixels[NUM_3D_TRI],
    bit8 z_buffer[MAX_X][MAX_Y], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
    volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = fragments offset = slave bundle = data
#pragma HLS interface m_axi port = size offset = slave bundle = data
#pragma HLS interface m_axi port = pixels offset = slave bundle = data
#pragma HLS interface m_axi port = size_pixels offset = slave bundle = data
#pragma HLS interface m_axi port = z_buffer offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  for (bit16 m = 0; m < NUM_3D_TRI; m++) {
    if (m == 0) {
    ZCULLING_INIT_ROW:
      for (bit16 i = 0; i < MAX_X; i++) {
      ZCULLING_INIT_COL:
        for (bit16 j = 0; j < MAX_Y; j++) {
          z_buffer[i][j] = 255;
        }
      }
    }

    bit16 pixel_cntr = 0;

  ZCULLING:
    for (bit16 n = 0; n < size[m]; n++) {
      if (fragments[m][n].z < z_buffer[fragments[m][n].y][fragments[m][n].x]) {
        pixels[m][pixel_cntr].x = fragments[m][n].x;
        pixels[m][pixel_cntr].y = fragments[m][n].y;
        pixels[m][pixel_cntr].color = fragments[m][n].color;
        pixel_cntr++;
        z_buffer[fragments[m][n].y][fragments[m][n].x] = fragments[m][n].z;
      }
    }
    size_pixels[m] = pixel_cntr;
  }
  return 0;
}

int bundle2_module(
    bit16 size_pixels[NUM_3D_TRI], Pixel pixels[NUM_3D_TRI][500],
    bit32 output[NUM_FB], bit8 frame_buffer[MAX_X][MAX_Y],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {

#pragma HLS interface m_axi port = size_pixels offset = slave bundle = data
#pragma HLS interface m_axi port = pixels offset = slave bundle = data
#pragma HLS interface m_axi port = output offset = slave bundle = data
#pragma HLS interface m_axi port = frame_buffer offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  for (bit16 m = 0; m < NUM_3D_TRI; m++) {
    if (m == 0) {
    COLORING_FB_INIT_ROW:
      for (bit16 i = 0; i < MAX_X; i++) {
      COLORING_FB_INIT_COL:
        for (bit16 j = 0; j < MAX_Y; j++)
          frame_buffer[i][j] = 0;
      }
    }

  COLORING_FB:
    for (bit16 i = 0; i < size_pixels[m]; i++) {
      frame_buffer[pixels[m][i].x][pixels[m][i].y] = pixels[m][i].color;
    }
  }

  bit32 out_FB = 0;
OUTPUT_FB_ROW:
  for (bit16 i = 0; i < MAX_X; i++) {
  OUTPUT_FB_COL:
    for (bit16 j = 0; j < MAX_Y; j = j + 4) {
      out_FB(7, 0) = frame_buffer[i][j + 0];
      out_FB(15, 8) = frame_buffer[i][j + 1];
      out_FB(23, 16) = frame_buffer[i][j + 2];
      out_FB(31, 24) = frame_buffer[i][j + 3];
      output[i * MAX_Y / 4 + j / 4] = out_FB;
    }
  }
  return 0;
}

int render3d_p(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析所有数据都从 buf1 和 buf2 中解析出来
  bit32 *input = (bit32 *)(buf1);
  CandidatePixel(*fragment2)[500] = (CandidatePixel(*)[500])(buf2);
  bit16 *size_fragment = (bit16 *)(buf1 + sizeof(*input));
  bit16 *size = (bit16 *)(buf1 + sizeof(*input) + sizeof(*size_fragment));
  Pixel(*pixels)[500] = (Pixel(*)[500])(buf2 + sizeof(*fragment2));
  bit16 *size_pixels = (bit16 *)(buf2 + sizeof(*fragment2) + sizeof(*pixels));
  bit32 *output = (bit32 *)(buf2 + sizeof(*fragment2) + sizeof(*pixels) +
                            sizeof(*size_pixels));
  bit8(*z_buffer)[MAX_Y] = (bit8(*)[MAX_Y])(
      buf1 + sizeof(*input) + sizeof(*size_fragment) + sizeof(*size));
  bit8(*frame_buffer)[MAX_Y] = (bit8(*)[MAX_Y])(
      buf2 + sizeof(*fragment2) + sizeof(*pixels) + sizeof(*output));

  // 解析 buf6: 解析 render3d 的第三个子模块 bundle2_module 所需的数据
  //   bit32 *output = (bit32 *)(buf6 + sizeof(*z_buffer));
  //   bit8(*frame_buffer)[MAX_Y] =
  //       (bit8(*)[MAX_Y])(buf6 + sizeof(*z_buffer) + sizeof *(output));

  // 调用第一个模块 bundle1_module
  bundle1_module(input, fragment2, size_fragment, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第二个模块 zculling_module
  zculling_module(fragment2, size_fragment, pixels, size_pixels, z_buffer,
                  dummy_data6, dummy_data6, dummy_data6, dummy_data6,
                  dummy_data6, dummy_data6, dummy_data6, dummy_data11,
                  dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第三个模块 bundle2_module
  bundle2_module(size_pixels, pixels, output, frame_buffer, dummy_data7,
                 dummy_data7, dummy_data7, dummy_data7, dummy_data7,
                 dummy_data7, dummy_data7, dummy_data7, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int render3d_s(
    volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
    volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析所有数据都从 buf1 和 buf2 中解析出来
  bit32 *input = (bit32 *)(buf1);
  CandidatePixel(*fragment2)[500] = (CandidatePixel(*)[500])(buf2);
  bit16 *size_fragment = (bit16 *)(buf1 + sizeof(*input));
  bit16 *size = (bit16 *)(buf1 + sizeof(*input) + sizeof(*size_fragment));
  Pixel(*pixels)[500] = (Pixel(*)[500])(buf2 + sizeof(*fragment2));
  bit16 *size_pixels = (bit16 *)(buf2 + sizeof(*fragment2) + sizeof(*pixels));
  bit32 *output = (bit32 *)(buf2 + sizeof(*fragment2) + sizeof(*pixels) +
                            sizeof(*size_pixels));
  bit8(*z_buffer)[MAX_Y] = (bit8(*)[MAX_Y])(
      buf1 + sizeof(*input) + sizeof(*size_fragment) + sizeof(*size));
  bit8(*frame_buffer)[MAX_Y] = (bit8(*)[MAX_Y])(
      buf2 + sizeof(*fragment2) + sizeof(*pixels) + sizeof(*output));

  // 调用第一个模块 bundle1_module
  bundle1_module(input, fragment2, size_fragment, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第二个模块 zculling_module
  zculling_module(fragment2, size_fragment, pixels, size_pixels, z_buffer,
                  dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                  dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                  dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第三个模块 bundle2_module
  bundle2_module(size_pixels, pixels, output, frame_buffer, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int render3d(
    bit32 input[3 * NUM_3D_TRI], CandidatePixel fragment2[NUM_3D_TRI][500],
    bit16 size_fragment[NUM_3D_TRI], bit16 size[NUM_3D_TRI],
    Pixel pixels[NUM_3D_TRI][500], bit16 size_pixels[NUM_3D_TRI],
    bit32 output[NUM_FB], bit8 z_buffer[MAX_X][MAX_Y],
    bit8 frame_buffer[MAX_X][MAX_Y], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = input offset = slave bundle = data
#pragma HLS interface m_axi port = fragment2 offset = slave bundle = data
#pragma HLS interface m_axi port = size_fragment offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = size offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = pixels offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = size_pixels offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = output offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = z_buffer offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = frame_buffer offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl
  // 调用第一个模块 bundle1_module
  bundle1_module(input, fragment2, size_fragment, dummy_data1, dummy_data2,
                 dummy_data3, dummy_data1, dummy_data2, dummy_data3,
                 dummy_data1, dummy_data2, dummy_data3, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第二个模块 zculling_module
  zculling_module(fragment2, size_fragment, pixels, size_pixels, z_buffer,
                  dummy_data1, dummy_data2, dummy_data3, dummy_data1,
                  dummy_data2, dummy_data3, dummy_data1, dummy_data11,
                  dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 调用第三个模块 bundle2_module
  bundle2_module(size_pixels, pixels, output, frame_buffer, dummy_data1,
                 dummy_data2, dummy_data3, dummy_data1, dummy_data2,
                 dummy_data3, dummy_data1, dummy_data2, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);
  return 0;
}

WholeDigitType pack_whole_digit(const DigitType *input) {
#pragma HLS inline
  WholeDigitType output;
  output.range(63, 0) = input[0];
  output.range(127, 64) = input[1];
  output.range(191, 128) = input[2];
  output.range(255, 192) = input[3];
  return output;
}

void unpack_whole_digit(DigitType *output, WholeDigitType input) {
  output[0] = input.range(63, 0);
  output[1] = input.range(127, 64);
  output[2] = input.range(191, 128);
  output[3] = input.range(255, 192);
}

// popcount function
int popcount(WholeDigitType x) {
  // most straightforward implementation
  // actually not bad on FPGA
  int cnt = 0;
  for (int i = 0; i < 256; i++)
    cnt = cnt + x[i];

  return cnt;
}

// Given the test instance and a (new) training instance, this
// function maintains/updates an array of K minimum
// distances per training set.
// void update_knn( WholeDigitType test_inst, WholeDigitType train_inst, int
// min_distances[K_CONST] )
void update_knn(const DigitType *test_inst, const DigitType *train_inst,
                int min_distances[K_CONST]) {
#pragma HLS inline

  // Compute the difference using XOR
  WholeDigitType test_inst_whole = pack_whole_digit(test_inst);
  WholeDigitType train_inst_whole = pack_whole_digit(train_inst);
  WholeDigitType diff = test_inst_whole ^ train_inst_whole;

  int dist = 0;

  dist = popcount(diff);

  int max_dist = 0;
  int max_dist_id = K_CONST + 1;
  int k = 0;

// Find the max distance
FIND_MAX_DIST:
  for (int k = 0; k < K_CONST; ++k) {
    if (min_distances[k] > max_dist) {
      max_dist = min_distances[k];
      max_dist_id = k;
    }
  }

  // Replace the entry with the max distance
  if (dist < max_dist)
    min_distances[max_dist_id] = dist;

  return;
}

// Given 10xK minimum distance values, this function
// finds the actual K nearest neighbors and determines the
// final output based on the most common int represented by
// these nearest neighbors (i.e., a vote among KNNs).
LabelType knn_vote(int knn_set[PAR_FACTOR * K_CONST]) {
#pragma HLS inline

  // local buffers

  // final K nearest neighbors
  int min_distance_list[K_CONST];
#pragma HLS array_partition variable = min_distance_list complete dim = 0
  // labels for the K nearest neighbors
  int label_list[K_CONST];
#pragma HLS array_partition variable = label_list complete dim = 0
  // voting boxes
  int vote_list[10];
#pragma HLS array_partition variable = vote_list complete dim = 0

  int pos = 1000;

// initialize
INIT_1:
  for (int i = 0; i < K_CONST; i++) {
#pragma HLS unroll
    min_distance_list[i] = 256;
    label_list[i] = 9;
  }

INIT_2:
  for (int i = 0; i < 10; i++) {
#pragma HLS unroll
    vote_list[i] = 0;
  }

// go through all the lanes
// do an insertion sort to keep a sorted neighbor list
LANES:
  for (int i = 0; i < PAR_FACTOR; i++) {
  INSERTION_SORT_OUTER:
    for (int j = 0; j < K_CONST; j++) {
#pragma HLS pipeline
      pos = 1000;
    INSERTION_SORT_INNER:
      for (int r = 0; r < K_CONST; r++) {
#pragma HLS unroll
        pos = ((knn_set[i * K_CONST + j] < min_distance_list[r]) &&
               (pos > K_CONST))
                  ? r
                  : pos;
      }

    INSERT:
      for (int r = K_CONST; r > 0; r--) {
#pragma HLS unroll
        if (r - 1 > pos) {
          min_distance_list[r - 1] = min_distance_list[r - 2];
          label_list[r - 1] = label_list[r - 2];
        } else if (r - 1 == pos) {
          min_distance_list[r - 1] = knn_set[i * K_CONST + j];
          if (PAR_FACTOR >= 10) {
            label_list[r - 1] = i / (PAR_FACTOR / 10);
          } else {
            label_list[r - 1] = i;
          }
        }
      }
    }
  }

// vote
INCREMENT:
  for (int i = 0; i < K_CONST; i++) {
#pragma HLS pipeline
    vote_list[label_list[i]] += 1;
  }

  LabelType max_vote;
  max_vote = 0;

// find the maximum value
VOTE:
  for (int i = 0; i < 10; i++) {
#pragma HLS unroll
    if (vote_list[i] >= vote_list[max_vote]) {
      max_vote = i;
    }
  }

  return max_vote;
}

int knn_init_module(
    int knn_set[NUM_TEST][PAR_FACTOR * K_CONST],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<64> *dummy_data12, volatile ap_uint<32> dummy_data11,
    volatile ap_uint<32> dummy_data22, volatile ap_uint<32> dummy_data33,
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = knn_set offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data12 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  for (int t = 0; t < NUM_TEST; ++t) {
    for (int i = 0; i < K_CONST * PAR_FACTOR; ++i) {
#pragma HLS pipeline
      knn_set[t][i] = 256;
    }
  }
  return 0;
}

int train_module(
    const DigitType training_set[NUM_TRAINING * 4],
    const DigitType test_set[NUM_TEST * 4],
    int knn_set[NUM_TEST][PAR_FACTOR * K_CONST],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<32> dummy_data11,
    volatile ap_uint<32> dummy_data22, volatile ap_uint<32> dummy_data33,
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = training_set offset = slave bundle = data
#pragma HLS interface m_axi port = test_set offset = slave bundle = data
#pragma HLS interface m_axi port = knn_set offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

// loop through test set
TEST_LOOP:
  for (int t = 0; t < NUM_TEST; ++t) {
  TRAINING_LOOP:
    for (int i = 0; i < NUM_TRAINING / PAR_FACTOR; ++i) {
    // #pragma HLS pipeline
    LANES:
      for (int j = 0; j < PAR_FACTOR; j++) {
        // #pragma HLS pipeline
        // #pragma HLS unroll
        update_knn(&test_set[t * 4],
                   &training_set[(j * NUM_TRAINING / PAR_FACTOR + i) * 4],
                   &knn_set[t][j * K_CONST]);
      }
    }
  }
  return 0;
}

int predict_module(
    LabelType global_results[NUM_TEST],
    int knn_set[NUM_TEST][PAR_FACTOR * K_CONST],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = global_results offset = slave bundle = data
#pragma HLS interface m_axi port = knn_set offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl

  // local buffer
  static LabelType results[NUM_TEST];

  // loop through test set
TEST_LOOP:
  for (int t = 0; t < NUM_TEST; ++t) {
    // Compute the final output
    LabelType max_vote = knn_vote(knn_set[t]);
    results[t] = max_vote;
  }

  // copy the results out
  for (int i = 0; i < NUM_TEST; i++) {
    // #pragma HLS pipeline
    global_results[i] = results[i];
  }
  return 0;
}

int knn_p(volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
          volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
          volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
          volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
          volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
          volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
          volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
          volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析所有数据都从 buf1 和 buf2 中解析出来
  const DigitType(*training_set)[4] =
      (DigitType(*)[4])(buf1); // 解析 training_set
  const DigitType(*test_set)[4] = (DigitType(*)[4])(buf2); // 解析 test_set
  LabelType *global_results = (LabelType *)(buf2); // 解析 global_results
  int(*knn_set)[PAR_FACTOR * K_CONST] =
      (int(*)[PAR_FACTOR * K_CONST])(buf2); // 解析 knn_set

  // // 初始化 knn_set
  // knn_init_module(knn_set, dummy_data5, dummy_data5, dummy_data5,
  //                 dummy_data5, dummy_data5, dummy_data5,
  //                 dummy_data5, dummy_data5, dummy_data5,
  //                 dummy_data5, dummy_data5, dummy_data11,
  //                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 训练 knn_set
  train_module(*training_set, *test_set, knn_set, dummy_data6, dummy_data6,
               dummy_data6, dummy_data6, dummy_data6, dummy_data6, dummy_data6,
               dummy_data6, dummy_data6, dummy_data11, dummy_data22,
               dummy_data33, dummy_data44, dummy_data55);

  // // 预测结果
  // predict_module(global_results, knn_set,
  //                dummy_data7, dummy_data7, dummy_data7,
  //                dummy_data7, dummy_data7, dummy_data7,
  //                dummy_data7, dummy_data7, dummy_data7,
  //                dummy_data7, dummy_data11, dummy_data22,
  //                dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int knn_s(volatile DTYPE *buf1, volatile DTYPE *buf2, volatile DTYPE *buf3,
          volatile DTYPE *buf4, volatile DTYPE *buf5, volatile DTYPE *buf6,
          volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
          volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
          volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
          volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
          volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
          volatile ap_uint<32> dummy_data55) {
#pragma HLS INTERFACE m_axi port = buf1 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf2 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = buf3 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf4 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = buf5 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = buf6 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data2
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data3
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data1
#pragma HLS INTERFACE m_axi port = dummy_data10 offset = slave bundle = data1

#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = return bundle = ctrl

  // 解析所有数据都从 buf1 和 buf2 中解析出来
  const DigitType(*training_set)[4] =
      (DigitType(*)[4])(buf1); // 解析 training_set
  const DigitType(*test_set)[4] = (DigitType(*)[4])(buf2); // 解析 test_set
  LabelType *global_results = (LabelType *)(buf1); // 解析 global_results
  int(*knn_set)[PAR_FACTOR * K_CONST] =
      (int(*)[PAR_FACTOR * K_CONST])(buf2); // 解析 knn_set

  // 初始化 knn_set
  knn_init_module(knn_set, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                  dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                  dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                  dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // 训练 knn_set
  train_module(*training_set, *test_set, knn_set, dummy_data5, dummy_data5,
               dummy_data5, dummy_data5, dummy_data5, dummy_data5, dummy_data5,
               dummy_data5, dummy_data5, dummy_data11, dummy_data22,
               dummy_data33, dummy_data44, dummy_data55);

  // 预测结果
  predict_module(global_results, knn_set, dummy_data5, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data5,
                 dummy_data5, dummy_data5, dummy_data5, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  return 0;
}

int knn(const DigitType training_set[NUM_TRAINING * 4],
        const DigitType test_set[NUM_TEST * 4],
        LabelType global_results[NUM_TEST], volatile ap_uint<64> *dummy_data1,
        volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
        volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
        volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
        volatile ap_uint<64> *dummy_data8, volatile ap_uint<64> *dummy_data9,
        volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
        volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
        volatile ap_uint<32> dummy_data55) {
#pragma HLS interface m_axi port = global_results offset = slave bundle = data
#pragma HLS interface m_axi port = training_set offset = slave bundle = data
#pragma HLS interface m_axi port = test_set offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data1 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data2 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data3 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data4 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data5 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data6 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data7 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data8 offset = slave bundle = data
#pragma HLS INTERFACE m_axi port = dummy_data9 offset = slave bundle = data
#pragma HLS INTERFACE s_axilite register port = dummy_data11 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data22 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data33 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data44 bundle = ctrl
#pragma HLS INTERFACE s_axilite register port = dummy_data55 bundle = ctrl
#pragma HLS interface s_axilite port = return bundle = ctrl
  // Initialize knn_set
  int knn_set[NUM_TEST][PAR_FACTOR * K_CONST];
  knn_init_module(knn_set, dummy_data1, dummy_data2, dummy_data3, dummy_data4,
                  dummy_data5, dummy_data6, dummy_data7, dummy_data8,
                  dummy_data9, dummy_data1, dummy_data1, dummy_data11,
                  dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  // Train knn_set with training data
  train_module(training_set, test_set, knn_set, dummy_data1, dummy_data2,
               dummy_data3, dummy_data4, dummy_data5, dummy_data6, dummy_data7,
               dummy_data8, dummy_data9, dummy_data11, dummy_data22,
               dummy_data33, dummy_data44, dummy_data55);

  // Predict results using trained knn_set
  predict_module(global_results, knn_set, dummy_data1, dummy_data2, dummy_data3,
                 dummy_data4, dummy_data5, dummy_data6, dummy_data7,
                 dummy_data8, dummy_data9, dummy_data1, dummy_data11,
                 dummy_data22, dummy_data33, dummy_data44, dummy_data55);

  return 0;
}