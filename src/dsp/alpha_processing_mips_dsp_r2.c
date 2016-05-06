// Copyright 2014 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Utilities for processing transparent channel.
//
// Author(s): Branimir Vasic (branimir.vasic@imgtec.com)
//            Djordje Pesut  (djordje.pesut@imgtec.com)

#include "./dsp.h"

#if defined(WEBP_USE_MIPS_DSP_R1)

static void MultARGBRow(uint32_t* const ptr, int width, int inverse) { // dsp r1 func
  int x;
  const uint32_t c_00ffffff = 0x00ffffffu;
  const uint32_t c_ff000000 = 0xff000000u;
  const uint32_t c_8000000  = 0x00800000u;
  const uint32_t c_8000080  = 0x00800080u;
  for (x = 0; x < width; ++x) {
    const uint32_t argb = ptr[x];
    if (argb < 0xff000000u) {      // alpha < 255
      if (argb <= 0x00ffffffu) {   // alpha == 0
        ptr[x] = 0;
      } else {
        int temp0, temp1, temp2, temp3, alpha;
        __asm__ volatile (
          "srl          %[alpha],   %[argb],       24                \n\t"
          "replv.qb     %[temp0],   %[alpha]                         \n\t"
          "and          %[temp0],   %[temp0],      %[c_00ffffff]     \n\t"
          "beqz         %[inverse], 0f                               \n\t"
          "divu         $zero,      %[c_ff000000], %[alpha]          \n\t"
          "mflo         %[temp0]                                     \n\t"
        "0:                                                          \n\t"
          "andi         %[temp1],   %[argb],       0xff              \n\t"
          "ext          %[temp2],   %[argb],       8,             8  \n\t"
          "ext          %[temp3],   %[argb],       16,            8  \n\t"
          "mul          %[temp1],   %[temp1],      %[temp0]          \n\t"
          "mul          %[temp2],   %[temp2],      %[temp0]          \n\t"
          "mul          %[temp3],   %[temp3],      %[temp0]          \n\t"
          "precrq.ph.w  %[temp1],   %[temp2],      %[temp1]          \n\t"
          "addu         %[temp3],   %[temp3],      %[c_8000000]      \n\t"
          "addu         %[temp1],   %[temp1],      %[c_8000080]      \n\t"
          "precrq.ph.w  %[temp3],   %[argb],       %[temp3]          \n\t"
          "precrq.qb.ph %[temp1],   %[temp3],      %[temp1]          \n\t"
          : [temp0]"=&r"(temp0), [temp1]"=&r"(temp1), [temp2]"=&r"(temp2),
            [temp3]"=&r"(temp3), [alpha]"=&r"(alpha)
          : [inverse]"r"(inverse), [c_00ffffff]"r"(c_00ffffff),
            [c_8000000]"r"(c_8000000), [c_8000080]"r"(c_8000080),
            [c_ff000000]"r"(c_ff000000), [argb]"r"(argb)
          : "memory", "hi", "lo"
        );
        ptr[x] = temp1;
      }
    }
  }
}

//------------------------------------------------------------------------------
// Entry point

extern void WebPInitAlphaProcessingMIPSdsp(void);

void WebPInitAlphaProcessingMIPSdsp(void) {
  WebPMultARGBRow = MultARGBRow;
}

#endif  // WEBP_USE_MIPS_DSP_R1
