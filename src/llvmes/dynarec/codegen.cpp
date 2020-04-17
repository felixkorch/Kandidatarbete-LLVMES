#include "llvmes/dynarec/compiler.h"

namespace llvmes {

void Compiler::CodeGen(Instruction& i)
{
    switch (i.opcode) {
        case 0xD0: {  // BNE Immediate
            llvm::Value* load_z = c->builder.CreateLoad(c->status_z);
            llvm::Value* is_nonzero =
                c->builder.CreateICmpNE(load_z, GetConstant1(1), "ne");
            CreateCondBranch(is_nonzero, c->basicblocks[i.target_label]);
            break;
        }
        case 0xF0: {  // BEQ Immediate
            llvm::Value* load_z = c->builder.CreateLoad(c->status_z);
            llvm::Value* is_zero =
                c->builder.CreateICmpEQ(load_z, GetConstant1(1), "eq");
            CreateCondBranch(is_zero, c->basicblocks[i.target_label]);
            break;
        }
        case 0x30: {  // BMI Immediate
            llvm::Value* load_n = c->builder.CreateLoad(c->status_n);
            llvm::Value* is_negative =
                c->builder.CreateICmpEQ(load_n, GetConstant1(1), "eq");
            CreateCondBranch(is_negative, c->basicblocks[i.target_label]);
            break;
        }
        case 0x90: {  // BCC Immediate
            llvm::Value* load_c = c->builder.CreateLoad(c->status_c);
            llvm::Value* is_carry_clear =
                c->builder.CreateICmpEQ(load_c, GetConstant1(0), "eq");
            CreateCondBranch(is_carry_clear, c->basicblocks[i.target_label]);
            break;
        }
        case 0xB0: {  // BCS Immediate
            llvm::Value* load_c = c->builder.CreateLoad(c->status_c);
            llvm::Value* is_carry_set =
                c->builder.CreateICmpEQ(load_c, GetConstant1(1), "eq");
            CreateCondBranch(is_carry_set, c->basicblocks[i.target_label]);
            break;
        }
        case 0x10: {  // BPL Immediate
            llvm::Value* load_n = c->builder.CreateLoad(c->status_n);
            llvm::Value* is_positive =
                c->builder.CreateICmpEQ(load_n, GetConstant1(0), "eq");
            CreateCondBranch(is_positive, c->basicblocks[i.target_label]);
            break;
        }
        case 0x50: {  // BVC Immediate
            llvm::Value* load_v = c->builder.CreateLoad(c->status_v);
            llvm::Value* is_overflow_clear =
                c->builder.CreateICmpEQ(load_v, GetConstant1(0), "eq");
            CreateCondBranch(is_overflow_clear, c->basicblocks[i.target_label]);
            break;
        }
        case 0x70: {  // BVS Immediate
            llvm::Value* load_v = c->builder.CreateLoad(c->status_v);
            llvm::Value* is_overflow_set =
                c->builder.CreateICmpEQ(load_v, GetConstant1(1), "eq");
            CreateCondBranch(is_overflow_set, c->basicblocks[i.target_label]);
            break;
        }
        case 0xE8: {  // INX Implied
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* inx = c->builder.CreateAdd(load_x, GetConstant8(1));
            c->builder.CreateStore(inx, c->reg_x);
            DynamicTestZ(inx);
            DynamicTestN(inx);
            break;
        }
        case 0xC8: {  // INY Implied
            break;
        }
        case 0x88: {  // DEY Implied
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* dey = c->builder.CreateSub(load_y, GetConstant8(1));
            c->builder.CreateStore(dey, c->reg_y);
            DynamicTestZ(dey);
            DynamicTestN(dey);
            break;
        }
        case 0xCA: {  // DEX Implied
            break;
        }
        case 0xE6: {  // INC Zeropage
            break;
        }
        case 0xF6: {  // INC ZeropageX
            break;
        }
        case 0xEE: {  // INC Absolute
            break;
        }
        case 0xFE: {  // INC AbsoluteX
            break;
        }
        case 0x4C: {  // JMP Absolute
            c->builder.CreateBr(c->basicblocks[i.target_label]);
            break;
        }
        case 0x6C: {  // JMP Indirect
            break;
        }
        case 0x20: {  // JSR Absolute
            break;
        }
        case 0x24: {  // BIT Zeropage
            llvm::Value* load_zero_page = ReadMemory(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, load_zero_page);
            // Set Z to zero if result is zero
            DynamicTestZ(result);
            // Set V to Bit 6 of Memory value - I'm not sure if this way of
            // calculating the V-flag is unique for BIT or if it should be
            // abstracted into a function
            llvm::Constant* c_0x40 = llvm::ConstantInt::get(int8, 0x40);
            llvm::Value* do_and_v =
                c->builder.CreateAnd(load_zero_page, c_0x40);
            llvm::Value* is_overflow =
                c->builder.CreateICmpEQ(do_and_v, c_0x40);
            c->builder.CreateStore(is_overflow, c->status_v);
            // Set N to Bit 7 of Memory value
            DynamicTestN(load_zero_page);
            break;
        }
        case 0x2C: {  // BIT Absolute
            llvm::Value* load_addr = ReadMemory(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, load_addr);
            // Set Z to zero if result is zero
            DynamicTestZ(result);
            // Set V to Bit 6 of Memory value - I'm not sure if this way of
            // calculating the V-flag is unique for BIT or if it should be
            // abstracted into a function
            llvm::Constant* c_0x40 = llvm::ConstantInt::get(int8, 0x40);
            llvm::Value* do_and_v = c->builder.CreateAnd(load_addr, c_0x40);
            llvm::Value* is_overflow =
                c->builder.CreateICmpEQ(do_and_v, c_0x40);
            c->builder.CreateStore(is_overflow, c->status_v);
            // Set N to Bit 7 of Memory value
            DynamicTestN(load_addr);
            break;
        }
        case 0x00: {  // BRK Implied
            break;
        }
        case 0x69: {  // ADC Immediate
            break;
        }
        case 0xC9: {  // CMP Immediate
            // In data
            llvm::Value* operand = llvm::ConstantInt::get(int8, i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xC5: {  // CMP Zeropage
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xD5: {  // CMP ZeropageX
            // In data
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            // Get reg_a and reg_x
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            // Add data and reg_x
            llvm::Value* target = c->builder.CreateAdd(zpg_addr, reg_x);
            // Get value to compare with
            llvm::Value* target_16 = c->builder.CreateZExt(target, int16);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, target_16);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xCD: {  // CMP Absolute
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xDD: {  // CMP AbsoluteX
            // In data
            llvm::Constant* abs_addr = GetConstant16(i.arg);
            // Get reg_a and reg_x
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* reg_x_16 = c->builder.CreateZExt(reg_x, int16);
            // Add reg_x and abs_addr
            llvm::Value* target = c->builder.CreateAdd(abs_addr, reg_x_16);
            // Get mem data
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, target);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xD9: {  // CMP AbsoluteY
            // In data
            llvm::Constant* abs_addr = GetConstant16(i.arg);
            // Get reg_a and reg_y
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            llvm::Value* reg_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* reg_y_16 = c->builder.CreateZExt(reg_y, int16);
            // Add reg_y and abs_addr
            llvm::Value* target = c->builder.CreateAdd(abs_addr, reg_y_16);
            // Get mem data
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, target);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xC1: {  // CMP IndirectX
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* addr_base =
                c->builder.CreateAdd(load_x, GetConstant8(i.arg));

            // low
            llvm::Value* addr_base_16 = c->builder.CreateZExt(addr_base, int16);
            llvm::Value* addr_low =
                c->builder.CreateCall(c->read_fn, addr_base_16);
            llvm::Value* addr_low_16 = c->builder.CreateZExt(addr_low, int16);

            // high
            llvm::Value* addr_get_high =
                c->builder.CreateAdd(addr_base, GetConstant8(1));
            llvm::Value* addr_get_high_16 =
                c->builder.CreateZExt(addr_get_high, int16);
            llvm::Value* addr_high =
                c->builder.CreateCall(c->read_fn, addr_get_high_16);
            llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
            llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);

            llvm::Value* addr_hl_or =
                c->builder.CreateOr(addr_high_shl, addr_low_16);
            // reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, addr_hl_or);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xD1: {  // CMP IndirectY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* load_y_16 = c->builder.CreateZExt(load_y, int16);

            // low
            llvm::Value* addr_low =
                c->builder.CreateCall(c->read_fn, GetConstant8(i.arg));
            llvm::Value* addr_low_16 = c->builder.CreateZExt(addr_low, int16);

            // high
            llvm::Value* addr_get_high =
                c->builder.CreateAdd(GetConstant8(i.arg), GetConstant8(1));
            llvm::Value* addr_get_high_16 =
                c->builder.CreateZExt(addr_get_high, int16);
            llvm::Value* addr_high =
                c->builder.CreateCall(c->read_fn, addr_get_high);
            llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
            llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);

            llvm::Value* addr_hl_or =
                c->builder.CreateOr(addr_high_shl, addr_low_16);
            llvm::Value* addr_or_with_y =
                c->builder.CreateAdd(addr_hl_or, load_y_16);
            // reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result =
                c->builder.CreateSub(reg_a_16, addr_or_with_y);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xE0: {  // CPX Immediate
            // In data
            llvm::Value* operand = llvm::ConstantInt::get(int8, i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_x
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* reg_x_16 = c->builder.CreateZExt(reg_x, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_x_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xE4: {  // CPX Zeropage
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_x
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* reg_x_16 = c->builder.CreateZExt(reg_x, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_x_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xEC: {  // CPX Absolute
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_x
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* reg_x_16 = c->builder.CreateZExt(reg_x, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_x_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xC0: {  // CPY Immediate
            // In data
            llvm::Value* operand = llvm::ConstantInt::get(int8, i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_y
            llvm::Value* reg_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* reg_y_16 = c->builder.CreateZExt(reg_y, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_y_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xC4: {  // CPY Zeropage
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_y
            llvm::Value* reg_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* reg_y_16 = c->builder.CreateZExt(reg_y, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_y_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xCC: {  // CPY Absolute
            // In data
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* operand_16 = c->builder.CreateZExt(operand, int16);
            // Get reg_y
            llvm::Value* reg_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* reg_y_16 = c->builder.CreateZExt(reg_y, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_y_16, operand_16);
            // Flag Test
            DynamicTestZ16(result);
            DynamicTestN16(result);
            DynamicTestCCmp(result);
            break;
        }
        case 0xC6: {  // DEC Zeropage
            break;
        }
        case 0xD6: {  // DEC ZeropageX
            break;
        }
        case 0xCE: {  // DEC Absolute
            break;
        }
        case 0xDE: {  // DEC AbsoluteX
            break;
        }
        case 0x49: {  // EOR Immediate
            break;
        }
        case 0x45: {  // EOR Zeropage
            break;
        }
        case 0x55: {  // EOR ZeropageX
            break;
        }
        case 0x4D: {  // EOR Absolute
            break;
        }
        case 0x5D: {  // EOR AbsoluteX
            break;
        }
        case 0x59: {  // EOR AbsoluteY
            break;
        }
        case 0x41: {  // EOR IndirectX
            break;
        }
        case 0x51: {  // EOR IndirectY
            break;
        }
        case 0xA9: {  // LDA Immediate
            int arg = i.arg;
            llvm::Value* a = llvm::ConstantInt::get(int8, arg);
            c->builder.CreateStore(a, c->reg_a);
            StaticTestZ(arg);
            StaticTestN(arg);
            break;
        }
        case 0xA5: {  // LDA Zeropage
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_a);
            break;
        }
        case 0xB5: {  // LDA ZeropageX
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            llvm::Value* target_addr = c->builder.CreateAdd(load_x, zpg_addr);
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr_16);
            c->builder.CreateStore(answer, c->reg_a);
            break;
        }
        case 0xA1: {  // LDA IndirectX
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* addr_base =
                c->builder.CreateAdd(load_x, GetConstant8(i.arg));
            llvm::Value* addr_low =
                c->builder.CreateCall(c->read_fn, addr_base);
            llvm::Value* addr_get_high =
                c->builder.CreateAdd(addr_base, GetConstant1(1));
            llvm::Value* addr_high =
                c->builder.CreateCall(c->read_fn, addr_get_high);
            llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
            llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);
            llvm::Value* addr_hl_or =
                c->builder.CreateOr(addr_high_shl, addr_low);
            llvm::Value* answer = c->builder.CreateCall(c->read_fn, addr_hl_or);
            c->builder.CreateStore(answer, c->reg_a);
            break;
        }
        case 0xB1: {  // LDA IndirectY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Constant* addr = GetConstant8(i.arg);
            llvm::Value* target_addr = c->builder.CreateAdd(load_y, addr);
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr_16);
            c->builder.CreateStore(answer, c->reg_a);
            break;
        }
        case 0xAD: {  // LDA Absolute
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_a);
            break;
        }
        case 0xBD: {  // LDA AbsoluteX
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_x, int16);
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* target_addr =
                c->builder.CreateAdd(target_addr_16, addr);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr);
            c->builder.CreateStore(answer, c->reg_a);
            break;
        }
        case 0xB9: {  // LDA AbsoluteY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_y, int16);
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* target_addr =
                c->builder.CreateAdd(target_addr_16, addr);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr);
            c->builder.CreateStore(answer, c->reg_a);
            break;
        }
        case 0xA2: {  // LDX Immediate
            int arg = i.arg;
            llvm::Value* x = llvm::ConstantInt::get(int8, arg);
            c->builder.CreateStore(x, c->reg_x);
            StaticTestZ(arg);
            StaticTestN(arg);
            break;
        }
        case 0xA6: {  // LDX Zeropage
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_x);
            break;
        }
        case 0xB6: {  // LDX ZeropageY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            llvm::Value* target_addr = c->builder.CreateAdd(load_y, zpg_addr);
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr_16);
            c->builder.CreateStore(answer, c->reg_x);
            break;
        }
        case 0xAE: {  // LDX Absolute
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_x);
            break;
        }
        case 0xBE: {  // LDX AbsoluteY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_y, int16);
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* target_addr =
                c->builder.CreateAdd(target_addr_16, addr);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr);
            c->builder.CreateStore(answer, c->reg_x);
            break;
        }
        case 0xA0: {  // LDY Immediate
            uint8_t arg = i.arg;
            c->builder.CreateStore(GetConstant8(arg), c->reg_y);
            StaticTestZ(arg);
            StaticTestN(arg);
            break;
        }
        case 0xA4: {  // LDY Zeropage
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_y);
            break;
        }
        case 0xB4: {  // LDY ZeropageX
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            // Loads the X register into a placeholder
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            // Adds the X register to the RAM pointer
            llvm::Value* target_addr = c->builder.CreateAdd(zpg_addr, load_x);
            // Makes the address a 16 bit by adding 8 zeros
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            // create call to read function that returns
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr_16);
            c->builder.CreateStore(answer, c->reg_y);
            break;
        }
        case 0xAC: {  // LDY Absolute
            c->builder.CreateStore(ReadMemory(i.arg), c->reg_y);
            break;
        }
        case 0xBC: {  // LDY AbsoluteX
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_x, int16);
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* target_addr =
                c->builder.CreateAdd(target_addr_16, addr);
            llvm::Value* answer =
                c->builder.CreateCall(c->read_fn, target_addr);
            c->builder.CreateStore(answer, c->reg_y);
            break;
        }
        case 0x4A: {  // ACC Accumulator
            break;
        }
        case 0x46: {  // LSR Zeropage
            break;
        }
        case 0x56: {  // LSR ZeropageX
            break;
        }
        case 0x4E: {  // LSR Absolute
            break;
        }
        case 0x5E: {  // LSR AbsoluteX
            break;
        }
        case 0x09: {  // ORA Immediate
            // Fetch operands
            llvm::Value* operand = AddressModeImmediate(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x05: {  // ORA Zeropage
            // Fetch operands
            llvm::Value* target_addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x15: {  // ORA ZeropageX
            // Fetch operands
            llvm::Value* target_addr = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x0D: {  // ORA Absolute
            // Fetch operands
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x1D: {  // ORA AbsoluteX
            // Fetch operands
            llvm::Value* target_addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x19: {  // ORA AbsoluteY
            // Fetch operands
            llvm::Value* target_addr = AddressModeAbsoluteY(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x01: {  // ORA IndirectX
            // Fetch operands
            llvm::Value* target_addr = AddressModeIndirectX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x11: {  // ORA IndirectY
            // Fetch operands
            llvm::Value* target_addr = AddressModeIndirectY(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            // Compure OR between operands
            llvm::Value* result = c->builder.CreateOr(operand, load_a);
            // Set affected CPU flags
            DynamicTestN(result);
            DynamicTestZ(result);
            // Store result in accumulator
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x48: {  // PHA Implied
            break;
        }
        case 0x08: {  // PHP Implied
            break;
        }
        case 0x68: {  // PLA Implied
            break;
        }
        case 0x28: {  // PLP Implied
            break;
        }
        case 0x2A: {  // ACC Accumulator
            break;
        }
        case 0x26: {  // ROL Zeropage
            break;
        }
        case 0x36: {  // ROL ZeropageX
            break;
        }
        case 0x2E: {  // ROL Absolute
            break;
        }
        case 0x3E: {  // ROL AbsoluteX
            break;
        }
        case 0x6A: {  // ACC Accumulator
            break;
        }
        case 0x66: {  // ROR Zeropage
            break;
        }
        case 0x76: {  // ROR ZeropageX
            break;
        }
        case 0x6E: {  // ROR Absolute
            break;
        }
        case 0x7E: {  // ROR AbsoluteX
            break;
        }
        case 0x40: {  // RTI Implied
            break;
        }
        case 0x60: {  // RTS Implied
            break;
        }
        case 0xE9: {  // SBC Immediate
            break;
        }
        case 0xE5: {  // SBC Zeropage
            break;
        }
        case 0xF5: {  // SBC ZeropageX
            break;
        }
        case 0xED: {  // SBC Absolute
            break;
        }
        case 0xFD: {  // SBC AbsoluteX
            break;
        }
        case 0xF9: {  // SBC AbsoluteY
            break;
        }
        case 0xE1: {  // SBC IndirectX
            break;
        }
        case 0xF1: {  // SBC IndirectY
            break;
        }
        case 0x38: {  // SEC Implied
            break;
        }
        case 0xF8: {  // SED Implied
            // Since our abstract 6502 does not implement a dedicated decimal mode
            // but instead handles decimal on the fly all the time, there exists no
            // decimal flag. Therefore there is no need to ever set or clear the
            // decimal flag and those instructions are instead replaced with nothing
            // when compiled
            break;
        }
        case 0x78: {  // SEI Implied
            c->builder.CreateStore(GetConstant1(1), c->status_i);
            break;
        }
        case 0x18: {  // CLC Implied
            break;
        }
        case 0xD8: {  // CLD Implied
            // Since our abstract 6502 does not implement a dedicated decimal mode
            // but instead handles decimal on the fly all the time, there exists no
            // decimal flag. Therefore there is no need to ever set or clear the
            // decimal flag and those instructions are instead replaced with nothing
            // when compiled
            break;
        }
        case 0x58: {  // CLI Implied
            c->builder.CreateStore(GetConstant1(0), c->status_i);
            break;
        }
        case 0xB8: {  // CLV Implied
            c->builder.CreateStore(GetConstant1(0), c->status_v);
            break;
        }
        case 0x85: {  // STA Zeropage
            break;
        }
        case 0x95: {  // STA ZeropageX
            break;
        }
        case 0x8D: {  // STA Absolute
            uint16_t addr = i.arg;
            // Write to '0x2008' and 'A' will be written to stdout as char
            if (addr == 0x2008) {
                llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                c->builder.CreateCall(c->putchar_fn, {load_a});
            }
            // Write A to stdout
            else if (addr == 0x2009) {
                llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                c->builder.CreateCall(c->putreg_fn, {load_a});
            }
            // Write X to stdout
            else if (addr == 0x200A) {
                llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
                c->builder.CreateCall(c->putreg_fn, {load_x});
            }
            // Write Y to stdout
            else if (addr == 0x200B) {
                llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
                c->builder.CreateCall(c->putreg_fn, {load_y});
            }
            // Write N to stdout
            else if (addr == 0x200C) {
                llvm::Value* load_n = c->builder.CreateLoad(c->status_n);
                llvm::Value* n_8bit = c->builder.CreateZExt(load_n, int8);
                c->builder.CreateCall(c->putreg_fn, {n_8bit});
            }
            // Write C to stdout
            else if (addr == 0x200D) {
                llvm::Value* load_c = c->builder.CreateLoad(c->status_c);
                llvm::Value* c_8bit = c->builder.CreateZExt(load_c, int8);
                c->builder.CreateCall(c->putreg_fn, {c_8bit});
            }
            // Write Z to stdout
            else if (addr == 0x200E) {
                llvm::Value* load_z = c->builder.CreateLoad(c->status_z);
                llvm::Value* z_8bit = c->builder.CreateZExt(load_z, int8);
                c->builder.CreateCall(c->putreg_fn, {z_8bit});
            }
            // Write V to stdout
            else if (addr == 0x200F) {
                llvm::Value* load_v = c->builder.CreateLoad(c->status_v);
                llvm::Value* v_8bit = c->builder.CreateZExt(load_v, int8);
                c->builder.CreateCall(c->putreg_fn, {v_8bit});
            }
            // Store normally
            else {
                llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                WriteMemory(addr, load_a);
            }
            break;
        }
        case 0x9D: {  // STA AbsoluteX
            break;
        }
        case 0x99: {  // STA AbsoluteY
            break;
        }
        case 0x81: {  // STA IndirectX
            break;
        }
        case 0x91: {  // STA IndirectY
            break;
        }
        case 0x86: {  // STX Zeropage
            llvm::Value* ram_ptr = GetRAMPtr(i.arg);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            c->builder.CreateStore(load_x, ram_ptr);
            break;
        }
        case 0x96: {  // STX ZeropageY
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            llvm::Value* target_addr = c->builder.CreateAdd(load_y, zpg_addr);
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            c->builder.CreateCall(c->write_fn, {target_addr_16, load_x});
            break;
        }
        case 0x8E: {  // STX Absolute
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            WriteMemory(i.arg, load_x);
            break;
        }
        case 0x84: {  // STY Zeropage
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            WriteMemory(i.arg, load_y);
            break;
        }
        case 0x94: {  // STY ZeropageX
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Constant* zpg_addr = GetConstant8(i.arg);
            llvm::Value* target_addr = c->builder.CreateAdd(load_x, zpg_addr);
            llvm::Value* target_addr_16 =
                c->builder.CreateZExt(target_addr, int16);
            c->builder.CreateCall(c->write_fn, {target_addr_16, load_y});
            break;
        }
        case 0x8C: {  // STY Absolute
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            WriteMemory(i.arg, load_y);
            break;
        }
        case 0xAA: {  // TAX Implied

            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateStore(load_a, c->reg_x);

            // flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0xA8: {  // TAY Implied

            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateStore(load_a, c->reg_y);

            // flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0xBA: {  // TSX Implied
            break;
        }
        case 0x8A: {  // TXA Implied
            break;
        }
        case 0x9A: {  // TXS Implied
            break;
        }
        case 0x98: {  // TYA Implied
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* result = c->builder.CreateStore(load_y, c->reg_a);

            // flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x29: {  // AND Immediate
            break;
        }
        case 0x25: {  // AND Zeropage
            break;
        }
        case 0x35: {  // AND ZeropageX
            break;
        }
        case 0x2D: {  // AND Absolute
            break;
        }
        case 0x3D: {  // AND AbsoluteX
            break;
        }
        case 0x39: {  // AND AbsoluteY
            break;
        }
        case 0x21: {  // AND IndirectX
            break;
        }
        case 0x31: {  // AND IndirectY
            break;
        }
        case 0x0A: {  // ACC Accumulator
            break;
        }
        case 0x06: {  // ASL Zeropage
            break;
        }
        case 0x16: {  // ASL ZeropageX
            break;
        }
        case 0x0E: {  // ASL Absolute
            break;
        }
        case 0x1E: {  // ASL AbsoluteX
            break;
        }
        case 0x61: {  // ADC IndirectX
            break;
        }
        case 0x71: {  // ADC IndirectY
            break;
        }
        case 0x65: {  // ADC Zeropage
            break;
        }
        case 0x75: {  // ADC ZeropageX
            break;
        }
        case 0x6D: {  // ADC Absolute
            break;
        }
        case 0x7D: {  // ADC AbsoluteX
            break;
        }
        case 0x79: {  // ADC AbsoluteY
            break;
        }
        case 0xEA: {  // NOP Implied
            break;
        }
    }
}
}  // namespace llvmes
