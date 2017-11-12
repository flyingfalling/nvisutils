
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards
#include <visdefs.hpp>
#include <string_tokenizer.hpp>

#include "dendmaker.hpp"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

int main( int argc, char*[] argv )
{
  std::string fname = "testspks/circuit0.circuit.neurons";

  std::ifstream ifs( fname );
  for( std::string line : ifs )
    {
      fprintf(stdout, "LINE: [%s]\n", line.c_str() );
    }
}
