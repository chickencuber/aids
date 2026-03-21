#include <stdio.h>
#include <aids.h>


HM(String, int) h;

int main() {
    printf("=== Arena Test ===\n");
    ArenaAllocator arena = arena_create(&DEFAULT_ALLOCATOR, 1024);

    int* numbers = ALLOC(&arena, 10 * sizeof(int));
    if(numbers) {
        for(int i=0;i<10;i++) numbers[i] = i*2;
        for(int i=0;i<10;i++) printf("%d ", numbers[i]);
        printf("\n");
    }

    arena_destroy(&arena);

    printf("=== Scratch Test ===\n");
    ScratchAllocator scratch = scratch_create(&DEFAULT_ALLOCATOR);

    char* s1 = ALLOC(&scratch, 20);
    strcpy(s1, "Hello Scratch!");
    char* s2 = ALLOC(&scratch, 50);
    strcpy(s2, "More allocations in the scratch arena.");

    printf("%s\n", s1);
    printf("%s\n", s2);

    scratch_destroy(&scratch);
    printf("Scratch allocator destroyed safely.\n");

    return 0;
}
