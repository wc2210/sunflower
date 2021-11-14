#include "base/Threads.h"
#include <iostream>

class thread_test : public sunflower::Thread
{
public:
    thread_test(const std::string &name) : Thread(name) {}

protected:
    void RunThread() override
    {
        for (int i = 0; i < 1000000; i++)
        {
        }

        std::string name_h = GetName();
        std::string name_a = name_h + "_plus";
        SetName(name_a);

        std::cout << name_h << "##" << name_a << "##" << GetName() << std::endl;
    }
};

int main()
{
    thread_test mytest1("1");
    thread_test mytest2("2");
    thread_test mytest3("3");
    thread_test mytest4("4");

    mytest1.Start();
    mytest2.Start();
    mytest3.Start();
    mytest4.Start();

    mytest1.WaitThreadFinish();
    mytest2.WaitThreadFinish();
    mytest3.WaitThreadFinish();
    mytest4.WaitThreadFinish();

    return 0;
}
