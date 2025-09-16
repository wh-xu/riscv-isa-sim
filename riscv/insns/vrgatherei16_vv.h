// vrgatherei16.vv vd, vs2, vs1, vm # vd[i] = (vs1[i] >= VLMAX) ? 0 : vs2[vs1[i]];
// Project: RVV-LUT Acceleration
// Author(s): Weihong Xu <weihong.xu@epfl.ch>
// Description: Blocked permutation with bit packing

float vemul = (16.0 / P.VU.vsew * P.VU.vflmul);
require(vemul >= 0.125 && vemul <= 8);
require_align(insn.rd(), P.VU.vflmul);
require_align(insn.rs2(), P.VU.vflmul);
require_align(insn.rs1(), vemul);
require_noover(insn.rd(), P.VU.vflmul, insn.rs1(), vemul);
require(insn.rd() != insn.rs2());
require_vm;

// Prepare for blocked permutation
// Offset value for blocked permutation
uint16_t block_size = 2 << P.VU.vlut;
uint16_t num_packs = 16 / (P.VU.vlut + 1);

// Mask and shift for bit packing
uint16_t vs1_unpacked, vs1_offset_idx;
uint16_t MASK = 0xFFFF;
uint16_t MASK_SHIFT = 0;

if(P.VU.vlut_pack) {
  MASK_SHIFT = P.VU.vlut+1;
}

switch(P.VU.vlut) {
  case 1:
    MASK = 0x3;
    break;
  case 2:
    MASK = 0x7;
    break;
  case 3:
    MASK = 0xF;
    break;
  case 4:
    MASK = 0x1F;
    break;
  case 5:
    MASK = 0x3F;
    break;
  case 6:
    MASK = 0x7F;
    break;
  case 7:
    MASK = 0xFF;
    break;
}

// printf("num_packs: %d\n", num_packs);
// printf("MASK: %x\n", MASK);
// printf("MASK_SHIFT: %d\n", MASK_SHIFT);
// printf("block_size: %d\n", block_size);

VI_LOOP_BASE
  switch (sew) {
  case e8: {
    auto vs1 = P.VU.elt<uint16_t>(rs1_num, i);
    P.VU.elt<uint8_t>(rd_num, i, true) = vs1 >= P.VU.vlmax ? 0 : P.VU.elt<uint8_t>(rs2_num, vs1);
    break;
  }
  case e16: {
    auto vs1 = P.VU.elt<uint16_t>(rs1_num, i);
    if(P.VU.vlut > 0) {
      if(P.VU.vlut_pack) {
        // Blocked permutation with bit packing
        for(int j = 0; j < num_packs; j++) {
          vs1_unpacked = (vs1 >> (MASK_SHIFT*j)) & MASK;
          vs1_offset_idx = vs1_unpacked + (i - (i % block_size));
          P.VU.elt<uint16_t>(rd_num+j, i, true) = vs1_offset_idx >= P.VU.vlmax ? 0 : P.VU.elt<uint16_t>(rs2_num, vs1_offset_idx);
        }
      } else {
        // Blocked permutation without bit packing
        vs1 = vs1 & MASK;
        vs1 += (i - (i % block_size));
        P.VU.elt<uint16_t>(rd_num, i, true) = vs1 >= P.VU.vlmax ? 0 : P.VU.elt<uint16_t>(rs2_num, vs1);
      }
    } else {
      // Generic permutation
      P.VU.elt<uint16_t>(rd_num, i, true) = vs1 >= P.VU.vlmax ? 0 : P.VU.elt<uint16_t>(rs2_num, vs1);
    }

    break;
  }
  case e32: {
    auto vs1 = P.VU.elt<uint16_t>(rs1_num, i);
    P.VU.elt<uint32_t>(rd_num, i, true) = vs1 >= P.VU.vlmax ? 0 : P.VU.elt<uint32_t>(rs2_num, vs1);
    break;
  }
  default: {
    auto vs1 = P.VU.elt<uint16_t>(rs1_num, i);
    P.VU.elt<uint64_t>(rd_num, i, true) = vs1 >= P.VU.vlmax ? 0 : P.VU.elt<uint64_t>(rs2_num, vs1);
    break;
  }
  }
VI_LOOP_END;
