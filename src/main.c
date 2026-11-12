#include "aids.h"

int main() { 
    DA(int) ints;
    da_init(&ints, &DEFAULT_ALLOCATOR);
    da_append_many(&ints, 1, 2, 3);
    da_free_call(&ints, item, {
        printf("%d\n", *item);
    });
}
