#include "IntArray.h"

void multiplyByX(IntArray *p, int x)
{
    // multiply x with all elements of IntArray
    // (overwrite the previous elements)
    for (int i = 0; i < p->size; i++) {
        p->elements[i] *= x;
    }
}
