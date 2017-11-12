
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <random>


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>



//lol dynamic programming from source to each. We never have loops or multiple paths from root to it (i.e. it is a real tree)
//REV: need a graph type. A list of vertices.
struct treegraph
{
  std::vector<glm::vec3> nodelocs;
  std::vector<glm::size_t> pair1;
  std::vector<glm::size_t> pair2;
  std::vector<float> dists_to_root;
  
  //Only care about distance from root to myself...
  treegraph( const glm::vec3& startpos=glm::vec3(0,0,0) )
  {
    //nodelocs.push_back( glm::vec3(0,0,0) );
    nodelocs.push_back( startpos );
    dists_to_root.push_back( 0.0 );
  }
  
  void add_edge( const size_t& edge1, const glm::vec3& newnodeloc )
  {
    pair1.push_back( edge1 );
    pair2.push_back( nodelocs.size() );
    nodelocs.push_back( newnodeloc );
    float prevdist = get_dist_to_root( edge1 );
    float newdist = euclid( nodelocs[edge1] , newnodeloc );
    dists_to_root.push_back( prevdist + newdist );
  }
  
  float get_dist_to_root( const size_t& nodeidx )
  {
    if(nodeidx > dists_to_root.size() )
      {
	fprintf(stderr, "ERROR dist to root requested node idx too large\n");
	exit(1);
      }
    return dists_to_root[nodeidx];
  }

  //given that I add a new node, which node is closest to it.
  std::vector<float> dists_to_new_node( const glm::vec3& newnode )
  {
    std::vector<float> ret( nodelocs.size() );
    for(size_t x=0; x<nodelocs.size(); ++x)
      {
	ret[x] = euclid( newnode, nodelocs[x] );
      }
    return ret;
  }


  std::vector<float> pathlengths_to_new_node( const glm::vec3& newnode )
  {
    std::vector<float> ret( nodelocs.size() );
    for(size_t x=0; x<nodelocs.size(); ++x)
      {
	ret[x] = euclid( newnode, nodelocs[x] ) +
	  get_dist_to_root( x );
      }
    return ret;
  }

  std::vector<size_t> get_closest_for_bf( const glm::vec3& newnode, const float& bf )
  {
    std::vector<float> pl = pathlengths_to_new_node( newnode );
    std::vector<float> d = dists_to_new_node( newnode );
    std::vector<size_t> ret;
    
    float curr_min = d[0] + (bf*pl[0]);
    ret.push_back(0);

    for(size_t x=1; x<nodelocs.size(); ++x)
      {
	float mycost = (d[x] + (bf*pl[x]));
	if( mycost < curr_min )
	  {
	    curr_min = mycost;
	    ret.resize(1);
	    ret[0] = x;
	  }
	else if( mycost == curr_min )
	  {
	    ret.push_back( x );
	  }
	//else nothing
      }
    return ret;
  }
};





//REV: todo: generate carrier points realistically...
std::vector<glm::vec3> build_dend_tree_carrier_points( const std::vector<glm::vec3>& carrier_points, const float& bf, const glm::vec3& startpos )
{
  //Keep list of un-connected carrier points? Use them inline?
  treegraph mygraph( startpos );
  
  //while there are still carrier points left
  std::vector<bool> availcarrierpts( carrier_points.size(), true );
  size_t used=0;
  
  while( used < carrier_points.size() )
    {
      for( size_t c=0; c<carrier_points.size(); ++c)
	{
	  if( availcarrierpts[c] )
	    {
	      std::vector<size_t> best = mygraph.get_closest_for_bf( carrier_points[c], bf );

	      //assume only 1. Really, randomly select between if > 1
	      mygraph.add_edge( best[0], carrier_points[c] );
	      availcarrierpts[c] = false;
	      ++used;
	    }
	}
    }

  //from graph, construct object
  std::vector<glm::vec3> ret;
  for(size_t x=0; x<mygraph.pair1.size(); ++x)
    {
      ret.push_back( mygraph.nodelocs[mygraph.pair1[x]] );
      ret.push_back( mygraph.nodelocs[mygraph.pair2[x]] );
      
    }

  return ret;
}




//Should probability fall off with distance?
//Note, in some cases, a "random walk" might be best to generate realistic shape, because you start with two and it makes cones that way...
//REV: generate points in canonically oriented cone, and then scale/rotate/transform them all.
//No scaling, just rotate and translate ;)
std::vector<glm::vec3> generate_carrier_points_from_cone_uniform( const size_t& npoints, const double& radius, const glm::vec3& basecenter,
								  const glm::vec3& tipcenter )
{
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()

  std::uniform_real_distribution<> dis( 0.0, 1.0 );
  double height = euclid( basecenter, tipcenter );

  std::vector<glm::vec3> points( npoints );

  for(size_t n=0; n<npoints; ++n)
    {
      double h = height * pow( dis(gen), (1.0/3.0) );
      double r = (radius / height) * h * sqrt( dis(gen) );
      double t = 2.0 * M_PI * dis(gen);

      double x = r * cos(t);
      double y = h;
      double z = r * sin(t);

      //goes from 0,0 I assume)
      glm::mat4 trans = glm::translate( glm::mat4(), tipcenter );
      //glm::mat4 rot = mat4_cast( RotationBetweenVectors2(glm::vec3(0.0, 1.0, 0.0), (tipcenter-basecenter) ) );
      glm::mat4 rot = mat4_cast( RotationBetweenVectors2(glm::vec3(0.0, 1.0, 0.0), (basecenter-tipcenter) ) );
      glm::vec3 oldpos( x, y, z);
      glm::mat4 total = trans * rot;
      glm::vec3 newpos = glm::vec3( total * glm::vec4( oldpos, 1.0 ) ); //chops out w.
      points[n] = newpos;
      
    }
  
  return points;
}


std::vector<glm::vec3> generate_carrier_points_from_cylinder_uniform( const size_t& npoints, const double& radius, const glm::vec3& basecenter,
								  const glm::vec3& tipcenter )
{
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()

  std::uniform_real_distribution<> dis( 0.0, 1.0 );
  double height = euclid( basecenter, tipcenter );

  std::vector<glm::vec3> points( npoints );

  for(size_t n=0; n<npoints; ++n)
    {
      double h = height * dis(gen);
      double t = 2.0 * M_PI * dis(gen);
      double s = dis(gen);

      double r = sqrt(s) * radius; //to equalize densities due to polar coordinates
      
      double x = r * cos(t);
      double y = h;
      double z = r * sin(t);
      
      //goes from 0,0 I assume)
      glm::mat4 trans = translate( glm::mat4(), tipcenter );
      //glm::mat4 rot = mat4_cast( RotationBetweenVectors2(glm::vec3(0.0, 1.0, 0.0), (tipcenter-basecenter) ) );
      glm::mat4 rot = mat4_cast( RotationBetweenVectors2(glm::vec3(0.0, 1.0, 0.0), (basecenter-tipcenter) ) );
      glm::vec3 oldpos( x, y, z);
      glm::mat4 total = trans * rot;
      glm::vec3 newpos = glm::vec3( total * glm::vec4( oldpos, 1.0 ) ); //chops out w.
      points[n] = newpos;
      
    }
  
  return points;
}



//REV: todo:
// Make selection thing (clicking on a thing selects it)
// Selecting a thing indexes into *all* the parts of that thing. E.g. soma/dend/axon.
// Change color of all those indicies (OK).
// Modulate color of things based on Vm (?). Max is "spike" or something, meh. What is a "spike" event. In axon of course. For soma/dend it's VM. For
//   axon, it's spike.

// To make "highlight" selection, put another model scaled up a bit, and make it white-ish and have alpha=0.2 or something.

//REV: could use multinomial, or just uniform draw from within sphere.
//Ghetto in sphere, by drawing in cube and discarding if outside radius.
std::vector<glm::vec3> generate_carrier_points_from_sphere_uniform( const size_t& npoints, const double& radius, const glm::vec3& center=glm::vec3(0,0,0) )
{
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen( rd() ); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<> dis(-radius, radius );
  
  std::vector<glm::vec3> points( npoints );
  size_t selected=0;
  while( selected < npoints )
    {
      double x=dis(gen) + center.x;
      double y=dis(gen) + center.y;
      double z=dis(gen) + center.z;
      glm::vec3 v(x,y,z);
      if( euclid( center, v ) <= radius )
	{
	  points[selected] = v;
	  ++selected;
	}
    }
  return points;
}
