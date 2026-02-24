#include <iostream>
#include "IntArray.h"

extern void multiplyBy61(IntArray *p);

int main()
{
    int n = 5;
    IntArray arr;
    arr.size = n;
    arr.elements = new int[n]{1, 2, 3, 4, 5};

    std::cout << "Original array: ";
    for (int i = 0; i < arr.size; i++) {
        std::cout << arr.elements[i];
        if (i < arr.size - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    multiplyBy61(&arr);

    std::cout << "After multiplying by 61: ";
    for (int i = 0; i < arr.size; i++) {
        std::cout << arr.elements[i];
        if (i < arr.size - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // Verify: expected values are 61, 122, 183, 244, 305
    int expected[] = {61, 122, 183, 244, 305};
    bool correct = true;
    for (int i = 0; i < n; i++) {
        if (arr.elements[i] != expected[i]) {
            correct = false;
            std::cerr << "MISMATCH at index " << i << ": got " << arr.elements[i]
                      << ", expected " << expected[i] << std::endl;
        }
    }
    if (correct) std::cout << "All values correct!" << std::endl;

    delete[] arr.elements;
    return 0;
}
