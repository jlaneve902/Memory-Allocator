#include "unity.h"   //unity testing framework
#include <stddef.h>  //null
#include <unistd.h>  //sbrk 
#include <stdint.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}


//MALLOC TESTING
void test_malloc_non_void_return(void){
    void *ptr = malloc(5);
    TEST_ASSERT_NOT_NULL(ptr);
}

void test_malloc_program_break_advance(void){
    void *before = sbrk(0);
    void *newMemory = malloc(50);
    void *after = sbrk(0);

    TEST_ASSERT_NOT_NULL(newMemory);
    TEST_ASSERT_EQUAL_PTR((char*)before + 32, newMemory);
    TEST_ASSERT_EQUAL_PTR((char*)before + 82, after);
}

void test_malloc_zero_size(void){
    void *ptr = malloc(0);
    TEST_ASSERT_NULL(ptr);
}

int main(void){
    UNITY_BEGIN();

    //MALLOC TESTING
    RUN_TEST(test_malloc_non_void_return);
    RUN_TEST(test_malloc_program_break_advance);
    RUN_TEST(test_malloc_zero_size);

    return UNITY_END();
}