#include "gc_pointer.h"
#include "LeakTester.h"
#include <iostream>

using namespace std;


int main()
{
    Pointer<int> p = new int(19);
    p = new int(21);
    p = new int(28);
    cout << "start";

    return 0;
}
