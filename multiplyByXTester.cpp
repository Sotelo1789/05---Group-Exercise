#include <iostream>
#include "IntArray.h"

extern void multiplyByX(IntArray *p, int x);

int main()
{
    // Create a new IntArray with our own elements
    int n = 5;
    IntArray arr;
    arr.size = n;
    arr.elements = new int[n]{1, 2, 3, 4, 5};

    // Print original array
    std::cout << "Original array: ";
    for (int i = 0; i < arr.size; i++) {
        std::cout << arr.elements[i];
        if (i < arr.size - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // Multiply by 7
    int x = 7;
    multiplyByX(&arr, x);

    // Print result
    std::cout << "After multiplying by " << x << ": ";
    for (int i = 0; i < arr.size; i++) {
        std::cout << arr.elements[i];
        if (i < arr.size - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // Deallocate memory
    delete[] arr.elements;

    return 0;
}
