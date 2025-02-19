//===------------ VECustomDAG.h - VE Custom DAG Nodes -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the helper functions that VE uses to lower LLVM code into a
// selection DAG.  For example, hiding SDLoc, and easy to use SDNodeFlags.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_VE_VECUSTOMDAG_H
#define LLVM_LIB_TARGET_VE_VECUSTOMDAG_H

#include "VE.h"
#include "VEISelLowering.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

Optional<unsigned> getVVPOpcode(unsigned Opcode);

bool isVVPBinaryOp(unsigned Opcode);

bool isPackedVectorType(EVT SomeVT);

bool isMaskType(EVT SomeVT);

bool isMaskArithmetic(SDValue Op);

bool isVVPOrVEC(unsigned);

bool maySafelyIgnoreMask(SDValue Op);

/// The VE backend uses a two-staged process to lower and legalize vector
/// instructions:
//
/// 1. VP and standard vector SDNodes are lowered to SDNodes of the VVP_* layer.
//
//     All VVP nodes have a mask and an Active Vector Length (AVL) parameter.
//     The AVL parameters refers to the element position in the vector the VVP
//     node operates on.
//
//
//  2. The VVP SDNodes are legalized. The AVL in a legal VVP node refers to
//     chunks of 64bit. We track this by wrapping the AVL in a LEGALAVL node.
//
//     The AVL mechanism in the VE architecture always refers to chunks of
//     64bit, regardless of the actual element type vector instructions are
//     operating on. For vector types v256.32 or v256.64 nothing needs to be
//     legalized since each element occupies a 64bit chunk - there is no
//     difference between counting 64bit chunks or element positions. However,
//     all vector types with > 256 elements store more than one logical element
//     per 64bit chunk and need to be transformed.
//     However legalization is performed, the resulting legal VVP SDNodes will
//     have a LEGALAVL node as their AVL operand. The LEGALAVL nodes wraps
//     around an AVL that refers to 64 bit chunks just as the architecture
//     demands - that is, the wrapped AVL is the correct setting for the VL
//     register for this VVP operation to get the desired behavior.
//
/// AVL Functions {
// The AVL operand position of this node.
Optional<int> getAVLPos(unsigned);

// Whether this is a LEGALAVL node.
bool isLegalAVL(SDValue AVL);

// The AVL operand of this node.
SDValue getNodeAVL(SDValue);

// Return the AVL operand of this node. If it is a LEGALAVL node, unwrap it.
// Return with the boolean whether unwrapping happened.
std::pair<SDValue, bool> getAnnotatedNodeAVL(SDValue);

/// } AVL Functions

enum class Packing {
  Normal = 0, // 256 element standard mode.
  Dense = 1   // 512 element packed mode.
};

// Get the vector or mask register type for this packing and element type.
MVT getLegalVectorType(Packing P, MVT ElemVT);

// Whether this type belongs to a packed mask or vector register.
Packing getTypePacking(EVT);

enum class PackElem : int8_t {
  Lo = 0, // Integer (63, 32]
  Hi = 1  // Float   (32,  0]
};

class VECustomDAG {
  SelectionDAG &DAG;
  SDLoc DL;

public:
  SelectionDAG *getDAG() const { return &DAG; }

  VECustomDAG(SelectionDAG &DAG, SDLoc DL) : DAG(DAG), DL(DL) {}

  VECustomDAG(SelectionDAG &DAG, SDValue WhereOp) : DAG(DAG), DL(WhereOp) {}

  VECustomDAG(SelectionDAG &DAG, const SDNode *WhereN) : DAG(DAG), DL(WhereN) {}

  /// getNode {
  SDValue getNode(unsigned OC, SDVTList VTL, ArrayRef<SDValue> OpV,
                  Optional<SDNodeFlags> Flags = None) const {
    auto N = DAG.getNode(OC, DL, VTL, OpV);
    if (Flags)
      N->setFlags(*Flags);
    return N;
  }

  SDValue getNode(unsigned OC, ArrayRef<EVT> ResVT, ArrayRef<SDValue> OpV,
                  Optional<SDNodeFlags> Flags = None) const {
    auto N = DAG.getNode(OC, DL, ResVT, OpV);
    if (Flags)
      N->setFlags(*Flags);
    return N;
  }

  SDValue getNode(unsigned OC, EVT ResVT, ArrayRef<SDValue> OpV,
                  Optional<SDNodeFlags> Flags = None) const {
    auto N = DAG.getNode(OC, DL, ResVT, OpV);
    if (Flags)
      N->setFlags(*Flags);
    return N;
  }

  SDValue getUNDEF(EVT VT) const { return DAG.getUNDEF(VT); }
  /// } getNode

  /// Packing {
  SDValue getUnpack(EVT DestVT, SDValue Vec, PackElem Part, SDValue AVL);
  SDValue getPack(EVT DestVT, SDValue LoVec, SDValue HiVec, SDValue AVL);
  /// } Packing

  SDValue getConstant(uint64_t Val, EVT VT, bool IsTarget = false,
                      bool IsOpaque = false) const;

  SDValue getConstantMask(Packing Packing, bool AllTrue) const;
  SDValue getMaskBroadcast(EVT ResultVT, SDValue Scalar, SDValue AVL) const;
  SDValue getBroadcast(EVT ResultVT, SDValue Scalar, SDValue AVL) const;

  // Wrap AVL in a LEGALAVL node (unless it is one already).
  SDValue annotateLegalAVL(SDValue AVL) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_VE_VECUSTOMDAG_H
