#pragma once

std::vector<glm::vec3> make_sharp_normals( const std::vector<glm::vec3>& vertices )
{
  //For all triangles in vertices. Assume it is collections of ever 3 vertices.
  if( vertices.size() % 3 != 0 )
    {
      fprintf( stderr, "REV: vertices in make_sharp_normals has %ld vertices, not divisible by 3 (e.g. not triangles?)\n", vertices.size() );
      exit(1);
    }
  std::vector<glm::vec3> normals( vertices.size() );
  
  for( size_t t=0; t<vertices.size(); t+=3 )
    {
      glm::vec3 t1 = vertices[t];
      glm::vec3 t2 = vertices[t+1];
      glm::vec3 t3 = vertices[t+2];
      glm::vec3 V = t2-t1;
      glm::vec3 U = t3-t1;
      glm::vec3 normal = glm::vec3( ((U.y * V.z) - (U.z * V.y)),
				    ((U.z * V.x) - (U.x * V.z)),
				    ((U.x * V.y) - (U.y * V.x))
				    );

      //REV: do I need to normalize this?
      normal = normalize(normal);
      normals[t] = normal;
      normals[t+1] = normal;
      normals[t+2] = normal;
    }
  return normals;
}


bool is_near( const glm::vec3& v1, const glm::vec3& v2 )
{
  if( glm::length(v1 - v2) < 0.01f  )
    {
      return true;
    }
  return false;
}

std::vector<glm::vec3> make_smooth_normals( const std::vector<glm::vec3>& vertices )
{
  //For all triangles in vertices. Assume it is collections of ever 3 vertices.
  if( vertices.size() % 3 != 0 )
    {
      fprintf( stderr, "REV: vertices in make_sharp_normals has %ld vertices, not divisible by 3 (e.g. not triangles?)\n", vertices.size() );
      exit(1);
    }
  
  //make sharp normals, then take mean of all other guys. This is faster if they're indexed, but meh.
  std::vector<glm::vec3> normals1 = make_sharp_normals( vertices );
  std::vector<glm::vec3> normals2 = normals1;

  for(size_t x=0; x<vertices.size(); ++x)
    {
      for(size_t y=x+1; y<vertices.size(); ++y)
	{
	  if( true == is_near( vertices[x], vertices[y] ) )
	    {
	      normals2[ x ] += normals1[ y ];
	      normals2[ y ] += normals1[ x ];
	    }
	}
    }
  for(size_t x=0; x<vertices.size(); ++x)
    {
      normals2[x] = normalize(normals2[x]);
    }
  
  return normals2;
  
}


std::vector<glm::vec3> verts_from_indices( const std::vector<glm::vec3>& verts, const std::vector<unsigned short>& indices )
{
  std::vector<glm::vec3> verts2;

  for(size_t x=0; x<indices.size(); ++x)
    {
      size_t idx = indices[x];
      if(idx > verts.size())
	{
	  fprintf(stderr, "verts_from_indices, outside range\n");
	  exit(1);
	}
      verts2.push_back( verts[ idx ] );
    }
  
  return verts2;
}

