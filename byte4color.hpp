#pragma once

#include <glm/glm.hpp>
#include <cstdlib>


struct byte4color
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a=255; //REV: will this work?


  byte4color()
  {
    r=0, b=0, g=0;
  }
  
  byte4color( const unsigned char& rval,
	      const unsigned char& gval,
	      const unsigned char& bval )
  {
    r = rval;
    g = gval;
    b = bval;
  }

  byte4color( const unsigned char& rval,
	      const unsigned char& gval,
	      const unsigned char& bval,
	      const unsigned char& aval)
  {
    r = rval;
    g = gval;
    b = bval;
    a = aval;
  }

  byte4color( const glm::vec3& colorvec )
  {
    r = (unsigned char)(colorvec.x * 255.0f); //meh. should be floor..?
    g = (unsigned char)(colorvec.y * 255.0f); //meh. should be floor..?
    b = (unsigned char)(colorvec.z * 255.0f); //meh. should be floor..?
  }

  byte4color( const glm::vec4& colorvec )
  {
    r = (unsigned char)(colorvec.x * 255.0f); //meh. should be floor..?
    g = (unsigned char)(colorvec.y * 255.0f); //meh. should be floor..?
    b = (unsigned char)(colorvec.z * 255.0f); //meh. should be floor..?
    a = (unsigned char)(colorvec.w * 255.0f); //meh. should be floor..?
  }

  byte4color& operator-=(const byte4color& rhs) // compound assignment (does not need to be a member,
  {                           // but often is, to modify the private members)
    /* addition of rhs to *this takes place here */
    if( rhs.r > r ) {r=0;} else{r -= rhs.r;}
    if( rhs.g > g ) {g=0;} else{g -= rhs.g;}
    if( rhs.b > b ) {b=0;} else{b -= rhs.b;}
    if( rhs.a > a ) {a=0;} else{a -= rhs.a;}
    return *this; // return the result by reference
  }
  
  byte4color& operator+=(const byte4color& rhs) // compound assignment (does not need to be a member,
  {                           // but often is, to modify the private members)
    /* addition of rhs to *this takes place here */
    if( 0xFF - rhs.r >= r ) { r += rhs.r; } else { r=0xFF; }
    if( 0xFF - rhs.g >= g ) { g += rhs.g; } else { g=0xFF; }
    if( 0xFF - rhs.b >= b ) { b += rhs.b; } else { b=0xFF; }
    if( 0xFF - rhs.a >= a ) { a += rhs.a; } else { a=0xFF; }
    return *this; // return the result by reference
  }
  
  // friends defined inside class body are inline and are hidden from non-ADL lookup
  friend byte4color operator+(byte4color lhs,        // passing lhs by value helps optimize chained a+b+c
		       const byte4color& rhs) // otherwise, both parameters may be const references
  {
    lhs += rhs; // reuse compound assignment
    return lhs; // return the result by value (uses move constructor)
  }

  // friends defined inside class body are inline and are hidden from non-ADL lookup
  friend byte4color operator-(byte4color lhs,        // passing lhs by value helps optimize chained a+b+c
		       const byte4color& rhs) // otherwise, both parameters may be const references
  {
    lhs -= rhs; // reuse compound assignment
    return lhs; // return the result by value (uses move constructor)
  }
};
