// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/instruction-selector-impl.h"
#include "src/compiler/node-matchers.h"
#include "src/compiler/node-properties-inl.h"

namespace v8 {
namespace internal {
namespace compiler {

// Adds IA32-specific methods for generating operands.
class IA32OperandGenerator FINAL : public OperandGenerator {
 public:
  explicit IA32OperandGenerator(InstructionSelector* selector)
      : OperandGenerator(selector) {}

  InstructionOperand* UseByteRegister(Node* node) {
    // TODO(dcarney): relax constraint.
    return UseFixed(node, edx);
  }

  bool CanBeImmediate(Node* node) {
    switch (node->opcode()) {
      case IrOpcode::kInt32Constant:
      case IrOpcode::kNumberConstant:
      case IrOpcode::kExternalConstant:
        return true;
      case IrOpcode::kHeapConstant: {
        // Constants in new space cannot be used as immediates in V8 because
        // the GC does not scan code objects when collecting the new generation.
        Unique<HeapObject> value = OpParameter<Unique<HeapObject> >(node);
        return !isolate()->heap()->InNewSpace(*value.handle());
      }
      default:
        return false;
    }
  }
};


void InstructionSelector::VisitLoad(Node* node) {
  MachineType rep = RepresentationOf(OpParameter<LoadRepresentation>(node));
  MachineType typ = TypeOf(OpParameter<LoadRepresentation>(node));
  IA32OperandGenerator g(this);
  Node* base = node->InputAt(0);
  Node* index = node->InputAt(1);

  ArchOpcode opcode;
  // TODO(titzer): signed/unsigned small loads
  switch (rep) {
    case kRepFloat32:
      opcode = kIA32Movss;
      break;
    case kRepFloat64:
      opcode = kIA32Movsd;
      break;
    case kRepBit:  // Fall through.
    case kRepWord8:
      opcode = typ == kTypeInt32 ? kIA32Movsxbl : kIA32Movzxbl;
      break;
    case kRepWord16:
      opcode = typ == kTypeInt32 ? kIA32Movsxwl : kIA32Movzxwl;
      break;
    case kRepTagged:  // Fall through.
    case kRepWord32:
      opcode = kIA32Movl;
      break;
    default:
      UNREACHABLE();
      return;
  }
  if (g.CanBeImmediate(base)) {
    if (Int32Matcher(index).Is(0)) {  // load [#base + #0]
      Emit(opcode | AddressingModeField::encode(kMode_MI),
           g.DefineAsRegister(node), g.UseImmediate(base));
    } else {  // load [#base + %index]
      Emit(opcode | AddressingModeField::encode(kMode_MRI),
           g.DefineAsRegister(node), g.UseRegister(index),
           g.UseImmediate(base));
    }
  } else if (g.CanBeImmediate(index)) {  // load [%base + #index]
    Emit(opcode | AddressingModeField::encode(kMode_MRI),
         g.DefineAsRegister(node), g.UseRegister(base), g.UseImmediate(index));
  } else {  // load [%base + %index + K]
    Emit(opcode | AddressingModeField::encode(kMode_MR1I),
         g.DefineAsRegister(node), g.UseRegister(base), g.UseRegister(index));
  }
  // TODO(turbofan): addressing modes [r+r*{2,4,8}+K]
}


void InstructionSelector::VisitStore(Node* node) {
  IA32OperandGenerator g(this);
  Node* base = node->InputAt(0);
  Node* index = node->InputAt(1);
  Node* value = node->InputAt(2);

  StoreRepresentation store_rep = OpParameter<StoreRepresentation>(node);
  MachineType rep = RepresentationOf(store_rep.machine_type());
  if (store_rep.write_barrier_kind() == kFullWriteBarrier) {
    DCHECK_EQ(kRepTagged, rep);
    // TODO(dcarney): refactor RecordWrite function to take temp registers
    //                and pass them here instead of using fixed regs
    // TODO(dcarney): handle immediate indices.
    InstructionOperand* temps[] = {g.TempRegister(ecx), g.TempRegister(edx)};
    Emit(kIA32StoreWriteBarrier, NULL, g.UseFixed(base, ebx),
         g.UseFixed(index, ecx), g.UseFixed(value, edx), arraysize(temps),
         temps);
    return;
  }
  DCHECK_EQ(kNoWriteBarrier, store_rep.write_barrier_kind());
  InstructionOperand* val;
  if (g.CanBeImmediate(value)) {
    val = g.UseImmediate(value);
  } else if (rep == kRepWord8 || rep == kRepBit) {
    val = g.UseByteRegister(value);
  } else {
    val = g.UseRegister(value);
  }
  ArchOpcode opcode;
  switch (rep) {
    case kRepFloat32:
      opcode = kIA32Movss;
      break;
    case kRepFloat64:
      opcode = kIA32Movsd;
      break;
    case kRepBit:  // Fall through.
    case kRepWord8:
      opcode = kIA32Movb;
      break;
    case kRepWord16:
      opcode = kIA32Movw;
      break;
    case kRepTagged:  // Fall through.
    case kRepWord32:
      opcode = kIA32Movl;
      break;
    default:
      UNREACHABLE();
      return;
  }
  if (g.CanBeImmediate(base)) {
    if (Int32Matcher(index).Is(0)) {  // store [#base], %|#value
      Emit(opcode | AddressingModeField::encode(kMode_MI), NULL,
           g.UseImmediate(base), val);
    } else {  // store [#base + %index], %|#value
      Emit(opcode | AddressingModeField::encode(kMode_MRI), NULL,
           g.UseRegister(index), g.UseImmediate(base), val);
    }
  } else if (g.CanBeImmediate(index)) {  // store [%base + #index], %|#value
    Emit(opcode | AddressingModeField::encode(kMode_MRI), NULL,
         g.UseRegister(base), g.UseImmediate(index), val);
  } else {  // store [%base + %index], %|#value
    Emit(opcode | AddressingModeField::encode(kMode_MR1I), NULL,
         g.UseRegister(base), g.UseRegister(index), val);
  }
  // TODO(turbofan): addressing modes [r+r*{2,4,8}+K]
}


// Shared routine for multiple binary operations.
static void VisitBinop(InstructionSelector* selector, Node* node,
                       InstructionCode opcode, FlagsContinuation* cont) {
  IA32OperandGenerator g(selector);
  Int32BinopMatcher m(node);
  InstructionOperand* inputs[4];
  size_t input_count = 0;
  InstructionOperand* outputs[2];
  size_t output_count = 0;

  // TODO(turbofan): match complex addressing modes.
  // TODO(turbofan): if commutative, pick the non-live-in operand as the left as
  // this might be the last use and therefore its register can be reused.
  if (g.CanBeImmediate(m.right().node())) {
    inputs[input_count++] = g.Use(m.left().node());
    inputs[input_count++] = g.UseImmediate(m.right().node());
  } else {
    inputs[input_count++] = g.UseRegister(m.left().node());
    inputs[input_count++] = g.Use(m.right().node());
  }

  if (cont->IsBranch()) {
    inputs[input_count++] = g.Label(cont->true_block());
    inputs[input_count++] = g.Label(cont->false_block());
  }

  outputs[output_count++] = g.DefineSameAsFirst(node);
  if (cont->IsSet()) {
    // TODO(turbofan): Use byte register here.
    outputs[output_count++] = g.DefineAsRegister(cont->result());
  }

  DCHECK_NE(0, input_count);
  DCHECK_NE(0, output_count);
  DCHECK_GE(arraysize(inputs), input_count);
  DCHECK_GE(arraysize(outputs), output_count);

  Instruction* instr = selector->Emit(cont->Encode(opcode), output_count,
                                      outputs, input_count, inputs);
  if (cont->IsBranch()) instr->MarkAsControl();
}


// Shared routine for multiple binary operations.
static void VisitBinop(InstructionSelector* selector, Node* node,
                       InstructionCode opcode) {
  FlagsContinuation cont;
  VisitBinop(selector, node, opcode, &cont);
}


void InstructionSelector::VisitWord32And(Node* node) {
  VisitBinop(this, node, kIA32And);
}


void InstructionSelector::VisitWord32Or(Node* node) {
  VisitBinop(this, node, kIA32Or);
}


void InstructionSelector::VisitWord32Xor(Node* node) {
  IA32OperandGenerator g(this);
  Int32BinopMatcher m(node);
  if (m.right().Is(-1)) {
    Emit(kIA32Not, g.DefineSameAsFirst(node), g.Use(m.left().node()));
  } else {
    VisitBinop(this, node, kIA32Xor);
  }
}


// Shared routine for multiple shift operations.
static inline void VisitShift(InstructionSelector* selector, Node* node,
                              ArchOpcode opcode) {
  IA32OperandGenerator g(selector);
  Node* left = node->InputAt(0);
  Node* right = node->InputAt(1);

  // TODO(turbofan): assembler only supports some addressing modes for shifts.
  if (g.CanBeImmediate(right)) {
    selector->Emit(opcode, g.DefineSameAsFirst(node), g.UseRegister(left),
                   g.UseImmediate(right));
  } else {
    Int32BinopMatcher m(node);
    if (m.right().IsWord32And()) {
      Int32BinopMatcher mright(right);
      if (mright.right().Is(0x1F)) {
        right = mright.left().node();
      }
    }
    selector->Emit(opcode, g.DefineSameAsFirst(node), g.UseRegister(left),
                   g.UseFixed(right, ecx));
  }
}


void InstructionSelector::VisitWord32Shl(Node* node) {
  VisitShift(this, node, kIA32Shl);
}


void InstructionSelector::VisitWord32Shr(Node* node) {
  VisitShift(this, node, kIA32Shr);
}


void InstructionSelector::VisitWord32Sar(Node* node) {
  VisitShift(this, node, kIA32Sar);
}


void InstructionSelector::VisitWord32Ror(Node* node) {
  VisitShift(this, node, kIA32Ror);
}


void InstructionSelector::VisitInt32Add(Node* node) {
  VisitBinop(this, node, kIA32Add);
}


void InstructionSelector::VisitInt32Sub(Node* node) {
  IA32OperandGenerator g(this);
  Int32BinopMatcher m(node);
  if (m.left().Is(0)) {
    Emit(kIA32Neg, g.DefineSameAsFirst(node), g.Use(m.right().node()));
  } else {
    VisitBinop(this, node, kIA32Sub);
  }
}


void InstructionSelector::VisitInt32Mul(Node* node) {
  IA32OperandGenerator g(this);
  Node* left = node->InputAt(0);
  Node* right = node->InputAt(1);
  if (g.CanBeImmediate(right)) {
    Emit(kIA32Imul, g.DefineAsRegister(node), g.Use(left),
         g.UseImmediate(right));
  } else if (g.CanBeImmediate(left)) {
    Emit(kIA32Imul, g.DefineAsRegister(node), g.Use(right),
         g.UseImmediate(left));
  } else {
    // TODO(turbofan): select better left operand.
    Emit(kIA32Imul, g.DefineSameAsFirst(node), g.UseRegister(left),
         g.Use(right));
  }
}


static inline void VisitDiv(InstructionSelector* selector, Node* node,
                            ArchOpcode opcode) {
  IA32OperandGenerator g(selector);
  InstructionOperand* temps[] = {g.TempRegister(edx)};
  size_t temp_count = arraysize(temps);
  selector->Emit(opcode, g.DefineAsFixed(node, eax),
                 g.UseFixed(node->InputAt(0), eax),
                 g.UseUnique(node->InputAt(1)), temp_count, temps);
}


void InstructionSelector::VisitInt32Div(Node* node) {
  VisitDiv(this, node, kIA32Idiv);
}


void InstructionSelector::VisitInt32UDiv(Node* node) {
  VisitDiv(this, node, kIA32Udiv);
}


static inline void VisitMod(InstructionSelector* selector, Node* node,
                            ArchOpcode opcode) {
  IA32OperandGenerator g(selector);
  InstructionOperand* temps[] = {g.TempRegister(eax), g.TempRegister(edx)};
  size_t temp_count = arraysize(temps);
  selector->Emit(opcode, g.DefineAsFixed(node, edx),
                 g.UseFixed(node->InputAt(0), eax),
                 g.UseUnique(node->InputAt(1)), temp_count, temps);
}


void InstructionSelector::VisitInt32Mod(Node* node) {
  VisitMod(this, node, kIA32Idiv);
}


void InstructionSelector::VisitInt32UMod(Node* node) {
  VisitMod(this, node, kIA32Udiv);
}


void InstructionSelector::VisitChangeInt32ToFloat64(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEInt32ToFloat64, g.DefineAsRegister(node), g.Use(node->InputAt(0)));
}


void InstructionSelector::VisitChangeUint32ToFloat64(Node* node) {
  IA32OperandGenerator g(this);
  // TODO(turbofan): IA32 SSE LoadUint32() should take an operand.
  Emit(kSSEUint32ToFloat64, g.DefineAsRegister(node),
       g.UseRegister(node->InputAt(0)));
}


void InstructionSelector::VisitChangeFloat64ToInt32(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64ToInt32, g.DefineAsRegister(node), g.Use(node->InputAt(0)));
}


void InstructionSelector::VisitChangeFloat64ToUint32(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64ToUint32, g.DefineAsRegister(node), g.Use(node->InputAt(0)));
}


void InstructionSelector::VisitFloat64Add(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64Add, g.DefineSameAsFirst(node),
       g.UseRegister(node->InputAt(0)), g.UseRegister(node->InputAt(1)));
}


void InstructionSelector::VisitFloat64Sub(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64Sub, g.DefineSameAsFirst(node),
       g.UseRegister(node->InputAt(0)), g.UseRegister(node->InputAt(1)));
}


void InstructionSelector::VisitFloat64Mul(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64Mul, g.DefineSameAsFirst(node),
       g.UseRegister(node->InputAt(0)), g.UseRegister(node->InputAt(1)));
}


void InstructionSelector::VisitFloat64Div(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64Div, g.DefineSameAsFirst(node),
       g.UseRegister(node->InputAt(0)), g.UseRegister(node->InputAt(1)));
}


void InstructionSelector::VisitFloat64Mod(Node* node) {
  IA32OperandGenerator g(this);
  InstructionOperand* temps[] = {g.TempRegister(eax)};
  Emit(kSSEFloat64Mod, g.DefineSameAsFirst(node),
       g.UseRegister(node->InputAt(0)), g.UseRegister(node->InputAt(1)), 1,
       temps);
}


void InstructionSelector::VisitFloat64Sqrt(Node* node) {
  IA32OperandGenerator g(this);
  Emit(kSSEFloat64Sqrt, g.DefineAsRegister(node), g.Use(node->InputAt(0)));
}


void InstructionSelector::VisitInt32AddWithOverflow(Node* node,
                                                    FlagsContinuation* cont) {
  VisitBinop(this, node, kIA32Add, cont);
}


void InstructionSelector::VisitInt32SubWithOverflow(Node* node,
                                                    FlagsContinuation* cont) {
  VisitBinop(this, node, kIA32Sub, cont);
}


// Shared routine for multiple compare operations.
static inline void VisitCompare(InstructionSelector* selector,
                                InstructionCode opcode,
                                InstructionOperand* left,
                                InstructionOperand* right,
                                FlagsContinuation* cont) {
  IA32OperandGenerator g(selector);
  if (cont->IsBranch()) {
    selector->Emit(cont->Encode(opcode), NULL, left, right,
                   g.Label(cont->true_block()),
                   g.Label(cont->false_block()))->MarkAsControl();
  } else {
    DCHECK(cont->IsSet());
    // TODO(titzer): Needs byte register.
    selector->Emit(cont->Encode(opcode), g.DefineAsRegister(cont->result()),
                   left, right);
  }
}


// Shared routine for multiple word compare operations.
static inline void VisitWordCompare(InstructionSelector* selector, Node* node,
                                    InstructionCode opcode,
                                    FlagsContinuation* cont, bool commutative) {
  IA32OperandGenerator g(selector);
  Node* left = node->InputAt(0);
  Node* right = node->InputAt(1);

  // Match immediates on left or right side of comparison.
  if (g.CanBeImmediate(right)) {
    VisitCompare(selector, opcode, g.Use(left), g.UseImmediate(right), cont);
  } else if (g.CanBeImmediate(left)) {
    if (!commutative) cont->Commute();
    VisitCompare(selector, opcode, g.Use(right), g.UseImmediate(left), cont);
  } else {
    VisitCompare(selector, opcode, g.UseRegister(left), g.Use(right), cont);
  }
}


void InstructionSelector::VisitWord32Test(Node* node, FlagsContinuation* cont) {
  switch (node->opcode()) {
    case IrOpcode::kInt32Sub:
      return VisitWordCompare(this, node, kIA32Cmp, cont, false);
    case IrOpcode::kWord32And:
      return VisitWordCompare(this, node, kIA32Test, cont, true);
    default:
      break;
  }

  IA32OperandGenerator g(this);
  VisitCompare(this, kIA32Test, g.Use(node), g.TempImmediate(-1), cont);
}


void InstructionSelector::VisitWord32Compare(Node* node,
                                             FlagsContinuation* cont) {
  VisitWordCompare(this, node, kIA32Cmp, cont, false);
}


void InstructionSelector::VisitFloat64Compare(Node* node,
                                              FlagsContinuation* cont) {
  IA32OperandGenerator g(this);
  Node* left = node->InputAt(0);
  Node* right = node->InputAt(1);
  VisitCompare(this, kSSEFloat64Cmp, g.UseRegister(left), g.Use(right), cont);
}


void InstructionSelector::VisitCall(Node* call, BasicBlock* continuation,
                                    BasicBlock* deoptimization) {
  IA32OperandGenerator g(this);
  CallDescriptor* descriptor = OpParameter<CallDescriptor*>(call);

  FrameStateDescriptor* frame_state_descriptor = NULL;

  if (descriptor->NeedsFrameState()) {
    frame_state_descriptor =
        GetFrameStateDescriptor(call->InputAt(descriptor->InputCount()));
  }

  CallBuffer buffer(zone(), descriptor, frame_state_descriptor);

  // Compute InstructionOperands for inputs and outputs.
  InitializeCallBuffer(call, &buffer, true, true);

  // Push any stack arguments.
  for (NodeVectorRIter input = buffer.pushed_nodes.rbegin();
       input != buffer.pushed_nodes.rend(); input++) {
    // TODO(titzer): handle pushing double parameters.
    Emit(kIA32Push, NULL,
         g.CanBeImmediate(*input) ? g.UseImmediate(*input) : g.Use(*input));
  }

  // Select the appropriate opcode based on the call type.
  InstructionCode opcode;
  switch (descriptor->kind()) {
    case CallDescriptor::kCallCodeObject: {
      opcode = kArchCallCodeObject;
      break;
    }
    case CallDescriptor::kCallJSFunction:
      opcode = kArchCallJSFunction;
      break;
    default:
      UNREACHABLE();
      return;
  }
  opcode |= MiscField::encode(descriptor->flags());

  // Emit the call instruction.
  Instruction* call_instr =
      Emit(opcode, buffer.outputs.size(), &buffer.outputs.front(),
           buffer.instruction_args.size(), &buffer.instruction_args.front());

  call_instr->MarkAsCall();
  if (deoptimization != NULL) {
    DCHECK(continuation != NULL);
    call_instr->MarkAsControl();
  }
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
