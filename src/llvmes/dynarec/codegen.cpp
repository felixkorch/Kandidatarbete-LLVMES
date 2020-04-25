// #include "llvmes/dynarec/codegen.h"

#include "llvmes/dynarec/compiler.h"

namespace llvmes {

static std::unordered_map<uint16_t, uint16_t> return_map;

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
            llvm::Value* is_zero = c->builder.CreateICmpEQ(load_z, GetConstant1(1), "eq");
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
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            llvm::Value* iny = c->builder.CreateAdd(load_y, GetConstant8(1));
            c->builder.CreateStore(iny, c->reg_y);
            DynamicTestZ(iny);
            DynamicTestN(iny);
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
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* dex = c->builder.CreateSub(load_x, GetConstant8(1));
            c->builder.CreateStore(dex, c->reg_x);
            DynamicTestZ(dex);
            DynamicTestN(dex);
            break;
        }
        case 0xE6: {  // INC Zeropage
            llvm::Value* zpg_value = ReadMemory(i.arg);
            llvm::Value* inc = c->builder.CreateAdd(zpg_value, GetConstant8(1));
            WriteMemory(i.arg, inc);
            DynamicTestZ(inc);
            DynamicTestN(inc);
            break;
        }
        case 0xF6: {  // INC ZeropageX
            llvm::Value* zpg_addr = GetConstant8(i.arg);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* zpg_x_addr = c->builder.CreateAdd(zpg_addr, load_x);
            llvm::Value* zpg_x_addr_16 = c->builder.CreateZExt(zpg_x_addr, int16);
            llvm::Value* zpg_x_value = c->builder.CreateCall(c->read_fn, {zpg_x_addr_16});
            llvm::Value* incx = c->builder.CreateAdd(zpg_x_value, GetConstant8(1));
            c->builder.CreateCall(c->write_fn, {zpg_x_addr_16, incx});
            DynamicTestZ(incx);
            DynamicTestN(incx);
            break;
        }
        case 0xEE: {  // INC Absolute
            llvm::Value* value = ReadMemory(i.arg);
            llvm::Value* inca = c->builder.CreateAdd(value, GetConstant8(1));
            WriteMemory(i.arg, inca);
            DynamicTestZ(inca);
            DynamicTestN(inca);
            break;
        }
        case 0xFE: {  // INC AbsoluteX
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_x, int16);
            llvm::Value* addr_x = c->builder.CreateAdd(addr, load_x);
            llvm::Value* addr_x_value = c->builder.CreateLoad(addr_x);
            llvm::Value* incax = c->builder.CreateAdd(addr_x_value, GetConstant8(1));
            c->builder.CreateCall(c->write_fn, {incax, addr_x});
            DynamicTestZ(incax);
            DynamicTestN(incax);
            break;
        }
        case 0x4C: {  // JMP Absolute
            c->builder.CreateBr(c->basicblocks[i.target_label]);
            break;
        }
        case 0x6C: {  // JMP Indirect
            c->builder.CreateStore(ReadMemory16(i.arg), c->reg_idr);
            c->builder.CreateBr(c->dynJumpBlock);
            break;
        }
        case 0x20: {  // JSR Absolute
            uint16_t return_addr = i.offset + i.size;
            return_map[i.target_label.address] = return_addr;
            StackPush(GetConstant8(return_addr));
            c->builder.CreateBr(c->basicblocks[i.target_label]);
            llvm::BasicBlock* continue_block = CreateAutoLabel();
            c->builder.SetInsertPoint(continue_block);
            c->basicblocks[return_addr] = continue_block;
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
            llvm::Value* do_and_v = c->builder.CreateAnd(load_zero_page, c_0x40);
            llvm::Value* is_overflow = c->builder.CreateICmpEQ(do_and_v, c_0x40);
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
            llvm::Value* is_overflow = c->builder.CreateICmpEQ(do_and_v, c_0x40);
            c->builder.CreateStore(is_overflow, c->status_v);
            // Set N to Bit 7 of Memory value
            DynamicTestN(load_addr);
            break;
        }
        case 0x00: {  // BRK Implied
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
            llvm::Value* addr_base = c->builder.CreateAdd(load_x, GetConstant8(i.arg));

            // low
            llvm::Value* addr_base_16 = c->builder.CreateZExt(addr_base, int16);
            llvm::Value* addr_low = c->builder.CreateCall(c->read_fn, addr_base_16);
            llvm::Value* addr_low_16 = c->builder.CreateZExt(addr_low, int16);

            // high
            llvm::Value* addr_get_high = c->builder.CreateAdd(addr_base, GetConstant8(1));
            llvm::Value* addr_get_high_16 = c->builder.CreateZExt(addr_get_high, int16);
            llvm::Value* addr_high = c->builder.CreateCall(c->read_fn, addr_get_high_16);
            llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
            llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);

            llvm::Value* addr_hl_or = c->builder.CreateOr(addr_high_shl, addr_low_16);
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
            llvm::Value* addr_get_high_16 = c->builder.CreateZExt(addr_get_high, int16);
            llvm::Value* addr_high = c->builder.CreateCall(c->read_fn, addr_get_high);
            llvm::Value* high_addr_16 = c->builder.CreateZExt(addr_high, int16);
            llvm::Value* addr_high_shl = c->builder.CreateShl(high_addr_16, 8);

            llvm::Value* addr_hl_or = c->builder.CreateOr(addr_high_shl, addr_low_16);
            llvm::Value* addr_or_with_y = c->builder.CreateAdd(addr_hl_or, load_y_16);
            // reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* reg_a_16 = c->builder.CreateZExt(reg_a, int16);
            // Compare
            llvm::Value* result = c->builder.CreateSub(reg_a_16, addr_or_with_y);
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
            llvm::Value* zpg_value = ReadMemory(i.arg);
            llvm::Value* dec = c->builder.CreateSub(zpg_value, GetConstant8(1));
            WriteMemory(i.arg, dec);
            DynamicTestZ(dec);
            DynamicTestN(dec);
            break;
        }
        case 0xD6: {  // DEC ZeropageX
            llvm::Value* zpg_addr = GetConstant8(i.arg);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* zpg_x_addr = c->builder.CreateAdd(zpg_addr, load_x);
            llvm::Value* zpg_x_value = c->builder.CreateLoad(zpg_x_addr);
            llvm::Value* decx = c->builder.CreateSub(zpg_x_value, GetConstant8(1));
            c->builder.CreateCall(c->write_fn, {decx, zpg_x_addr});
            DynamicTestZ(decx);
            DynamicTestN(decx);
            break;
        }
        case 0xCE: {  // DEC Absolute
            llvm::Value* value = ReadMemory(i.arg);
            llvm::Value* deca = c->builder.CreateSub(value, GetConstant8(1));
            WriteMemory(i.arg, deca);
            DynamicTestZ(deca);
            DynamicTestN(deca);
            break;
        }
        case 0xDE: {  // DEC AbsoluteX
            llvm::Constant* addr = GetConstant16(i.arg);
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* target_addr_16 = c->builder.CreateZExt(load_x, int16);
            llvm::Value* addr_x = c->builder.CreateAdd(addr, load_x);
            llvm::Value* addr_x_value = c->builder.CreateLoad(addr_x);
            llvm::Value* decax = c->builder.CreateSub(addr_x_value, GetConstant8(1));
            c->builder.CreateCall(c->write_fn, {decax, addr_x});
            DynamicTestZ(decax);
            DynamicTestN(decax);
            break;
        }
        case 0x49: {  // EOR Immediate
            // In data
            llvm::Value* operand = AddressModeImmediate(i.arg);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x45: {  // EOR Zeropage
            // In data
            llvm::Value* addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x55: {  // EOR ZeropageX
            // In data
            llvm::Value* addr = AddressModeZeropageX(i.arg);
            llvm::Value* addr_16 = c->builder.CreateZExt(addr, int16);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr_16);
            // get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x4D: {  // EOR Absolute
            // In data
            llvm::Value* addr = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x5D: {  // EOR AbsoluteX
            // In data
            llvm::Value* addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x59: {  // EOR AbsoluteY
            // In data
            llvm::Value* addr = AddressModeAbsoluteY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x41: {  // EOR IndirectX
            llvm::Value* addr = AddressModeIndirectX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x51: {  // EOR IndirectY

            llvm::Value* addr = AddressModeIndirectY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get reg_a
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            // Exclusive or
            llvm::Value* result = c->builder.CreateXor(reg_a, operand);
            c->builder.CreateStore(result, c->reg_a);
            // Flag Test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0xA9: {  // LDA Immediate
            llvm::Value* load_value = AddressModeImmediate(i.arg);

            c->builder.CreateStore(load_value, c->reg_a);

            StaticTestZ(i.arg);
            StaticTestN(i.arg);
            break;
        }
        case 0xA5: {  // LDA Zeropage
            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xB5: {  // LDA ZeropageX
            llvm::Value* ram_pointer = AddressModeZeropageX(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xA1: {  // LDA IndirectX
            llvm::Value* ram_pointer = AddressModeIndirectX(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xB1: {  // LDA IndirectY
            llvm::Value* ram_pointer = AddressModeIndirectY(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xAD: {  // LDA Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xBD: {  // LDA AbsoluteX
            llvm::Value* ram_pointer = AddressModeAbsoluteX(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xB9: {  // LDA AbsoluteY
            llvm::Value* ram_pointer = AddressModeAbsoluteY(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDA(load_value);
            break;
        }
        case 0xA2: {  // LDX Immediate
            llvm::Value* load_value = AddressModeImmediate(i.arg);

            c->builder.CreateStore(load_value, c->reg_x);

            StaticTestZ(i.arg);
            StaticTestN(i.arg);
            break;
        }
        case 0xA6: {  // LDX Zeropage
            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDX(load_value);
            break;
        }
        case 0xB6: {  // LDX ZeropageY
            llvm::Value* ram_pointer = AddressModeZeropageY(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDX(load_value);
            break;
        }
        case 0xAE: {  // LDX Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDX(load_value);
            break;
        }
        case 0xBE: {  // LDX AbsoluteY
            llvm::Value* ram_pointer = AddressModeAbsoluteY(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDX(load_value);
            break;
        }
        case 0xA0: {  // LDY Immediate
            llvm::Value* load_value = AddressModeImmediate(i.arg);

            c->builder.CreateStore(load_value, c->reg_y);

            StaticTestZ(i.arg);
            StaticTestN(i.arg);
            break;
        }
        case 0xA4: {  // LDY Zeropage
            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDY(load_value);
            break;
        }
        case 0xB4: {  // LDY ZeropageX
            llvm::Value* ram_pointer = AddressModeZeropageX(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDY(load_value);
            break;
        }
        case 0xAC: {  // LDY Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* load_value = c->builder.CreateLoad(ram_pointer);

            OP_LDY(load_value);
            break;
        }
        case 0xBC: {  // LDY AbsoluteX
            llvm::Value* ram_pointer = AddressModeAbsoluteX(i.arg);
            llvm::Value* load_value = c->builder.CreateCall(c->read_fn, ram_pointer);

            OP_LDY(load_value);
            break;
        }
        case 0x4A: {  // LSR Accumulator
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* and_1 = c->builder.CreateAnd(reg_a, GetConstant8(1));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_1, GetConstant8(1));
            c->builder.CreateStore(status_c, c->status_c);
            reg_a = c->builder.CreateLShr(reg_a, 1);
            DynamicTestZ(reg_a);
            DynamicTestN(reg_a);
            c->builder.CreateStore(reg_a, c->reg_a);
            break;
        }
        case 0x46: {  // LSR Zeropage
            llvm::Value* addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(addr);
            llvm::Value* and_1 = c->builder.CreateAnd(operand, GetConstant8(1));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_1, GetConstant8(1));
            c->builder.CreateStore(status_c, c->status_c);
            operand = c->builder.CreateLShr(operand, 1);
            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateStore(operand, addr);
            break;
        }
        case 0x56: {  // LSR ZeropageX
            llvm::Value* addr = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, {addr});
            llvm::Value* and_1 = c->builder.CreateAnd(operand, GetConstant8(1));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_1, GetConstant8(1));
            c->builder.CreateStore(status_c, c->status_c);
            operand = c->builder.CreateLShr(operand, 1);
            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateCall(c->write_fn, {addr, operand});
            break;
        }
        case 0x4E: {  // LSR Absolute
            llvm::Value* addr = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(addr);
            llvm::Value* and_1 = c->builder.CreateAnd(operand, GetConstant8(1));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_1, GetConstant8(1));
            c->builder.CreateStore(status_c, c->status_c);
            operand = c->builder.CreateLShr(operand, 1);
            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateStore(operand, addr);
            break;
        }
        case 0x5E: {  // LSR AbsoluteX
            llvm::Value* addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, {addr});
            llvm::Value* and_1 = c->builder.CreateAnd(operand, GetConstant8(1));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_1, GetConstant8(1));
            c->builder.CreateStore(status_c, c->status_c);
            operand = c->builder.CreateLShr(operand, 1);
            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateCall(c->write_fn, {addr, operand});
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
            llvm::Value* a = c->builder.CreateLoad(c->reg_a);
            StackPush(a);
            break;
        }
        case 0x08: {  // PHP Implied
            llvm::Value* status_c = c->builder.CreateLoad(c->status_c);
            status_c = c->builder.CreateZExt(status_c, int8);
            llvm::Value* status_z = c->builder.CreateLoad(c->status_z);
            status_z = c->builder.CreateZExt(status_z, int8);
            status_z = c->builder.CreateShl(status_z, 1);
            llvm::Value* status_i = c->builder.CreateLoad(c->status_i);
            status_i = c->builder.CreateZExt(status_i, int8);
            status_i = c->builder.CreateShl(status_i, 2);
            llvm::Value* status_d = c->builder.CreateLoad(c->status_d);
            status_d = c->builder.CreateZExt(status_d, int8);
            status_d = c->builder.CreateShl(status_d, 3);
            llvm::Value* status_b = GetConstant8(0x30);  // Always on bits
            llvm::Value* status_v = c->builder.CreateLoad(c->status_v);
            status_v = c->builder.CreateZExt(status_v, int8);
            status_v = c->builder.CreateShl(status_v, 6);
            llvm::Value* status_n = c->builder.CreateLoad(c->status_n);
            status_n = c->builder.CreateZExt(status_n, int8);
            status_n = c->builder.CreateShl(status_n, 7);
            llvm::Value* status = c->builder.CreateOr(status_z, status_c);
            status = c->builder.CreateOr(status, status_i);
            status = c->builder.CreateOr(status, status_d);
            status = c->builder.CreateOr(status, status_b);
            status = c->builder.CreateOr(status, status_v);
            status = c->builder.CreateOr(status, status_n);
            StackPush(status);
            break;
        }
        case 0x68: {  // PLA Implied
            llvm::Value* top_of_stack = StackPull();
            c->builder.CreateStore(top_of_stack, c->reg_a);
            break;
        }
        case 0x28: {  // PLP Implied
            // PLP ignore bits 4 and 5 from the stack
            llvm::Value* status = StackPull();
            llvm::Value* and_c = c->builder.CreateAnd(status, GetConstant8(0x01));
            llvm::Value* status_c = c->builder.CreateICmpEQ(and_c, GetConstant8(0x01));
            c->builder.CreateStore(status_c, c->status_c);
            llvm::Value* and_z = c->builder.CreateAnd(status, GetConstant8(0x02));
            llvm::Value* status_z = c->builder.CreateICmpEQ(and_z, GetConstant8(0x02));
            c->builder.CreateStore(status_z, c->status_z);
            llvm::Value* and_i = c->builder.CreateAnd(status, GetConstant8(0x04));
            llvm::Value* status_i = c->builder.CreateICmpEQ(and_i, GetConstant8(0x04));
            c->builder.CreateStore(status_i, c->status_i);
            llvm::Value* and_d = c->builder.CreateAnd(status, GetConstant8(0x08));
            llvm::Value* status_d = c->builder.CreateICmpEQ(and_d, GetConstant8(0x08));
            c->builder.CreateStore(status_d, c->status_d);
            llvm::Value* and_b = c->builder.CreateAnd(status, GetConstant8(0x10));
            llvm::Value* status_b = c->builder.CreateICmpEQ(and_b, GetConstant8(0x10));
            c->builder.CreateStore(status_b, c->status_b);
            llvm::Value* and_u = c->builder.CreateAnd(status, GetConstant8(0x20));
            llvm::Value* status_u = c->builder.CreateICmpEQ(and_u, GetConstant8(0x20));
            c->builder.CreateStore(status_u, c->status_u);
            llvm::Value* and_v = c->builder.CreateAnd(status, GetConstant8(0x40));
            llvm::Value* status_v = c->builder.CreateICmpEQ(and_v, GetConstant8(0x40));
            c->builder.CreateStore(status_v, c->status_v);
            llvm::Value* and_n = c->builder.CreateAnd(status, GetConstant8(0x80));
            llvm::Value* status_n = c->builder.CreateICmpEQ(and_n, GetConstant8(0x80));
            c->builder.CreateStore(status_n, c->status_n);
            break;
        }
        case 0x2A: {  // ROL Accumulator
            // Get reg_a and status_c
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(reg_a, GetConstant8(0x80));
            // Shift reg_a left
            llvm::Value* reg_a_Shr = c->builder.CreateShl(reg_a, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_8);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x80));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x26: {  // ROL Zeropage
            llvm::Value* addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x80));
            // Shift reg_a left
            llvm::Value* reg_a_Shr = c->builder.CreateShl(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_8);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x80));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x36: {  // ROL ZeropageX
            llvm::Value* addr = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x80));
            // Shift reg_a left
            llvm::Value* reg_a_Shr = c->builder.CreateShl(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_8);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x80));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x2E: {  // ROL Absolute
            llvm::Value* addr = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x80));
            // Shift reg_a left
            llvm::Value* reg_a_Shr = c->builder.CreateShl(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_8);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x80));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x3E: {  // ROL AbsoluteX
            llvm::Value* addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x80));
            // Shift reg_a left
            llvm::Value* reg_a_Shr = c->builder.CreateShl(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_8);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x80));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x6A: {  // ROR Accumulator
            // Get reg_a and status_c
            llvm::Value* reg_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(reg_a, GetConstant8(0x01));
            // Shift reg_a
            llvm::Value* reg_a_Shr = c->builder.CreateLShr(reg_a, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* carry_in_shl = c->builder.CreateShl(carry_in_8, 7);
            llvm::Value* result = c->builder.CreateOr(reg_a_Shr, carry_in_shl);
            // Stor reg_a
            c->builder.CreateStore(result, c->reg_a);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x01));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x66: {  // ROR Zeropage
            llvm::Value* addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x01));
            // Shift reg_a
            llvm::Value* operand_Shr = c->builder.CreateLShr(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* carry_in_shl = c->builder.CreateShl(carry_in_8, 7);
            llvm::Value* result = c->builder.CreateOr(operand_Shr, carry_in_shl);
            // Store in memory
            c->builder.CreateStore(result, addr);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x01));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);

            break;
        }
        case 0x76: {  // ROR ZeropageX
            llvm::Value* addr = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x01));
            // Shift reg_a
            llvm::Value* operand_Shr = c->builder.CreateLShr(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* carry_in_shl = c->builder.CreateShl(carry_in_8, 7);
            llvm::Value* result = c->builder.CreateOr(operand_Shr, carry_in_shl);
            // Store in memory
            c->builder.CreateCall(c->write_fn, {addr, result});
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x01));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x6E: {  // ROR Absolute
            llvm::Value* addr = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x01));
            // Shift reg_a
            llvm::Value* operand_Shr = c->builder.CreateLShr(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* carry_in_shl = c->builder.CreateShl(carry_in_8, 7);
            llvm::Value* result = c->builder.CreateOr(operand_Shr, carry_in_shl);
            // Store in memory
            c->builder.CreateStore(result, addr);
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x01));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x7E: {  // ROR AbsoluteX
            llvm::Value* addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, addr);
            // Get status_c
            llvm::Value* carry_in = c->builder.CreateLoad(c->status_c);
            // Get carry_out
            llvm::Value* carry_out = c->builder.CreateAnd(operand, GetConstant8(0x01));
            // Shift reg_a
            llvm::Value* operand_Shr = c->builder.CreateLShr(operand, 1);
            // Add carry_in
            llvm::Value* carry_in_8 = c->builder.CreateZExt(carry_in, int8);
            llvm::Value* carry_in_shl = c->builder.CreateShl(carry_in_8, 7);
            llvm::Value* result = c->builder.CreateOr(operand_Shr, carry_in_shl);
            // Store in memory
            c->builder.CreateCall(c->write_fn, {addr, result});
            // Set status_c
            llvm::Value* carry_out_1 =
                c->builder.CreateICmpEQ(carry_out, GetConstant8(0x01));
            c->builder.CreateStore(carry_out_1, c->status_c);
            // Flag test
            DynamicTestZ(result);
            DynamicTestN(result);
            break;
        }
        case 0x40: {  // RTI Implied
            break;
        }
        case 0x60: {  // RTS Implied
            // TODO: use this llvm::value to dynamically jump to a return address
            llvm::Value* return_addr = StackPull();
            uint16_t return_addr_s = return_map[current_block_address];
            c->builder.CreateBr(c->basicblocks[return_addr_s]);
            break;
        }
        case 0xE9: {  // SBC Immediate
            llvm::Value* operand = AddressModeImmediate(i.arg);
            OP_SBC(operand);
            break;
        }
        case 0xE5: {  // SBC Zeropage
            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xF5: {  // SBC ZeropageX
            llvm::Value* ram_pointer = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xED: {  // SBC Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xFD: {  // SBC AbsoluteX
            llvm::Value* ram_pointer = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xF9: {  // SBC AbsoluteY
            llvm::Value* ram_pointer = AddressModeAbsoluteY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xE1: {  // SBC IndirectX
            llvm::Value* ram_pointer = AddressModeIndirectX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0xF1: {  // SBC IndirectY
            llvm::Value* ram_pointer = AddressModeIndirectY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_SBC(operand);
            break;
        }
        case 0x38: {  // SEC Implied
            llvm::Constant* carry = llvm::ConstantInt::get(int1, 0x1);
            c->builder.CreateStore(carry, c->status_c);
            break;
        }
        case 0xF8: {  // SED Implied
            // Since our abstract 6502 does not implement a dedicated decimal
            // mode but instead handles decimal on the fly all the time, there
            // exists no decimal flag. Therefore there is no need to ever set or
            // clear the decimal flag and those instructions are instead
            // replaced with nothing when compiled
            break;
        }
        case 0x78: {  // SEI Implied
            c->builder.CreateStore(GetConstant1(1), c->status_i);
            break;
        }
        case 0x18: {  // CLC Implied
            llvm::Constant* carry = llvm::ConstantInt::get(int1, 0x0);
            c->builder.CreateStore(carry, c->status_c);
            break;
        }
        case 0xD8: {  // CLD Implied
            // Since our abstract 6502 does not implement a dedicated decimal
            // mode but instead handles decimal on the fly all the time, there
            // exists no decimal flag. Therefore there is no need to ever set or
            // clear the decimal flag and those instructions are instead
            // replaced with nothing when compiled
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
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            WriteMemory(i.arg, load_a);
            break;
        }
        case 0x95: {  // STA ZeropageX
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* target_addr = AddressModeZeropageX(i.arg);
            c->builder.CreateCall(c->write_fn, {target_addr, load_a});
            break;
        }
        case 0x8D: {  // STA Absolute
            uint16_t addr = i.arg;
            // Write to '0x2008' and 'A' will be written to stdout as
            // char
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
            // Write flags to stdout
            else if (addr == 0x200C) {
                llvm::Value* status_c = c->builder.CreateLoad(c->status_c);
                status_c = c->builder.CreateZExt(status_c, int8);
                llvm::Value* status_z = c->builder.CreateLoad(c->status_z);
                status_z = c->builder.CreateZExt(status_z, int8);
                status_z = c->builder.CreateShl(status_z, 1);
                llvm::Value* status_i = c->builder.CreateLoad(c->status_i);
                status_i = c->builder.CreateZExt(status_i, int8);
                status_i = c->builder.CreateShl(status_i, 2);
                llvm::Value* status_d = c->builder.CreateLoad(c->status_d);
                status_d = c->builder.CreateZExt(status_d, int8);
                status_d = c->builder.CreateShl(status_d, 3);
                llvm::Value* status_b = c->builder.CreateLoad(c->status_b);
                status_b = c->builder.CreateZExt(status_b, int8);
                status_b = c->builder.CreateShl(status_b, 4);
                llvm::Value* status_u = c->builder.CreateLoad(c->status_u);
                status_u = c->builder.CreateZExt(status_u, int8);
                status_u = c->builder.CreateShl(status_u, 5);
                llvm::Value* status_v = c->builder.CreateLoad(c->status_v);
                status_v = c->builder.CreateZExt(status_v, int8);
                status_v = c->builder.CreateShl(status_v, 6);
                llvm::Value* status_n = c->builder.CreateLoad(c->status_n);
                status_n = c->builder.CreateZExt(status_n, int8);
                status_n = c->builder.CreateShl(status_n, 7);
                llvm::Value* status = c->builder.CreateOr(status_z, status_c);
                status = c->builder.CreateOr(status, status_i);
                status = c->builder.CreateOr(status, status_d);
                status = c->builder.CreateOr(status, status_b);
                status = c->builder.CreateOr(status, status_u);
                status = c->builder.CreateOr(status, status_v);
                status = c->builder.CreateOr(status, status_n);
                c->builder.CreateCall(c->putstatus_fn, {status});
            }
            else if (addr == 0x200F) {  // Exit
                llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                llvm::Value* a_32 = c->builder.CreateZExt(load_a, int32);
                c->builder.CreateRet(a_32);
            }
            // Store normally
            else {
                llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                WriteMemory(addr, load_a);
            }
            break;
        }
        case 0x9D: {  // STA AbsoluteX
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* addr_base = AddressModeAbsoluteX(i.arg);
            c->builder.CreateCall(c->write_fn, {addr_base, load_a});
            break;
        }
        case 0x99: {  // STA AbsoluteY
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* addr_base = AddressModeAbsoluteY(i.arg);
            c->builder.CreateCall(c->write_fn, {addr_base, load_a});
            break;
        }
        case 0x81: {  // STA IndirectX
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* addr_hl_or = AddressModeIndirectX(i.arg);
            c->builder.CreateCall(c->write_fn, {addr_hl_or, load_a});
            break;
        }
        case 0x91: {  // STA IndirectY
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* addr_hl_or = AddressModeIndirectY(i.arg);
            c->builder.CreateCall(c->write_fn, {addr_hl_or, load_a});
            break;
        }
        case 0x86: {  // STX Zeropage
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            WriteMemory(i.arg, load_x);
            break;
        }
        case 0x96: {  // STX ZeropageY
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            llvm::Value* target_addr = AddressModeZeropageY(i.arg);
            c->builder.CreateCall(c->write_fn, {target_addr, load_x});
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
            llvm::Value* target_addr = AddressModeZeropageX(i.arg);
            c->builder.CreateCall(c->write_fn, {target_addr, load_y});
            break;
        }
        case 0x8C: {  // STY Absolute
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            WriteMemory(i.arg, load_y);
            break;
        }
        case 0xAA: {  // TAX Implied

            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            c->builder.CreateStore(load_a, c->reg_x);

            // flag test
            DynamicTestZ(load_a);
            DynamicTestN(load_a);
            break;
        }
        case 0xA8: {  // TAY Implied

            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            c->builder.CreateStore(load_a, c->reg_y);

            // flag test
            DynamicTestZ(load_a);
            DynamicTestN(load_a);
            break;
        }
        case 0xBA: {  // TSX Implied
            llvm::Value* load_sp = c->builder.CreateLoad(c->reg_sp);
            c->builder.CreateStore(load_sp, c->reg_x);

            // Flag test
            DynamicTestZ(load_sp);
            DynamicTestN(load_sp);
            break;
        }
        case 0x8A: {  // TXA Implied
            llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
            c->builder.CreateStore(load_x, c->reg_a);

            // Flag test
            DynamicTestZ(load_x);
            DynamicTestN(load_x);
            break;
        }
        case 0x9A: {  // TXS Implied
            llvm::Value* reg_x = c->builder.CreateLoad(c->reg_x);
            c->builder.CreateStore(reg_x, c->reg_sp);
            break;
        }
        case 0x98: {  // TYA Implied
            llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
            c->builder.CreateStore(load_y, c->reg_a);

            // Flag test
            DynamicTestN(load_y);
            DynamicTestZ(load_y);

            break;
        }
        case 0x29: {  // AND Immediate
            llvm::Value* operand = AddressModeImmediate(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x25: {  // AND Zeropage
            llvm::Value* target_addr = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x35: {  // AND ZeropageX
            llvm::Value* target_addr = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x2D: {  // AND Absolute
            llvm::Value* operand = ReadMemory(i.arg);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x3D: {  // AND AbsoluteX
            llvm::Value* target_addr = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x39: {  // AND AbsoluteY
            llvm::Value* target_addr = AddressModeAbsoluteY(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x21: {  // AND IndirectX
            llvm::Value* target_addr = AddressModeIndirectX(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x31: {  // AND IndirectY
            llvm::Value* target_addr = AddressModeIndirectY(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(target_addr);
            llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
            llvm::Value* result = c->builder.CreateAnd(load_a, operand);
            // Set Z to zero if result is zero
            // Set N if bit 7 set
            DynamicTestZ(result);
            DynamicTestN(result);
            llvm::Value* store = c->builder.CreateStore(result, c->reg_a);
            break;
        }
        case 0x0A: {  // ACC Accumulator
            break;
        }
        case 0x06: {  // ASL Zeropage

            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);

            // Test C
            llvm::Value* C = c->builder.CreateAnd(operand, GetConstant8(0x80));
            C = c->builder.CreateICmpEQ(C, GetConstant8(0x80));
            c->builder.CreateStore(C, c->status_n);

            operand = c->builder.CreateShl(operand, 1);

            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateStore(operand, ram_pointer);        
            break;
        }
        case 0x16: {  // ASL ZeropageX
            llvm::Value* ram_pointer = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);

            // Test C
            llvm::Value* C = c->builder.CreateAnd(operand, GetConstant8(0x80));
            C = c->builder.CreateICmpEQ(C, GetConstant8(0x80));
            c->builder.CreateStore(C, c->status_n);

            operand = c->builder.CreateShl(operand, 1);

            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateCall(c->write_fn, {ram_pointer, operand});
            break;
        }
        case 0x0E: {  // ASL Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);

            // Test C
            llvm::Value* C = c->builder.CreateAnd(operand, GetConstant8(0x80));
            C = c->builder.CreateICmpEQ(C, GetConstant8(0x80));
            c->builder.CreateStore(C, c->status_n);

            operand = c->builder.CreateShl(operand, 1);

            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateStore(operand, ram_pointer);
            break;
        }
        case 0x1E: {  // ASL AbsoluteX
            llvm::Value* ram_pointer = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);

            // Test C
            llvm::Value* C = c->builder.CreateAnd(operand, GetConstant8(0x80));
            C = c->builder.CreateICmpEQ(C, GetConstant8(0x80));
            c->builder.CreateStore(C, c->status_n);

            operand = c->builder.CreateShl(operand, 1);

            DynamicTestZ(operand);
            DynamicTestN(operand);
            c->builder.CreateCall(c->write_fn, {ram_pointer, operand});
            break;
        }
        case 0x69: {  // ADC Immediate
            llvm::Value* operand = AddressModeImmediate(i.arg);
            OP_ADC(operand);
            break;
        }
        case 0x61: {  // ADC IndirectX
            llvm::Value* ram_pointer = AddressModeIndirectX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x71: {  // ADC IndirectY
            llvm::Value* ram_pointer = AddressModeIndirectY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x65: {  // ADC Zeropage
            llvm::Value* ram_pointer = AddressModeZeropage(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x75: {  // ADC ZeropageX
            llvm::Value* ram_pointer = AddressModeZeropageX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x6D: {  // ADC Absolute
            llvm::Value* ram_pointer = AddressModeAbsolute(i.arg);
            llvm::Value* operand = c->builder.CreateLoad(ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x7D: {  // ADC AbsoluteX
            llvm::Value* ram_pointer = AddressModeAbsoluteX(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0x79: {  // ADC AbsoluteY
            llvm::Value* ram_pointer = AddressModeAbsoluteY(i.arg);
            llvm::Value* operand = c->builder.CreateCall(c->read_fn, ram_pointer);
            OP_ADC(operand);
            break;
        }
        case 0xEA: {  // NOP Implied
            break;
        }
    }
}

void Compiler::OP_ADC(llvm::Value* operand)
{
    operand = c->builder.CreateZExtOrBitCast(operand, int16);

    // Loads the A register into a placeholder
    llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
    load_a = c->builder.CreateZExtOrBitCast(load_a, int16);

    // Loads Carry register into a placeholder
    llvm::Value* load_c = c->builder.CreateLoad(c->status_c);
    load_c = c->builder.CreateZExtOrBitCast(load_c, int16);

    // Subtract ram & carry from A
    llvm::Value* result_a_ram = c->builder.CreateAdd(load_a, operand);
    llvm::Value* result = c->builder.CreateAdd(result_a_ram, load_c);

    // Handling overflow

    llvm::Constant* c_0x80 = llvm::ConstantInt::get(int16, 0x80);

    llvm::Value* xor_result = c->builder.CreateXor(load_a, result);
    llvm::Value* xor_operand = c->builder.CreateXor(load_a, operand);

    llvm::Value* and_result = c->builder.CreateAnd(xor_result, c_0x80);
    llvm::Value* and_operand = c->builder.CreateAnd(xor_operand, c_0x80);

    llvm::Value* NE = c->builder.CreateICmpNE(and_result, GetConstant16(0));
    llvm::Value* EQ = c->builder.CreateICmpEQ(and_operand, GetConstant16(0));

    llvm::Value* overflow = c->builder.CreateAnd(EQ, NE);

    c->builder.CreateStore(overflow, c->status_v);

    // Test Z
    llvm::Value* Z = c->builder.CreateICmpEQ(result, GetConstant16(0));
    c->builder.CreateStore(Z, c->status_z);

    // Test N
    llvm::Value* N = c->builder.CreateAnd(result, GetConstant16(0x80));
    N = c->builder.CreateICmpEQ(N, GetConstant16(0x80));
    c->builder.CreateStore(N, c->status_n);

    // Test C
    llvm::Value* C = c->builder.CreateICmpUGT(result, GetConstant16(0xFF));
    c->builder.CreateStore(C, c->status_c);

    // Truncate result and store in A
    result = c->builder.CreateZExtOrTrunc(result, int8);
    c->builder.CreateStore(result, c->reg_a);
}


void Compiler::OP_SBC(llvm::Value* operand)
{
    operand = c->builder.CreateZExtOrBitCast(operand, int16);

    // Loads the A register into a placeholder
    llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
    load_a = c->builder.CreateZExtOrBitCast(load_a, int16);

    // Loads Carry register into a placeholder
    llvm::Value* load_c = c->builder.CreateLoad(c->status_c);
    load_c = c->builder.CreateNot(load_c);
    load_c = c->builder.CreateZExtOrBitCast(load_c, int16);

    // Subtract ram & carry from A
    llvm::Value* result_a_ram = c->builder.CreateSub(load_a, operand);
    llvm::Value* result = c->builder.CreateSub(result_a_ram, load_c);

    // Handling overflow

    llvm::Constant* c_0x80 = llvm::ConstantInt::get(int16, 0x80);

    llvm::Value* xor_1 = c->builder.CreateXor(load_a, result);
    llvm::Value* xor_2 = c->builder.CreateXor(load_a, operand);

    llvm::Value* and_1 = c->builder.CreateAnd(xor_1, c_0x80);
    llvm::Value* and_2 = c->builder.CreateAnd(xor_2, c_0x80);

    llvm::Value* SGT_1 = c->builder.CreateICmpNE(and_1, GetConstant16(0));
    llvm::Value* SGT_2 = c->builder.CreateICmpNE(and_2, GetConstant16(0));

    llvm::Value* overflow = c->builder.CreateAnd(SGT_2, SGT_1);

    c->builder.CreateStore(overflow, c->status_v);

    // TEST Z
    llvm::Value* Z = c->builder.CreateICmpEQ(result, GetConstant16(0));
    c->builder.CreateStore(Z, c->status_z);

    // Test N
    llvm::Value* N = c->builder.CreateAnd(result, GetConstant16(0x80));
    N = c->builder.CreateICmpEQ(N, GetConstant16(0x80));
    c->builder.CreateStore(N, c->status_n);

    // Test C
    llvm::Value* C = c->builder.CreateICmpULT(result, GetConstant16(0x0100));
    c->builder.CreateStore(C, c->status_c);

    // Truncate result and store in A
    result = c->builder.CreateZExtOrTrunc(result, int8);
    c->builder.CreateStore(result, c->reg_a);
}

void Compiler::OP_LDA(llvm::Value* operand)
{
    c->builder.CreateStore(operand, c->reg_a);

    DynamicTestZ(operand);
    DynamicTestN(operand);
    
}

void Compiler::OP_LDX(llvm::Value* operand)
{
    c->builder.CreateStore(operand, c->reg_x);

    DynamicTestZ(operand);
    DynamicTestN(operand);
}

void Compiler::OP_LDY(llvm::Value* operand)
{
    c->builder.CreateStore(operand, c->reg_y);

    DynamicTestZ(operand);
    DynamicTestN(operand);
}



}  // namespace llvmes
