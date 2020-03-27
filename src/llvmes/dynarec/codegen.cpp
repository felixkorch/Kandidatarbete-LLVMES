#include "codegen.h"

namespace llvmes {
    void Compiler::CodeGen(Instruction& i)
    {
        switch (i.opcode) {
            case 0x8D: {  // STA ABS
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
                else {
                    llvm::Value* ram_ptr = GetRAMPtr(addr);
                    llvm::Value* load_a = c->builder.CreateLoad(c->reg_a);
                    c->builder.CreateStore(load_a, ram_ptr);
                }
                break;
            }
            case 0x8E: {  // STX
                llvm::Value* ram_ptr = GetRAMPtr(i.arg);
                llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
                c->builder.CreateStore(load_x, ram_ptr);
                break;
            }
            case 0xA0: {  // LDY
                int arg = i.arg;
                llvm::Value* a = llvm::ConstantInt::get(int8, arg);
                c->builder.CreateStore(a, c->reg_y);
                StaticTestZ(arg);
                StaticTestN(arg);
                break;
            }
            case 0xAD: {  // LDA Abs
                llvm::Value* ram_ptr = GetRAMPtr(i.arg);
                llvm::Value* load_ram = c->builder.CreateLoad(ram_ptr);
                c->builder.CreateStore(load_ram, c->reg_a);
                break;
            }
            case 0xE8: {  // INX
                llvm::Value* load_x = c->builder.CreateLoad(c->reg_x);
                llvm::Value* inx = c->builder.CreateAdd(load_x, c1_8);
                c->builder.CreateStore(inx, c->reg_x);
                DynamicTestZ(inx);
                DynamicTestN(inx);
                break;
            }
            case 0x88: {  // DEY
                llvm::Value* load_y = c->builder.CreateLoad(c->reg_y);
                llvm::Value* dey = c->builder.CreateSub(load_y, c1_8);
                c->builder.CreateStore(dey, c->reg_y);
                DynamicTestZ(dey);
                DynamicTestN(dey);
                break;
            }
            case 0xEA: {  // NOP
                break;
            }
            case 0xD0: {  // BNE
                llvm::Value* load_z = c->builder.CreateLoad(c->status_z);
                llvm::Value* is_nonzero =
                    c->builder.CreateICmpNE(load_z, c1_1, "ne");
                CreateCondBranch(is_nonzero, c->basicblocks[i.target_label]);
                break;
            }
            case 0xF0: {  // BEQ
                llvm::Value* load_z = c->builder.CreateLoad(c->status_z);
                llvm::Value* is_zero =
                    c->builder.CreateICmpEQ(load_z, c1_1, "eq");
                CreateCondBranch(is_zero, c->basicblocks[i.target_label]);
                break;
            }
            case 0x4C: { // JMP Abs
                c->builder.CreateBr(c->basicblocks[i.target_label]);
            }
        }
    }
}
