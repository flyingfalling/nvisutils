#pragma once


#include <sstream>
#include <iostream>

enum class op_enum { SET, ADD, SUB };
enum class shape_enum { SPHERE, CYLINDER, CUBE };


double doublefromstring( const std::string& s )
{
  std::istringstream iss(s);
  double d;
  iss >> d;
  //fprintf( stdout, "Got %lf from %s\n", d, s.c_str() );
  
  //return std::stod( s );
  return d;
}
