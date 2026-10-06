#include "unity.h"   //unity testing framework
#include <stddef.h>  //null
#include <unistd.h>  //sbrk 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}


//Malloc() TESTING
void test_malloc_non_void_return(void){
    void *ptr = malloc(5);
    TEST_ASSERT_NOT_NULL(ptr);
    free(ptr);
}

void test_malloc_program_break_advance(void){
    void *before = sbrk(0);
    void *newMemory = malloc(50);
    void *after = sbrk(0);

    TEST_ASSERT_NOT_NULL(newMemory);
    TEST_ASSERT_EQUAL_PTR((char*)before + 32, newMemory);
    TEST_ASSERT_EQUAL_PTR((char*)before + 82, after);
    free(newMemory);
}

void test_malloc_zero_size(void){
    void *ptr = malloc(0);
    TEST_ASSERT_NULL(ptr);
    free(ptr);
}


//Free() Testing
void test_free_tail(void){
    void* a = malloc(30);
    void* firstSbrk = sbrk(0);
    void* second = malloc(30);
    void* secondSbrk = sbrk(0);

    TEST_ASSERT_NOT_EQUAL(firstSbrk, secondSbrk); //makes sure the first and second are indeed different
    free(second);
    secondSbrk = sbrk(0);
    TEST_ASSERT_EQUAL(firstSbrk, secondSbrk); //once second free, 1 and 2 should be equal
    free(a);
}

void test_free_middle_block_is_reused_and_heap_unchanged(void) {
    char *a = malloc(1000);
    char *b = malloc(1000);
    void *brk_after_b = sbrk(0);
    char *c = malloc(1000);            // c is the tail, so b is in the middle 
    void *brk_after_c = sbrk(0);

    free(b);                           // middle block: should only be marked free 
    TEST_ASSERT_EQUAL_PTR(brk_after_c, sbrk(0));   // heap did not shrink 

    char *d = malloc(500);             // fits in b's block, so first-fit reuses it 
    TEST_ASSERT_EQUAL_PTR(b, d);                   // same memory handed back 
    TEST_ASSERT_EQUAL_PTR(brk_after_c, sbrk(0));   // no new sbrk call 

    free(c);                           // c is still the tail: heap should shrink 
    TEST_ASSERT_EQUAL_PTR(brk_after_b, sbrk(0));   // list was intact, so tail removal worked

    free(d);
    free(a);
}

//Calloc() Testing
void test_calloc_zeroes_reused_dirty_block(void){
    char *a = malloc(1000);
    char *b = malloc(1000);
    char *c = malloc(1000);     // c is the tail, so freeing b only marks it free

    memset(b, 0xAB, 1000);     // dirty the block 
    free(b);

    char *z = calloc(10, 50);    // 500 bytes, fits b's block, so first-fit reuses it 
    TEST_ASSERT_NOT_NULL(z);
    TEST_ASSERT_EQUAL_PTR(b, z);   // confirms we really got the dirtied block 
    for (int i = 0; i < 500; i++)
        TEST_ASSERT_EQUAL_UINT8(0, (uint8_t)z[i]);

    free(c);
    free(z);
    free(a);
}

void test_calloc_overflow_returns_null(void) {
    TEST_ASSERT_NULL(calloc(SIZE_MAX, 2));   // product wraps, overflow check must catch it
    TEST_ASSERT_NULL(calloc(2, SIZE_MAX));
}

//Realloc Testing
void test_realloc_grow_preserves_contents(void) {
    char *p = malloc(16);
    for (int i = 0; i < 16; i++) p[i] = (char)(i + 1);

    char *q = realloc(p, 1000);  //too small to fit, so it moves
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_TRUE(q != p);
    for (int i = 0; i < 16; i++)
        TEST_ASSERT_EQUAL_CHAR((char)(i + 1), q[i]);

    free(q);
}

void test_realloc_smaller_returns_same_pointer(void) {
    char *p = malloc(100);
    memset(p, 'x', 100);

    char *q = realloc(p, 50);   //block already big enough: nothing to do
    TEST_ASSERT_EQUAL_PTR(p, q);
    TEST_ASSERT_EQUAL_CHAR('x', q[0]);
    TEST_ASSERT_EQUAL_CHAR('x', q[49]);

    free(q);
}

int main(void){
    UNITY_BEGIN();

    //9 Tests 0 Failures 0 Ignored OK

    //MALLOC TESTING
    RUN_TEST(test_malloc_non_void_return); 
    RUN_TEST(test_malloc_program_break_advance);
    RUN_TEST(test_malloc_zero_size);

    //FREE TESTING
    RUN_TEST(test_free_tail);
    RUN_TEST(test_free_middle_block_is_reused_and_heap_unchanged);

    //CALLOC TESTING
    RUN_TEST(test_calloc_zeroes_reused_dirty_block);
    RUN_TEST(test_calloc_overflow_returns_null);
    
    //REALLOC TESTING
    RUN_TEST(test_realloc_grow_preserves_contents);
    RUN_TEST(test_realloc_smaller_returns_same_pointer);

    return UNITY_END();
}