#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "tests.h"
#include "verify.h"

std::vector<int>* member_vec = NULL;

void ctor_dtor_tests(int id)
{
    // print simple output
    global_barrier->arrive(id);
    if (id == 0)
        printf("Testing member vector constructors(10) and destructors(1)\n");

    // the first test is simple ctor and dtor
    //
    // NB: we haven't actually verified size yet, but we use it here.
    global_barrier->arrive(id);
    {
        verifier v;
        int size;
        BEGIN_TX;
        member_vec = new std::vector<int>();
        int size = member_vec->size();
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check_size("basic ctor(1) and dtor(1)", id, size);
    }
#if 0
    // the next test uses a custom allocator with ctor 1
    //
    // NB: we haven't actually verified size yet, but we use it here.
    global_barrier->arrive(id);
    {
        verifier v;
        int size;
        BEGIN_TX;
        member_deque = new std::deque<int>();
        // using get_allocator without checking it
        auto a = member_deque->get_allocator();
        delete(member_deque);
        member_deque = new std::deque<int>(a);
        size = member_deque->size();
        delete(member_deque);
        member_deque = NULL;
        END_TX;
        v.check_size("basic ctor(1) and dtor(1)", id, size);
    }
#endif
    // the next test will call the simple fill constructor and the fill
    // constructor with default value
    //
    // NB: we haven't actually verified iterators yet, but we use them in
    //     COPY_DEQUE
    global_barrier->arrive(id);
    {
        verifier v;
        BEGIN_TX;
        member_vec = new std::vector<int>(4);
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = new std::vector<int>(4, 98);
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("fill ctor (2a) and fill ctor (2b)", id, 8,
              { 0, 0, 0, 0, 98, 98, 98, 98, -2});
    }

    // the next test will use the range ctor
    global_barrier->arrive(id);
    {
        verifier v;
        int q [] = { 9, 8, 7 };
        BEGIN_TX;
        member_vec = new std::vector<int>(q, q+3);
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("range ctor (3)", id, 3, { 9, 8, 7, -2 });
   }
   // the next test will use the copy ctor
    global_barrier->arrive(id);
    {
        verifier v;
        std::vector<int> local = { 3, 4, 5, 6 };
        BEGIN_TX;
        member_vec = new std::vector<int>(local);
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("copy ctor (4a)", id, 4, {3, 4, 5, 6, -2});
    }
    
    // the next test is the move ctor
    global_barrier->arrive(id);
    {
        verifier v;
        std::vector<int> local = {5, 4, 3};
        BEGIN_TX;
        member_vec = new std::vector<int>(std::move(local));
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("move ctor (5a)", id, 3, {5, 4, 3, -2});
    }
    // the next test is the ilist ctor
    global_barrier->arrive(id);
    {
        verifier v;
        BEGIN_TX;
        member_vec = new std::vector<int>({11, 13, 15, 17});
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("move ilist ctor (6)", id, 4, {11, 13, 15, 17, -2});
    }
}

void op_eq_tests(int id)
{
    global_barrier->arrive(id);

    // a temporary array into which we can copy deque data
    int data[256], dsize;

    if (id == 0)
        printf("Testing operator= (3)\n");

    // test #1 is operator= copy
    //
    // NB: we haven't actually verified iterators yet, but we use them...
 /* This function hasn't been implementation
   global_barrier->arrive(id);
    {
        verifier v;
        std::vector<int> local = { 3, 4, 5, 6 };
        BEGIN_TX;
        member_vec = new std::vector<int>();
        *member_vec = local;
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("copy operator= (1)", id, 4, {3, 4, 5, 6, -2});
    }*/
    // test #2 is operator= move
    global_barrier->arrive(id);
    {
        verifier v;
        std::vector<int> local = { 9, 8, 7, 6 };
        BEGIN_TX;
        member_vec = new std::vector<int>();
        *member_vec = std::move(local);
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("move operator= (2)", id, 4, {9, 8, 7, 6, -2});
    }

    // test #3 is operator= ilist
    global_barrier->arrive(id);
    {
        verifier v;
        BEGIN_TX;
        member_vec = new std::vector<int>();
        *member_vec = { 13, 14, 15, 16 };
        v.insert_all<std::vector<int>>(member_vec);
        delete(member_vec);
        member_vec = NULL;
        END_TX;
        v.check("ilist operator= (3)", id, 4, {13, 14, 15, 16, -2});
    }
}
