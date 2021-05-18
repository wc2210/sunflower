#include "base/threads.h"
#include <iostream>

class thread_test : public sunflower::thread
{
public:
  thread_test(const std::string& name) : thread(name){}
protected:
  void run_thread() override { 
    for (int i = 0; i < 1000000; i++) {}
    print_priority();

    std::string name_h = get_name();    
    std::string name_a = name_h + "_plus";
    set_name(name_a);
    
    std::cout << name_h << "##" << name_a << "##" << get_name() << std::endl;
  }
};

int main(){
  thread_test mytest1("cpu1_pri1");
  thread_test mytest2("cpu1_pri2");
  thread_test mytest3("cpu2_pri1");
  thread_test mytest4("cpu1000_pri1");

  mytest1.start_cpu(1, 1);
  mytest2.start_cpu(2, 1);
  mytest3.start_cpu(1, 2);
  mytest4.start_cpu_mask(1, 0x08);
    
  mytest1.wait_thread_finish();
  mytest2.wait_thread_finish();
  mytest3.wait_thread_finish();
  mytest4.wait_thread_finish();
  
  return 0;
}
