#include <iostream>
#include <typeinfo>

#include "RewardFromAntibiotics.h"

int main( int argc, char* argv[] )
{
  return RewardFromAntibiotics(argv[1], *argv[2], *argv[3]);
}