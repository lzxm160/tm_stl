/*
  Driver to test the transactional version of the std::string interface

  According to http://www.cplusplus.com/reference/string/string/, the
  std::string interface consists of the following:

  Steps:
    1 - Put TBD traces into all of GCC's string functions
    2 - For one category at a time, replace TBDs and update columns 3 and 4
        of table below
    3 - Write test code for ensuring that every traced function is called,
        and then write DONE in the category

  Note: we must work on the libstdc++-v3/ext/vstring.h sources, since GCC's
  <string> is currently non-conforming

|-------------------+-------------------+----------+--------|
| Category          | Functions         | C++14    | GCC    |
|                   |                   | Expected | Actual |
|-------------------+-------------------+----------+--------|
| Member            | (constructor)     |          |        |
| Functions         |                   |          |        |
|                   | (destructor)      |          |        |
|                   | operator=         |          |        |
|-------------------+-------------------+----------+--------|
| Iterators         | begin             |          |        |
|                   | end               |          |        |
|                   | rbegin            |          |        |
|                   | rend              |          |        |
|                   | cbegin            |          |        |
|                   | cend              |          |        |
|                   | crbegin           |          |        |
|                   | crend             |          |        |
|-------------------+-------------------+----------+--------|
| Iterator          | ctor              |          |        |
| Methods           | dtor              |          |        |
|                   | operator*         |          |        |
|                   | operator->        |          |        |
|                   | operator++        |          |        |
|                   | operator--        |          |        |
|                   | operator+=        |          |        |
|                   | operator+         |          |        |
|                   | operator-=        |          |        |
|                   | operator-         |          |        |
|                   | operator[]        |          |        |
|-------------------+-------------------+----------+--------|
| Iterator          | operator==        |          |        |
| Operators         | operator!=        |          |        |
|                   | operator<         |          |        |
|                   | operator>         |          |        |
|                   | operator<=        |          |        |
|                   | operator>=        |          |        |
|                   | operator-         |          |        |
|                   | operator+         |          |        |
|-------------------+-------------------+----------+--------|
| Iterator          | copy-assignable   |          |        |
| Overloads         | destructible      |          |        |
|                   | swappable         |          |        |
|-------------------+-------------------+----------+--------|
| Iterator          | fill              |          |        |
| Functions         | copy              |          |        |
|                   | copy_backward     |          |        |
|                   | move              |          |        |
|                   | move_backward     |          |        |
|-------------------+-------------------+----------+--------|
| Const             |                   |          |        |
| Iterator          |                   |          |        |
| Methods           |                   |          |        |
|-------------------+-------------------+----------+--------|
| Reverse           |                   |          |        |
| Iterator          |                   |          |        |
| Methods           |                   |          |        |
|-------------------+-------------------+----------+--------|
| Const Reverse     |                   |          |        |
| Iterator          |                   |          |        |
| Methods           |                   |          |        |
|-------------------+-------------------+----------+--------|
| Capacity          | size              |          |        |
|                   | length            |          |        |
|                   | max_size          |          |        |
|                   | resize            |          |        |
|                   | capacity          |          |        |
|                   | reserve           |          |        |
|                   | clear             |          |        |
|                   | empty             |          |        |
|                   | shrink_to_fit     |          |        |
|-------------------+-------------------+----------+--------|
| Element           | operator[]        |          |        |
| Access            | at                |          |        |
|                   | front             |          |        |
|                   | back              |          |        |
|-------------------+-------------------+----------+--------|
| Modifiers         | operator+=        |          |        |
|                   | append            |          |        |
|                   | push_back         |          |        |
|                   | assign            |          |        |
|                   | insert            |          |        |
|                   | erase             |          |        |
|                   | replace           |          |        |
|                   | swap              |          |        |
|                   | pop_back          |          |        |
|-------------------+-------------------+----------+--------|
| String            | c_str             |          |        |
| Operations        | data              |          |        |
|                   | get_allocator     |          |        |
|                   | copy              |          |        |
|                   | find              |          |        |
|                   | rfind             |          |        |
|                   | find_first_of     |          |        |
|                   | find_last_of      |          |        |
|                   | find_first_not_of |          |        |
|                   | find_last_not_of  |          |        |
|                   | substr            |          |        |
|                   | compare           |          |        |
|-------------------+-------------------+----------+--------|
| Non-member        | '=='              |          |        |
| function          | '!='              |          |        |
| overloads (NMFOs) | '<'               |          |        |
|                   | '<='              |          |        |
|                   | '>'               |          |        |
|                   | '>='              |          |        |
|                   | swap              |          |        |
|                   | operator+         |          |        |
|                   | operator>>        |          |        |
|                   | operator<<        |          |        |
|                   | getline           |          |        |
|                   | stoi              |          |        |
|                   | stol              |          |        |
|                   | stoul             |          |        |
|                   | stoll             |          |        |
|                   | stoull            |          |        |
|                   | stof              |          |        |
|                   | stod              |          |        |
|                   | stold             |          |        |
|                   | to_string         |          |        |
|-------------------+-------------------+----------+--------|
*/

#include <cstdio>
#include <thread>
#include <atomic>
#include <mutex>
#include <cassert>
#include <iostream>
#include <unistd.h>

#include "../common/barrier.h"
#include "tests.h"

using std::cout;
using std::endl;

/// configured via command line args: number of threads
int  num_threads = 1;

/// the barrier to use when we are in concurrent mode
barrier* global_barrier;

/// the mutex to use when we are in concurrent mode with tm turned off
std::mutex global_mutex;

/// Report on how to use the command line to configure this program
void usage()
{
    cout << "Command-Line Options:" << endl
         << "  -n <int> : specify the number of threads" << endl
         << "  -h       : display this message" << endl
         << "  -T       : enable all tests" << endl
         << "  -t <int> : enable a specific test" << endl
         << "               1 constructors and destructors" << endl
         << "               2 operator=" << endl
         << "               3 iterator creation" << endl
         << "               4 iterator methods" << endl
         << "               5 iterator operators" << endl
         << "               6 iterator overloads" << endl
         << "               7 iterator functions" << endl
         << "               8 capacity methods" << endl
         << "               9 element access methods" << endl
         << "              10 modifier methods" << endl
         << "              11 operation methods" << endl
         << "              12 overloads" << endl
         << endl
         << "  Note: const, reverse, and const reverse iterators not tested"
         << endl
         << endl;
    exit(0);
}

#define NUM_TESTS 14
bool test_flags[NUM_TESTS] = {false};

void (*test_names[NUM_TESTS])(int) = {
    NULL,
    ctor_dtor_tests,                                    // member.cc
    op_eq_tests,                                        // member.cc
    iter_create_tests,                                  // iter.cc
    iter_method_tests,                                  // iter.cc
    iter_operator_tests,                                // iter.cc
    iter_overload_tests,                                // iter.cc
    iter_function_tests,                                // iter.cc
    cap_tests,                                          // cap.cc
    element_tests,                                      // element.cc
    modifier_tests,                                     // modifier.cc
    operation_tests,                                    // operation.cc
    overload_tests                                      // overloads.cc
};

/// Parse command line arguments using getopt()
void parseargs(int argc, char** argv)
{
    // parse the command-line options
    int opt;
    while ((opt = getopt(argc, argv, "n:ht:")) != -1) {
        switch (opt) {
          case 'n': num_threads = atoi(optarg); break;
          case 'h': usage();                    break;
          case 't': test_flags[atoi(optarg)] = true; break;
          case 'T': for (int i = 0; i < NUM_TESTS; ++i) test_flags[i] = true; break;
        }
    }
}

/// A concurrent test for exercising every method.  This is called by every
/// thread
void per_thread_test(int id)
{
    // wait for all threads to be ready
    global_barrier->arrive(id);

    // run the tests that were requested on the command line
    for (int i = 0; i < NUM_TESTS; ++i)
        if (test_flags[i])
            test_names[i](id);
}

/// main() just parses arguments, makes a barrier, and starts threads
int main(int argc, char** argv)
{
    // figure out what we're doing
    parseargs(argc, argv);

    // set up the barrier
    global_barrier = new barrier(num_threads);

    // make threads
    std::thread* threads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; ++i)
        threads[i] = std::thread(per_thread_test, i);

    // wait for the threads to finish
    for (int i = 0; i < num_threads; ++i)
        threads[i].join();
}
