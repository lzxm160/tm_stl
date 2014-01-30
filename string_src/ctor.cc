#include <iostream>
#include <string>
#include <list>
#include <cassert>
#include "concurrent_tests.h"

/// The list we will use for our tests
std::string* ctor_string = NULL;

/**
 * Here is the ctor test.  It also happens to be the dtor test,
 * since there is only a basic dtor.
 */
void ctor_test(int id)
{
    // invariant: the list starts off null
    assert(ctor_string == NULL);

    // helper message
    if (id == 0) {
        std::cout << std::endl
                  << "Testing ctor and dtor" << std::endl
                  << "(Note: we aren't testing the allocator parameter "
                  << "of the constructors)" << std::endl;
    }

    // test basic constructor (1)
    //
    // make an empty string and verify that its length is zero... note that
    // we haven't actually tested length yet, but it's OK
    global_barrier->arrive(id);
    bool OK = false;
    BEGIN_TX;
    // call the basic constructor, expect empty string
    ctor_string = new std::string();
    OK = (ctor_string->length() == 0);
    delete ctor_string;
    END_TX;
    if (OK)
        std::cout << "[OK] basic ctor(1)" << std::endl;
    else
        std::cout << "basic ctor(1) error" << std::endl;

    global_barrier->arrive(id);

    // NB: GCC has a string constructor that takes an allocator as a
    // parameter.  We don't test it, because it's not in the spec.

    // test copy constructor (2)
    global_barrier->arrive(id);
    std::string mine = "hello world";
    std::string third;
    BEGIN_TX;
    ctor_string = new std::string(mine);
    third = std::string(*ctor_string);
    delete ctor_string;
    END_TX;
    if (third.compare(mine) == 0)
        std::cout << "[OK] copy ctor(2)" << std::endl;
    else
        std::cout << "copy ctor(2) error" << std::endl;

#if 0
    // scratch space for holding data when computing whether the test
    // succeeded or failed.
    int data[32], dsize = 0;


    // test sized constructor (2)
    global_barrier->arrive(id);
    // make sure we'll detect errors in data[]
    for (int i = 0; i < 16; ++i)
        data[i] = 17;
    BEGIN_TX;
    // call the sized constructor, expect {0,0,0,0,0}
    // NB: we padded data[] to detect overflow
    ctor_list = new std::list<int>(5);
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    VERIFY("sized ctor (2a)", 5, {0, 0, 0, 0, 0, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17});

    // test sized constructor (2b)
    global_barrier->arrive(id);
    // make sure we'll detect errors in data[]
    for (int i = 0; i < 16; ++i)
        data[i] = 17;
    BEGIN_TX;
    // call the sized constructor, expect {0,0,0,0,0}
    // NB: we padded data[] to detect overflow
    ctor_list = new std::list<int>(5, 9);
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    VERIFY("sized ctor (2b)", 5, {9, 9, 9, 9, 9, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17});

    // test range constructor (3)
    global_barrier->arrive(id);
    int a[] = {1, 2, 3};
    BEGIN_TX;
    // use the range constructor, expect {1, 2, 3}
    ctor_list = new std::list<int>(a, a+3);
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    VERIFY("range ctor (3)", 3, {1, 2, 3});

    // test the copy constructor (4)
    global_barrier->arrive(id);
    int b[] = {7, 8, 9, 2};
    if (per_thread_list[id] == NULL) // always true
        per_thread_list[id] = new std::list<int>({b, b+4});
    BEGIN_TX;
    ctor_list = new std::list<int>(*per_thread_list[id]);
    delete per_thread_list[id];
    per_thread_list[id] = NULL;
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    VERIFY("copy ctor  (4)", 4, {7, 8, 9, 2});

    // test the move constructor (5)
    global_barrier->arrive(id);
    if (per_thread_list[id] == NULL) // always true
        per_thread_list[id] = new std::list<int>({a, a+3});
    int tmp;
    BEGIN_TX;
    // Possible GCC bug... nulling the null list before we start causes an
    // early write, without which we get concurrency errors
    if (id < 256)
        ctor_list = NULL;
    ctor_list = new std::list<int>(std::move(*per_thread_list[id]));
    tmp = per_thread_list[id]->size(); // expect 0
    delete per_thread_list[id];
    per_thread_list[id] = NULL;
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    assert(tmp == 0);
    VERIFY("move ctor  (5)", 3, {1, 2, 3});

    // test the initializer list constructor (6)
    global_barrier->arrive(id);
    BEGIN_TX;
    ctor_list = new std::list<int>({6, 3, 8, 2, 9, 1});
    COPY_LIST;
    CLEAR_LIST;
    END_TX;
    VERIFY("ilist ctor (6)", 6, {6, 3, 8, 2, 9, 1});

    global_barrier->arrive(id);
#endif
}

#if 0
/// clone the list to a local array represented by dsize, data[]
#define COPY_LIST                                 \
    dsize = 0;                                    \
    for (auto i : *ctor_list)                     \
        data[dsize++] = i

/// destroy the list and set the pointer to NULL
#define CLEAR_LIST                                \
    delete ctor_list;                             \
    ctor_list = NULL

/// compare the local array to our expected results
#define VERIFY(testName, size, ...)                                     \
    if (dsize != size)                                                  \
        printf(" [%d] size did not match %d != %d\n", id, dsize, size);  \
    else if (size > 0) {                                                \
        int counter = 0;                                                \
        bool good = true;                                               \
        for (auto i : __VA_ARGS__)                                      \
            good &= (data[counter++] == i);                             \
        if (!good)                                                      \
            std::cout << " ["<<id<<"] Arrays did not match for thread " << id << std::endl; \
        else if (id == 0)                                               \
            std::cout << " [OK] " << testName << std::endl;              \
    }                                                                   \
    else if (id == 0)                                                   \
            std::cout << " [OK] " << testName << std::endl

#endif
