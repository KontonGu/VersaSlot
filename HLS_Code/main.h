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

#include "ap_axi_sdata.h"
#include "ap_fixed.h"
#include "ap_int.h"
#include <iostream>
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Image_Compression

#define IMAGE_MAX_M 1920
#define IMAGE_MAX_N 1080
#define MAX_HEIGHT 1080
#define MAX_WIDTH 1920

typedef float DTYPE;
typedef float QTYPE;

#define DZERO 0.00f
#define QZERO 0

void padImage(volatile DTYPE *src, volatile DTYPE *dst, int srcm, int srcn,
              int dstm, int dstn);
void extractBlockD(volatile DTYPE *src, volatile DTYPE *dst, int dstm,
                   int dstn);
int matmul_fast(DTYPE srcA[8][8], DTYPE srcB[8][8], DTYPE dst[8][8]);

int bundle1(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

int dct(volatile DTYPE *buf1, volatile DTYPE *buf2,
        volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
        volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
        volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
        volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
        volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
        int srcm, int srcn, int dstm, int dstn,
        volatile ap_uint<32> dummy_data);

int quantize(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

int dequantize(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

int idct(volatile DTYPE *buf1, volatile DTYPE *buf2,
         volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
         volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
         volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
         volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
         volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
         int srcm, int srcn, int dstm, int dstn,
         volatile ap_uint<32> dummy_data);

int reconstructBlockD(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<32> *dummy_data1, volatile ap_uint<32> *dummy_data2,
    volatile ap_uint<32> *dummy_data3, volatile ap_uint<32> *dummy_data4,
    volatile ap_uint<32> *dummy_data5, volatile ap_uint<32> *dummy_data6,
    volatile ap_uint<32> *dummy_data7, volatile ap_uint<32> *dummy_data8,
    volatile ap_uint<32> *dummy_data9, volatile ap_uint<32> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

int bundle1_dct_quantize(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

int dequantize_idct_recon(
    volatile DTYPE *buf1, volatile DTYPE *buf2,
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<64> *dummy_data7, volatile ap_uint<64> *dummy_data8,
    volatile ap_uint<64> *dummy_data9, volatile ap_uint<64> *dummy_data10,
    int srcm, int srcn, int dstm, int dstn, volatile ap_uint<32> dummy_data);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Alex_Net

int conv_I(float input[3][227][227], float conv_core[96][3][11][11],
           float conv_core_weights[96][3][11][11], float conv_core_bias[96],
           float output[96][55][55]);

int MaxPool_I(float input[96][55][55], float output[96][27][27]);

void pad_image_I(float input[96][27][27], float output[96][31][31]);

int conv_II(float input[96][27][27], float conv_core[256][96][5][5],
            float conv_core_weights[256][96][5][5], float conv_core_bias[256],
            float output[256][27][27]);

int MaxPool_II(float input[256][27][27], float output[256][13][13]);

void pad_image_II(float input[256][13][13], float output[256][15][15]);

int conv_III(
    float input[256][13][13], float conv_core[384][256][3][3],
    float conv_core_weights[384][256][3][3], float conv_core_bias[384],
    float output[384][13][13], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
    volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

void pad_image_III(float input[384][13][13], float output[384][15][15]);

int conv_IV(
    float input[384][13][13], float conv_core[384][384][3][3],
    float conv_core_weights[384][384][3][3], float conv_core_bias[384],
    float output[384][13][13], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<64> *dummy_data4, volatile ap_uint<64> *dummy_data5,
    volatile ap_uint<64> *dummy_data6, volatile ap_uint<64> *dummy_data7,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

int conv_V(float input[384][13][13], float conv_core[256][384][3][3],
           float conv_core_weights[256][384][3][3], float conv_core_bias[256],
           float output[256][13][13]);

int MaxPool_III(float input[256][13][13], float output[256][6][6]);

int full_connection_I(float input[256][6][6], float weights[4096][256 * 6 * 6],
                      float bias[4096], float output[4096]);

int full_connection_II(float input[4096], float weights[4096][4096],
                       float bias[4096], float output[4096]);

int full_connection_III(float input[4096], float weights[1000][4096],
                        float bias[1000], float output[1000]);

int conv_pool_I(
    float input[3][227][227], float conv_core[96][3][11][11],
    float conv_core_weights[96][3][11][11], float conv_core_bias[96],
    float output1[96][55][55], float output2[96][27][27],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

int conv_pool_II(
    float input[96][27][27], float conv_core[256][96][5][5],
    float conv_core_weights[256][96][5][5], float conv_core_bias[256],
    float output1[256][27][27], float output2[256][13][13],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

int conv_pool_V(
    float input[384][13][13], float conv_core[256][384][3][3],
    float conv_core_weights[256][384][3][3], float conv_core_bias[256],
    float output1[256][13][13], float output2[256][6][6],
    volatile ap_uint<64> *dummy_data1, volatile ap_uint<64> *dummy_data2,
    volatile ap_uint<64> *dummy_data3, volatile ap_uint<64> *dummy_data4,
    volatile ap_uint<64> *dummy_data5, volatile ap_uint<64> *dummy_data6,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

int fc_bundle(float input[256][6][6], float weights_I[4096][256 * 6 * 6],
              float bias_I[4096], float weights_II[4096][4096],
              float bias_II[4096], float weights_III[1000][4096],
              float bias_III[1000], float output_I[4096], float output_II[4096],
              float output_III[1000], volatile ap_uint<64> *dummy_data1,
              volatile ap_uint<64> *dummy_data2,
              volatile ap_uint<32> dummy_data11,
              volatile ap_uint<32> dummy_data22,
              volatile ap_uint<32> dummy_data33,
              volatile ap_uint<32> dummy_data44,
              volatile ap_uint<32> dummy_data55);

// Bundle Alex Net

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
              volatile ap_uint<32> dummy_data55);

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
                volatile ap_uint<32> dummy_data55);

// 定义各种像素类型和定点数类型
typedef ap_fixed<16, 6> optimized_pixel_t; // 优化后的定点数类型
typedef ap_uint<64> frames_t;              // 输入帧的类型
typedef ap_fixed<32, 12> calc_pixel_t;     // 用于计算的像素类型
typedef ap_fixed<16, 6> vel_pixel_t; // 用于存储速度向量的像素类型

// 梯度类型
typedef struct {
  optimized_pixel_t x;
  optimized_pixel_t y;
  optimized_pixel_t z;
} gradient_t;

// 外积类型
typedef struct {
  calc_pixel_t val[6]; // 外积需要 6 个分量
} outer_t;

// 张量类型
typedef struct {
  calc_pixel_t val[6]; // 张量也包含 6 个分量
} tensor_t;

// 速度向量类型
typedef struct {
  vel_pixel_t x;
  vel_pixel_t y;
} velocity_t;

// 函数声明
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
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55);

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
    volatile ap_uint<32> dummy_data44, volatile ap_uint<32> dummy_data55);

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
    volatile ap_uint<32> dummy_data55);

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
    volatile ap_uint<32> dummy_data55);

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
    volatile ap_uint<32> dummy_data55);

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
    volatile ap_uint<32> dummy_data55);

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
                  volatile ap_uint<32> dummy_data55);

// 定义基础位宽数据类型
typedef ap_uint<8> bit8;
typedef ap_uint<16> bit16;
typedef ap_uint<32> bit32;
typedef ap_uint<1> bit1;
typedef ap_int<2> bit2;

// 定义常量
#define NUM_3D_TRI 100 // 假设有100个三角形用于处理
#define MAX_X 1920     // 最大屏幕宽度（像素）
#define MAX_Y 1080     // 最大屏幕高度（像素）
#define NUM_FB 2073600 // 假设 framebuffer 大小为 1920x1080

// 2D三角形的定义
struct Triangle_2D {
  bit8 x0, y0;
  bit8 x1, y1;
  bit8 x2, y2;
  bit8 z;
};

// 3D三角形的定义
struct Triangle_3D {
  bit8 x0, y0, z0;
  bit8 x1, y1, z1;
  bit8 x2, y2, z2;
};

// 候选像素的定义
struct CandidatePixel {
  bit8 x, y;
  bit8 z;
  bit8 color;
};

// 像素的定义
struct Pixel {
  bit8 x, y;
  bit8 color;
};

// Utility Functions
int check_clockwise(Triangle_2D triangle_2d);
void clockwise_vertices(Triangle_2D *triangle_2d);
bit1 pixel_in_triangle(bit8 x, bit8 y, Triangle_2D triangle_2d);
bit8 find_min(bit8 in0, bit8 in1, bit8 in2);
bit8 find_max(bit8 in0, bit8 in1, bit8 in2);

// Processing Stages
int bundle1_module(bit32 input[3 * NUM_3D_TRI],
                   CandidatePixel fragment2[NUM_3D_TRI][500],
                   bit16 size_fragment[NUM_3D_TRI], ap_int<64> *dummy_port_1,
                   ap_int<64> *dummy_port_2);

int zculling_module(CandidatePixel fragments[NUM_3D_TRI][500],
                    bit16 size[NUM_3D_TRI], Pixel pixels[NUM_3D_TRI][500],
                    bit16 size_pixels[NUM_3D_TRI], ap_int<64> *dummy_port_1);

int bundle2_module(bit16 size_pixels[NUM_3D_TRI], Pixel pixels[NUM_3D_TRI][500],
                   bit32 output[NUM_FB], ap_int<64> *dummy_port_1,
                   ap_int<64> *dummy_port_2);

int render3d(
    bit32 input[3 * NUM_3D_TRI], CandidatePixel fragment2[NUM_3D_TRI][500],
    bit16 size_fragment[NUM_3D_TRI], bit16 size[NUM_3D_TRI],
    Pixel pixels[NUM_3D_TRI][500], bit16 size_pixels[NUM_3D_TRI],
    bit32 output[NUM_FB], bit8 z_buffer[MAX_X][MAX_Y],
    bit8 frame_buffer[MAX_X][MAX_Y], volatile ap_uint<64> *dummy_data1,
    volatile ap_uint<64> *dummy_data2, volatile ap_uint<64> *dummy_data3,
    volatile ap_uint<32> dummy_data11, volatile ap_uint<32> dummy_data22,
    volatile ap_uint<32> dummy_data33, volatile ap_uint<32> dummy_data44,
    volatile ap_uint<32> dummy_data55);

// 定义 DigitType 代表输入的单个数字，使用 ap_uint 来指定 64 位无符号整数
typedef ap_uint<64> DigitType;

// 定义 WholeDigitType 代表一个完整的输入数字，由多个 DigitType 组成
typedef ap_uint<256> WholeDigitType;

// 定义 LabelType 代表分类标签，用于 KNN 分类结果
typedef ap_uint<4> LabelType; // 假设有最多16种分类（0-15）

// 常量定义
#define K_CONST 3         // 最近邻常量 K 值
#define PAR_FACTOR 4      // 并行处理因子
#define NUM_TEST 100      // 测试集的数量
#define NUM_TRAINING 1000 // 训练集的数量