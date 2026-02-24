// multiplyByXGenerator.cpp
// Generates x86_64 assembly for a function multiplyBy<N>(IntArray *p)
// that multiplies every element of the IntArray by the constant N,
// WITHOUT using any multiply instruction (imul/imulq/fmul).
//
// Algorithm: Binary decomposition (shift-and-add).
// Any integer N can be written as a sum of distinct powers of 2.
// Multiplying by a power of 2 is a left shift.
// e.g. 61 = 32+16+8+4+1 = 2^5+2^4+2^3+2^2+2^0
// so  val*61 = (val<<5)+(val<<4)+(val<<3)+(val<<2)+(val<<0)
//
// Sources consulted:
//   - GCC internals docs on RTX multiplication expansion (strength reduction)
//   - "Hacker's Delight" by Henry S. Warren Jr., Chapter 8 (Multiplication)
//   - x86_64 System V ABI calling convention reference
//   - Generated .s files from g++ -O as structural templates

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include "IntArray.h"

// Return the set bits (positions) of n, from lowest to highest.
// These correspond to the shifts needed: n = sum of (1 << bit) for each set bit.
std::vector<int> getBits(int n) {
    std::vector<int> bits;
    for (int i = 0; i < 32; i++) {
        if (n & (1 << i)) {
            bits.push_back(i);
        }
    }
    return bits;
}

// Emit the C++ name-mangled symbol for multiplyBy<n>(IntArray*)
// x86_64 Linux g++ mangling: _Z + len(name) + name + P8IntArray
// "multiplyBy" + digits, then "P8IntArray"
std::string mangledName(int n) {
    std::string funcName = "multiplyBy" + std::to_string(n);
    return "_Z" + std::to_string(funcName.size()) + funcName + "P8IntArray";
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <positive_integer>" << std::endl;
        return 1;
    }

    int constant = std::atoi(argv[1]);
    if (constant <= 0) {
        std::cerr << "Error: constant must be a positive integer." << std::endl;
        return 1;
    }

    std::string funcName = "multiplyBy" + std::to_string(constant);
    std::string sym = mangledName(constant);
    std::vector<int> bits = getBits(constant);

    // ---------------------------------------------------------------
    // Emit assembly header
    // ---------------------------------------------------------------
    std::cout << "\t.file\t\"" << funcName << ".cpp\"\n";
    std::cout << "\t.text\n";
    std::cout << "\t.globl\t" << sym << "\n";
    std::cout << "\t.type\t" << sym << ", @function\n";
    std::cout << sym << ":\n";
    std::cout << ".LFB0:\n";
    std::cout << "\t.cfi_startproc\n";
    std::cout << "\tendbr64\n";

    // ---------------------------------------------------------------
    // Function prologue / loop setup
    //
    // Calling convention (System V AMD64 ABI):
    //   %rdi = IntArray *p   (first argument)
    //
    // Register usage:
    //   %eax = loop counter (i), starts at 0
    //   %rdi = pointer to IntArray struct
    //   (%rdi)   = p->size   (int, 4 bytes)
    //   8(%rdi)  = p->elements (int*, 8 bytes)
    //   %rdx = pointer to current element: elements + i*4
    //   %ecx = current element value (loaded from *%rdx)
    //   %r8d = accumulator for the shifted sum
    // ---------------------------------------------------------------

    // Check if size <= 0, skip loop entirely
    std::cout << "\tcmpl\t$0, (%rdi)\n";
    std::cout << "\tjle\t.L1\n";
    std::cout << "\tmovl\t$0, %eax\n";       // i = 0

    // ---------------------------------------------------------------
    // Loop body
    // ---------------------------------------------------------------
    std::cout << ".L3:\n";
    // %rdx = &p->elements[i]  (elements ptr + i*4)
    std::cout << "\tmovq\t8(%rdi), %rdx\n";
    std::cout << "\tleaq\t(%rdx,%rax,4), %rdx\n";

    // Load current element into %ecx
    std::cout << "\tmovl\t(%rdx), %ecx\n";

    // ---------------------------------------------------------------
    // Compute ecx * constant using only shifts and adds (no imul).
    //
    // Strategy: iterate over each set bit b in `constant`.
    //   partial = ecx << b
    //   accumulate into %r8d
    //
    // Special case: if constant is a power of 2 (only 1 set bit),
    // we just shift and store directly.
    // ---------------------------------------------------------------

    if (bits.size() == 1) {
        // Power of 2 — single shift suffices
        int shift = bits[0];
        if (shift == 0) {
            // multiply by 1: no-op, value stays in %ecx
        } else {
            std::cout << "\tsall\t$" << shift << ", %ecx\n";
        }
        std::cout << "\tmovl\t%ecx, (%rdx)\n";
    } else {
        // General case: sum of shifted copies
        // We'll use %r8d as the running sum, %ecx as the base value.
        // For the first set bit, initialize %r8d.
        bool first = true;
        for (int bit : bits) {
            if (first) {
                first = false;
                if (bit == 0) {
                    // partial = ecx << 0 = ecx; r8d = ecx
                    std::cout << "\tmovl\t%ecx, %r8d\n";
                } else {
                    // r8d = ecx << bit
                    std::cout << "\tmovl\t%ecx, %r8d\n";
                    std::cout << "\tsall\t$" << bit << ", %r8d\n";
                }
            } else {
                // partial = ecx << bit, then r8d += partial
                // Use a temp register %r9d for the shifted value
                if (bit == 0) {
                    std::cout << "\taddl\t%ecx, %r8d\n";
                } else {
                    std::cout << "\tmovl\t%ecx, %r9d\n";
                    std::cout << "\tsall\t$" << bit << ", %r9d\n";
                    std::cout << "\taddl\t%r9d, %r8d\n";
                }
            }
        }
        // Store result
        std::cout << "\tmovl\t%r8d, (%rdx)\n";
    }

    // ---------------------------------------------------------------
    // Loop increment and condition check
    // ---------------------------------------------------------------
    std::cout << "\taddq\t$1, %rax\n";       // i++
    std::cout << "\tcmpl\t%eax, (%rdi)\n";   // if (size > i) loop
    std::cout << "\tjg\t.L3\n";

    // ---------------------------------------------------------------
    // Function epilogue
    // ---------------------------------------------------------------
    std::cout << ".L1:\n";
    std::cout << "\tret\n";
    std::cout << "\t.cfi_endproc\n";
    std::cout << ".LFE0:\n";
    std::cout << "\t.size\t" << sym << ", .-" << sym << "\n";

    // GNU stack note (marks stack as non-executable — standard for Linux)
    std::cout << "\t.section\t.note.GNU-stack,\"\",@progbits\n";
    std::cout << "\t.section\t.note.gnu.property,\"a\"\n";
    std::cout << "\t.align 8\n";
    std::cout << "\t.long\t1f - 0f\n";
    std::cout << "\t.long\t4f - 1f\n";
    std::cout << "\t.long\t5\n";
    std::cout << "0:\n";
    std::cout << "\t.string\t\"GNU\"\n";
    std::cout << "1:\n";
    std::cout << "\t.align 8\n";
    std::cout << "\t.long\t0xc0000002\n";
    std::cout << "\t.long\t3f - 2f\n";
    std::cout << "2:\n";
    std::cout << "\t.long\t0x3\n";
    std::cout << "3:\n";
    std::cout << "\t.align 8\n";
    std::cout << "4:\n";

    return 0;
}
