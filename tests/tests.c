#include "unity.h"   //unity testing framework
#include <stddef.h>  //null
#include <unistd.h>  //sbrk 
#include <stdint.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

void test_malloc_non_void_return(void){
    void *ptr = malloc(5);
    TEST_ASSERT_NOT_NULL(ptr);
}

void test_malloc_program_break_advance(void){
    void *before = sbrk(0);
    void *newMemory = malloc(50);
    void *after = sbrk(0);

    TEST_ASSERT_NOT_NULL(newMemory);
    TEST_ASSERT_EQUAL_PTR(before, newMemory);
    TEST_ASSERT_EQUAL_PTR((char*)before + 50, after);
}

int main(void){
    UNITY_BEGIN();
    RUN_TEST(test_malloc_non_void_return);
    RUN_TEST(test_malloc_program_break_advance);
    return UNITY_END();
}