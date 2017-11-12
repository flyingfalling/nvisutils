#pragma once

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards

#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>

#include <cstdio>
#include <iostream>

#include <visdefs.hpp>
#include <byte4color.hpp>
#include <string_tokenizer.hpp>


//Property includes SHAPE and ROTATION.
//For some things I need to set a "permanent" property. E.g. position of neuron. These are not events. I also need to set e.g. shape. Shape doesnt change for events.
//How do I set in the first place, e.g. from a config file. Same as "setting" the way that events are handled! It always convolves that with the event time ;)
//Allow it to conovlve with event at time "X". Which will set property now. Exp won't work, be careful rofl. Lin too.

//OK, set permanent properties in the same way. Create the property by parsing X/Y/Z/shape/color/etc.. Fuck rotation for now.

//Now everything is set, as properties, in the tree. Note, I need to build the tree. To do that, I go through the list of objects. Objects have types, positions, (sizes?) etc. That is static info.
//What is the object is a static thing, I need to make sure to create the correct property using "object creation" using start and end points. OK good. So, rotation is not a thing for events, which is fine.

//REV: I think I have everything. I need to specify everything. Need to know how I get "item" in the node thing. E.g. neurons blah. Specify "raw" numbers (items).

// Returns a quaternion such that q*start = dest
glm::quat RotationBetweenVectors2(glm::vec3 start, glm::vec3 dest)
{
  start = glm::normalize(start);
  dest = glm::normalize(dest);
  
  float cosTheta = glm::dot(start, dest);
  glm::vec3 rotationAxis;
  
  if (cosTheta < (-1 + 0.001f))
    {
      //fprintf( stdout, "I should be here!\n" );
      // special case when vectors in opposite directions :
      // there is no "ideal" rotation axis
      // So guess one; any will do as long as it's perpendicular to start
      // This implementation favors a rotation around the Up axis,
      // since it's often what you want to do.
      rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);

      if (length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
	{
	  rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
	}
      
      rotationAxis = glm::normalize(rotationAxis);
      /*
      float s = 1e-10; //sqrt( (1+cosTheta)*2 );
      float invs = 1 / s;
      
      return quat(
		  s * 0.5f, 
		  rotationAxis.x * invs,
		  rotationAxis.y * invs,
		  rotationAxis.z * invs
		  );
      */
      //REV: rofl, he was using 180f degrees but it had to be fucking PI
      return angleAxis( (float)M_PI, rotationAxis ); //180.0f, rotationAxis);
    }
  
  // Implementation from Stan Melax's Game Programming Gems 1 article
  rotationAxis = glm::cross(start, dest);
  //rotationAxis = normalize(rotationAxis );
  
  
  float s = sqrt( (1+cosTheta)*2 );
  float invs = 1 / s;
  
  return glm::quat(
	      s * 0.5f, 
	      rotationAxis.x * invs,
	      rotationAxis.y * invs,
	      rotationAxis.z * invs
	      );
}

float euclid(const glm::vec3& v1, const glm::vec3& v2 )
{
  float x = (v2.x-v1.x);
  float y = (v2.y-v1.y);
  float z = (v2.z-v1.z);
  return sqrt( (x*x) + (y*y) + (z*z) );
}

struct property
{
  byte4color* color = NULL;

  //REV: these 4 could be stored as a single transform.
  glm::mat4* rotation = NULL;
  //glm::mat4* scaling = NULL;
  //glm::mat4* translation = NULL;
  glm::vec3* scaling = NULL;
  glm::vec3* translation = NULL;
  
  shape_enum* shape = NULL;

  op_enum colorop = op_enum::SET;
  op_enum transop = op_enum::SET;
  op_enum scaleop = op_enum::SET;

  bool ispermanent=true;
  
  property()
      : color(NULL), rotation(NULL), translation(NULL), scaling(NULL), shape(NULL)
  {}

  
  void enumerate( )
  {
    fprintf(stdout, "PROPERTY:\n");
    if( color )
      {
	fprintf(stdout, "\tCOLOR:\n");
	fprintf(stdout, "\t\t[%d] [%d] [%d] [%d]\n", color->r, color->g, color->b, color->a );
      }
    if( scaling )
      {
	fprintf(stdout, "\tSCALING\n");
	fprintf(stdout, "\t\t[%f] [%f] [%f]\n", scaling->x, scaling->y, scaling->z );
      }
    if( translation)
      {
	fprintf(stdout, "\tTRANSLATION\n");
	fprintf(stdout, "\t\t[%f] [%f] [%f]\n", translation->x, translation->y, translation->z );
      }
    
  }
  
  void set_color( const byte4color& c )
  {
    if( color == NULL )
      {
	color = new byte4color( c );
      }
    else
      {
	*color = c;
      }
  }
  
  void set_rotation( const glm::mat4& r )
  {
    if( rotation == NULL )
      {
      	rotation = new glm::mat4( r );
      }
    else
      {
	*rotation = r;
      }
  }
  
  void set_scaling( const glm::vec3& s )
  {
    if( scaling == NULL )
      {
	scaling = new glm::vec3( s );
      }
    else
      {
	*scaling = s;
      }
  }
  
  void set_translation( const glm::vec3& t )
  {
    if( translation == NULL )
      {
	translation = new glm::vec3( t );
      }
    else
      {
	(*translation) = t;
      }
  }
  
  void set_shape( const shape_enum& s )
  {
    if( shape == NULL )
      {
	shape = new shape_enum( s );
      }
    else
      {
	*shape = s;
      }
    
  }


  glm::mat4 get_scaling()
  {
    if( scaling != NULL )
      {
	return glm::scale( glm::mat4(), *scaling );
      }
    else
      {
	return glm::mat4();
      }
  }

  glm::mat4 get_rotation()
  {
    if( rotation != NULL )
      {
	return *rotation;
      }
    else
      {
	return glm::mat4();
      }
  }

  glm::mat4 get_translation()
  {
    if( translation != NULL )
      {
	//fprintf(stdout, "Returning translation: %lf %lf %lf\n", translation->x, translation->y, translation->z );
	return glm::translate( glm::mat4(), *translation );
      }
    else
      {
	return glm::mat4();
      }
  }

  glm::mat4 get_model_matrix()
  {
    //T * R * S
    return ( get_translation() * get_rotation() * get_scaling() );
  }
  
  void create( const glm::vec3& point1, const glm::vec3& point2, const float& width, const float& depth )
  {
    float height = euclid( point1, point2 );
    scaling = new glm::vec3( width/2, height/2, depth/2 );
    glm::vec3 objectvect = point2-point1;
    glm::vec3 halfobjectvect = objectvect/2.0f;
    translation = new glm::vec3( point1 + halfobjectvect );

    //This should be normalized?
    rotation = new glm::mat4( mat4_cast( RotationBetweenVectors2( glm::vec3(0.0, 1.0, 0.0), objectvect ) ) );
  }
  
  //~property()
  void wreck()
  {
    if( color != NULL )
      {
	delete color;
      }
    if( rotation != NULL )
      {
	delete rotation;
      }
    if( scaling != NULL )
      {
	delete scaling;
      }
    if( translation != NULL )
      {
	delete translation;
      }
    if( shape != NULL )
      {
	delete shape;
      }
  }


  //property()
  //{}

  //Crap, need to set node (ID) and parents too....
  //REV: fuck, what do I do at an event time, if I want to do a "selection" type (glow) thing. Worry about that later, just change color now I guess...
  //REV: but, I really want to do that ugh. Same color, but more white? And more um, transparent? Fuck.
  //REV: have a literal copy of everything, all the time, but same size, and/or smaller, and 100% transparent? Make an option to 'select' or something.
  //REV: fuuuuuuuuuckkkkkkkk. How about add a tag to specify that it is "for selection" type thing? I like that. So, at rendertime, it ignores it, but instead it creates a copy of all atoms in the
  //glow matrix, scales them up a bit, and makes it transparent.
  //That's rather complex ;)

  void parse_position( const std::vector<std::string>& params, size_t& pos )
  {
    if( pos+8 > params.size() )
      {
	fprintf(stderr, "Not enough length in params\n");
	exit(1);
      }
    double p1x = doublefromstring( params[pos] );
    ++pos;
    double p1y = doublefromstring( params[pos] );
    ++pos;
    double p1z = doublefromstring( params[pos] );
    ++pos;
    double p2x = doublefromstring( params[pos] );
    ++pos;
    double p2y = doublefromstring( params[pos] );
    ++pos;
    double p2z = doublefromstring( params[pos] );
    ++pos;

    double width = doublefromstring( params[pos] );
    ++pos;
    double depth = doublefromstring( params[pos] );
    ++pos;
    
    glm::vec3 point1( p1x, p1y, p1z );
    glm::vec3 point2( p2x, p2y, p2z );

    create( point1, point2, width, depth );
  }

  void parse_color( const std::vector<std::string>& params, size_t& pos )
  {
    if( pos+3 > params.size() )
      {
	fprintf(stderr, "Not enough length in params\n");
	exit(1);
      }
    double r = doublefromstring( params[pos] );
    ++pos;
    double g = doublefromstring( params[pos] );
    ++pos;
    double b = doublefromstring( params[pos] );
    ++pos;

    //Assume it's never transparent...
    set_color( byte4color( glm::vec3( r,g,b ) ) );
  }

  void parse_scaling( const std::vector<std::string>& params, size_t& pos )
  {
    if( pos+3 > params.size() )
      {
	fprintf(stderr, "Not enough length in params for scaling\n");
	exit(1);
      }
    double x = doublefromstring( params[pos+0] );
    double y = doublefromstring( params[pos+1] );
    double z = doublefromstring( params[pos+2] );
    pos+=3;

    set_scaling( glm::vec3( x,y,z) );
  }
  
  void parse_shape( const std::vector<std::string>& params, size_t& pos )
  {
    
    if( params[pos].compare( "sphere" ) == 0 )
      {
	//fprintf(stdout, "SETTING SHAPE TO SPHERE\n" );
	set_shape( shape_enum::SPHERE );
      }
    else if( params[pos].compare( "cylinder" ) == 0 )
      {
	//fprintf(stdout, "SETTING SHAPE TO CYLINDER\n" );
	set_shape( shape_enum::CYLINDER );
      }
    else if( params[pos].compare( "cube" ) == 0 )
      {
	//fprintf(stdout, "SETTING SHAPE TO CUBE\n" );
	set_shape( shape_enum::CUBE );
      }
    else
      {
	fprintf(stderr, "Unrecognized shape [%s]\n", params[pos].c_str() );
	exit(1);
      }
    //fprintf(stdout, "Successfully parsed shape\n");
    ++pos;
  }
  
  property( const std::string& params )
  {
    std::vector<std::string> paramsv = tokenize_string( params, " " );
    initproperty( paramsv );
  }

  property( const std::vector<std::string>& paramsv )
  {
    initproperty( paramsv );
  }
  
  //REV: need start/end times
  void initproperty( const std::vector<std::string>& paramsv )
  {
    //pos1, pos2, width, depth.
    size_t pos=0;
    while( pos < paramsv.size() )
      {
	if( paramsv[pos].compare( "pos" ) == 0 )
	  {
	    ++pos;
	    parse_position( paramsv, pos );
	  }
	else if( paramsv[pos].compare( "color") == 0 )
	  {
	    ++pos;
	    parse_color( paramsv, pos );
	  }
	else if( paramsv[pos].compare( "shape") == 0 )
	  {
	    ++pos;
	    parse_shape( paramsv, pos );
	  }
	else if( paramsv[pos].compare( "scaling") == 0 )
	  {
	    ++pos;
	    parse_scaling( paramsv, pos );
	  }
	else
	  {
	    fprintf(stderr, "Error in ctor of property, unknown option [%s]\n", paramsv[pos].c_str() );
	    exit(1);
	  }
      }
    
  }
}; //END struct property



//REV: make a way to load/set settings permanently, but problem is, it needs to have each setting for each guy, can't say "look to node for position, but keep size same"?
//REV: actually, I could, but it would be in a "group" level which would not propogate down to making the actual vertices, fuck.
//So, just specify all required things for each...

//How can I do "parts" of cells, I need to know which groups have which parts I guess. Allow them to add (later). Fine.


