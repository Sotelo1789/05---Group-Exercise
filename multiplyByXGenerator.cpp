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

// This defines a function called getBits that takes one integer n and returns a 
// vector<int> — a list of integers. The list will contain the positions of every 
// bit that is set to 1 in the binary representation of n.

std::vector<int> getBits(int n) {
    std::vector<int> bits; // Creates an empty list called bits.
    // Loop from 0 to 31. We loop 32 times because a regular int is 32 bits wide. 
    // We're going to check every single bit position one at a time, from the least
    // significant (bit 0, worth 1) to the most significant (bit 31, worth 2,147,483,648).

    for (int i = 0; i < 32; i++) {
        if (n & (1 << i)) {
            bits.push_back(i);
        }
    }
    return bits;
    //  for 61 (00111101) the function returns {0, 2, 3, 4, 5}
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
    //If the user didn't provide exactly one argument (making argc exactly 2), print an 
    //error message to cerr (the error output stream) and exit with return code 1. 
    //Return code 1 tells the operating system something went wrong. Return code 0 
    //means success.

    int constant = std::atoi(argv[1]);
    // argv[1] is the string "61". atoi (ASCII to integer) converts it to the actual 
    // integer 61. If the user typed letters instead of a number, atoi returns 0.

    if (constant <= 0) {
        std::cerr << "Error: constant must be a positive integer." << std::endl;
        return 1;
    }
    // Reject 0 (which also catches the case where atoi failed because of non-numeric input) 
    // and any negative numbers. Our shift-and-add algorithm only works for positive integers.


    std::string funcName = "multiplyBy" + std::to_string(constant);
    std::string sym = mangledName(constant);
    std::vector<int> bits = getBits(constant);
    // Three setup lines before we start printing assembly:
    // funcName — the human readable name like "multiplyBy61". Used in the .file directive.
    //
    // sym — the mangled symbol name like "_Z12multiplyBy61P8IntArray". Used everywhere the 
    // symbol appears in the assembly.
    //
    // bits — the list of set bit positions from getBits. For 61 this 
    // is {0, 2, 3, 4, 5}. This drives the entire multiplication logic below.

    // ---------------------------------------------------------------
    // Emit assembly header
    // ---------------------------------------------------------------
    std::cout << "\t.file\t\"" << funcName << ".cpp\"\n"; // Prints the .file directive
    std::cout << "\t.text\n";                             // Prints the .text directive
    std::cout << "\t.globl\t" << sym << "\n";             // Prints the .globl directive
    std::cout << "\t.type\t" << sym << ", @function\n";   // Prints the .type directive
    std::cout << sym << ":\n";                            // Prints the function label 
    std::cout << ".LFB0:\n";                              // Prints the three opening 
    std::cout << "\t.cfi_startproc\n";                    // lines of the function body
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
    //
    //
    // ---------------------------------------------------------------

    // Check if size <= 0, skip loop entirely
    // Prints the size check and loop counter initialization
    // they check if the array is empty and set up %eax as the loop counter.
    std::cout << "\tcmpl\t$0, (%rdi)\n";
    std::cout << "\tjle\t.L1\n";
    std::cout << "\tmovl\t$0, %eax\n";       // i = 0

    // ---------------------------------------------------------------
    // Loop body
    // ---------------------------------------------------------------
    
    // Prints the top of the loop
    // Loads the elements pointer, calculates the address of elements[i], and loads 
    // the current element value into %ecx. After this point %ecx holds the value 
    // we need to multiply.

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

    // This is the only part of the generator that varies based on the constant. 
    // Everything before this was fixed boilerplate.


    // Check if only ONE bit is set in the constant. If so, the constant is a 
    // power of 2 and we can handle it with a single shift

    if (bits.size() == 1) {
        // Power of 2 — single shift suffices
        int shift = bits[0]; // Get the position of that single set bit.
        if (shift == 0) {
            // multiply by 1: no-op, value stays in %ecx
        } else {
            // generate a single left shift instruction. 
            // sall = Shift Arithmetic Left Long. $ before the number means
            // it's a literal value. So for constant 8 (shift = 3) this prints:
            // sall	$3, %ecx
            std::cout << "\tsall\t$" << shift << ", %ecx\n";
        }
        // Store the result from %ecx back into elements[i] in memory.
        std::cout << "\tmovl\t%ecx, (%rdx)\n"; 
    } else {
        // General case: sum of shifted copies
        // We'll use %r8d as the running sum, %ecx as the base value.
        // For the first set bit, initialize %r8d.

        //first is a flag to handle the first set bit differently from all subsequent ones. 
        // For the first bit we initialize the accumulator %r8d. For all following bits we 
        // add to it. We need this distinction because you can't add to %r8d before it has 
        // an initial value.

        bool first = true;
        for (int bit : bits) { // Loop through each set bit position in our list. For 61 its {0, 2, 3, 4, 5}.
            if (first) {
                first = false; // First time through — set the flag to false so subsequent iterations take the else branch.
                if (bit == 0) {
                    // partial = ecx << 0 = ecx; r8d = ecx
                    // If the first set bit is bit 0, initialize the accumulator
                    // with the unshifted value. No shift needed because 2⁰ = 1 
                    // and shifting left by 0 changes nothing. For 61, bit 0 IS
                    //  the first bit so this prints: 	movl	%ecx, %r8d
                    std::cout << "\tmovl\t%ecx, %r8d\n";
                } else {
                    // r8d = ecx << bit
                    // If the first set bit is NOT bit 0, initialize the accumulator with a 
                    // shifted copy. Copy %ecx to %r8d then shift it left by bit positions.

                    std::cout << "\tmovl\t%ecx, %r8d\n";
                    std::cout << "\tsall\t$" << bit << ", %r8d\n";
                }
            } else {
                // All subsequent iterations — we already have a value in %r8d, 
                // now we add more shifted copies to it.

                // partial = ecx << bit, then r8d += partial
                // Use a temp register %r9d for the shifted value
                if (bit == 0) {
                    // If this set bit is bit 0, just add the unshifted original value 
                    // directly to the accumulator. No separate temp register needed 
                    // since no shift is required. Prints: 	addl	%ecx, %r8d

                    std::cout << "\taddl\t%ecx, %r8d\n";
                } else {
                    // For any other set bit, copy the original value to the temp register 
                    // %r9d, shift it left by bit positions (multiplying by 2^bit), 
                    // then add the result to the accumulator %r8d.

                    std::cout << "\tmovl\t%ecx, %r9d\n";
                    std::cout << "\tsall\t$" << bit << ", %r9d\n";
                    std::cout << "\taddl\t%r9d, %r8d\n";
                }
            }
        }
        // Store result
        // After all bits are processed, %r8d holds the final product. 
        // Store it back into elements[i] in memory, overwriting the original value.

        std::cout << "\tmovl\t%r8d, (%rdx)\n";
    }

    // ---------------------------------------------------------------
    // Loop increment and condition check
    // ---------------------------------------------------------------
    // Prints the three loop control lines 
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

 // # Step 1: Build and test Part A
 // g++ -O -S multiplyByX.cpp -o multiplyByX.s
 // g++ multiplyByXTester.cpp multiplyByX.s -o multiplyByXTester.out
 // ./multiplyByXTester.out
 
 // # Step 2: Build generator
 // g++ multiplyByXGenerator.cpp -o multiplyByXGenerator.out
 
 // # Step 3: Generate and test multiplyBy61
 // ./multiplyByXGenerator.out 61 > multiplyBy61.s
 // g++ multiplyBy61Tester.cpp multiplyBy61.s -o multiplyBy61Tester.out
 // ./multiplyBy61Tester.out
 
 // # Step 4: Confirm no imul
 // grep 'imul' multiplyBy61.s && echo 'FAIL: imul found' || echo 'PASS: no imul'