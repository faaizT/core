#include <iostream>
#include <typeinfo>

#include "RewardFromAntibiotics.h"

int main( int argc, char* argv[] )
{
  // RewardFromAntibiotics("", 500.0, 10.0, "none");
  // std::thread sepsis_thread(simulate_mimic,"/Users/faaiz/exportdir", 4201.0, ".");
  // sepsis_thread.join();
  simulate_mimic("/Users/faaiz/exportdir", 4201.0, ".");
  return 0;
}