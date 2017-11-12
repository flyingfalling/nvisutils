//Renderer for each shape (I guess).

#pragma once



#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
//GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards

#include <common/shader.hpp>
#include <controls.hpp>
#include <common/vboindexer.hpp>

#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <random>


#include <node.hpp>
#include <controls.hpp>
#include <glutils.hpp>
//#include <visdefs.hpp>


void glUnBindVertexArray()
{
  glBindVertexArray(0);
}


glm::vec3 midpoint( const glm::vec3& v1, const glm::vec3& v2 )
{
  glm::vec3 mid( (v1.x+v2.x)/2.0, (v1.y+v2.y)/2.0, (v1.z+v2.z)/2.0 );
  return mid;
}

void refine_sphere( std::vector<glm::vec3>& verts, const size_t& n_refines )
{
  if( n_refines == 0 )
    {
      return;
    }

  size_t nverts=verts.size();
  for(size_t triangle=0; triangle<nverts; triangle+=3)
    {
      glm::vec3 t1=verts[triangle];
      glm::vec3 t2=verts[triangle+1];
      glm::vec3 t3=verts[triangle+2];

      glm::vec3 m12 = normalize( midpoint( t1, t2 ) );
      glm::vec3 m23 = normalize( midpoint( t2, t3 ) );
      glm::vec3 m31 = normalize( midpoint( t3, t1 ) );

      //add new triangles.
      verts.push_back( t1 );
      verts.push_back( m12 );
      verts.push_back( m31 );
      
      verts.push_back( m12 );
      verts.push_back( t2 );
      verts.push_back( m23 );

      verts.push_back( m23 );
      verts.push_back( t3 );
      verts.push_back( m31 );
      
      verts.push_back( m12 );
      verts.push_back( m23 );
      verts.push_back( m31 );
    }
  
  refine_sphere( verts, n_refines-1 );
}

//Collection of connected vertices that make a "shape" (object?) in opengl (a "mesh"?)
//Does mesh use specific shaders??
struct mesh
{
  std::vector<glm::vec3> indexed_vertices;
  std::vector<glm::vec3> indexed_normals;
  std::vector<unsigned short> indices;
  shape_enum myshape = shape_enum::CUBE;
  //REV: no need for UVs


  void enumerate()
  {
    fprintf(stdout, "Enumerating mesh (verts are:)\n");
    
    std::vector<glm::vec3> verts = verts_from_indices( indexed_vertices, indices );
    for(size_t x=0; x<verts.size(); ++x)
      {
	fprintf(stdout, "Vert [%ld]: %5.3lf %5.3lf %5.3lf\n", x, verts[x].x, verts[x].y, verts[x].z );
      }
  }
  
  mesh( const shape_enum& shape=shape_enum::CUBE )
  {
    myshape = shape;
    if( shape==shape_enum::SPHERE )
      {
	make_sphere();
      }
    else if( shape==shape_enum::CYLINDER )
      {
	make_cylinder();
      }
    else if( shape==shape_enum::CUBE )
      {
	make_cube();
      }
    else
      {
	fprintf(stderr, "REV: error in mesh constructor, shape is ???\n");
	exit(1);
      }
  }

  void clear()
  {
    indexed_vertices.clear();
    indices.clear();
    indexed_normals.clear();
  }
  
  void make_sphere()
  {
    clear();
    
    size_t n_faces=20;
    float radius = 1.0;
    float pi = 3.1415926f;

    float x, y, z, d; //d=diameter of circle at height y
    float theta = 0; //theta used for angle about y axis
    float phi = 1.0896091f; //phi used for angle to determine y and d.
    float thetad = 2 * pi / 10; //theta delta

    
    for(size_t i = 0; i < 5; ++i)
      {
	indexed_vertices.push_back( glm::vec3( 0.0f, 1.0f, 0.0f ) );
      }
  
    y = cos(phi);
    d = sin(phi);
    for (size_t i = 0; i < 12; i++)
      {
	x = sin(theta) * d;
	z = cos(theta) * d;
	//verts.push_back( glm::vec3( x/2, y/2, z/2 ) );
	indexed_vertices.push_back( glm::vec3( x, y, z ) );
	theta += thetad;
	y *= -1;
      }
    
    for (size_t i = 0; i < 5; i++)
      {
	indexed_vertices.push_back( glm::vec3( 0.0f, -1.0f, 0.0f ) );
      }

    indices = std::vector<unsigned short>( {
	0, 5, 7,
	  1, 7, 9,
	  2, 9, 11,
	  3, 11, 13,
	  4, 13, 15,
	  5, 6, 7,
	  6, 8, 7,
	  7, 8, 9,
	  8, 10, 9,
	  9, 10, 11,
	  10, 12, 11,
	  11, 12, 13,
	  12, 14, 13,
	  13, 14, 15,
	  14, 16, 15,
	  17, 8, 6,
	  18, 10, 8,
	  19, 12, 10,
	  20, 14, 12,
	  21, 16, 14 }
      );

    
    
    
    std::vector<glm::vec3> verts2 = verts_from_indices( indexed_vertices, indices );

    refine_sphere( verts2, 3 );
    
    //std::vector<glm::vec3> normals = make_sharp_normals( verts2 );
    std::vector<glm::vec3> normals = make_smooth_normals( verts2 );
    
    std::vector<glm::vec2> uvs( verts2.size() );
    std::vector<glm::vec2> indexed_uvs;

    
    
    //REV: this will appropriately clear them.
    indexVBO(verts2, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
  }

  void make_cube()
  {
    clear();
    
    std::vector<float> myvec( {
			       -1.0f,-1.0f,-1.0f,
				 -1.0f,-1.0f, 1.0f,
				 -1.0f, 1.0f, 1.0f,
				 1.0f, 1.0f,-1.0f,
				 -1.0f,-1.0f,-1.0f,
				 -1.0f, 1.0f,-1.0f,
				 1.0f,-1.0f, 1.0f,
				 -1.0f,-1.0f,-1.0f,
				 1.0f,-1.0f,-1.0f,
				 1.0f, 1.0f,-1.0f,
				 1.0f,-1.0f,-1.0f,
				 -1.0f,-1.0f,-1.0f,
				 -1.0f,-1.0f,-1.0f,
				 -1.0f, 1.0f, 1.0f,
				 -1.0f, 1.0f,-1.0f,
				 1.0f,-1.0f, 1.0f,
				 -1.0f,-1.0f, 1.0f,
				 -1.0f,-1.0f,-1.0f,
				 -1.0f, 1.0f, 1.0f,
				 -1.0f,-1.0f, 1.0f,
				 1.0f,-1.0f, 1.0f,
				 1.0f, 1.0f, 1.0f,
				 1.0f,-1.0f,-1.0f,
				 1.0f, 1.0f,-1.0f,
				 1.0f,-1.0f,-1.0f,
				 1.0f, 1.0f, 1.0f,
				 1.0f,-1.0f, 1.0f,
				 1.0f, 1.0f, 1.0f,
				 1.0f, 1.0f,-1.0f,
				 -1.0f, 1.0f,-1.0f,
				 1.0f, 1.0f, 1.0f,
				 -1.0f, 1.0f,-1.0f,
				 -1.0f, 1.0f, 1.0f,
				 1.0f, 1.0f, 1.0f,
				 -1.0f, 1.0f, 1.0f,
				 1.0f,-1.0f, 1.0f
				 });

    std::vector<glm::vec3> vertices;
    for(size_t x=0; x<myvec.size(); x+=3)
      {
	vertices.push_back( glm::vec3( myvec[x], myvec[x+1], myvec[x+2] ) );
      }
    
    std::vector<glm::vec3> normals = make_sharp_normals( vertices );
    
    
    std::vector<glm::vec2> uvs( vertices.size() );
    std::vector<glm::vec2> indexed_uvs;

    //REV: this will appropriately clear them.
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
  }

  void make_cylinder()
  {
    clear();
    
    std::vector<glm::vec3> verts;
    size_t n_faces = 20; //base it on radius?
    float radius = 1.0;
    
    for(size_t x=0; x<n_faces; ++x)
      {
	float theta1 = (x+0)*((2*M_PI)/n_faces);
	float theta2 = (x+1)*((2*M_PI)/n_faces);
	//REV: Order matters since I need to make it so it faces "outwards".
	//** clockwise (from outside of the object)
	glm::vec3 tl( radius*cos(theta1), 1.0, radius*sin(theta1) );
	glm::vec3 bl( radius*cos(theta1), -1.0, radius*sin(theta1) );
	glm::vec3 br( radius*cos(theta2), -1.0, radius*sin(theta2) );
	glm::vec3 tr( radius*cos(theta2), 1.0, radius*sin(theta2) );
	
	glm::vec3 tc( 0.0, 1.0, 0.0 );
	glm::vec3 bc( 0.0, -1.0, 0.0 );
	
	//side
	verts.push_back( tl );
	verts.push_back( br );
	verts.push_back( bl );
	
	verts.push_back( tl );
	verts.push_back( tr );
	verts.push_back( br );
	
      }

    std::vector<glm::vec3> normals = make_smooth_normals( verts );

    std::vector<glm::vec3> verts2;
    
    for(size_t x=0; x<n_faces; ++x)
      {
	float theta1 = (x+0)*((2*M_PI)/n_faces);
	float theta2 = (x+1)*((2*M_PI)/n_faces);
	//REV: Order matters since I need to make it so it faces "outwards".
	//** clockwise (from outside of the object)
	glm::vec3 tl( radius*cos(theta1), 1.0, radius*sin(theta1) );
	glm::vec3 bl( radius*cos(theta1), -1.0, radius*sin(theta1) );
	glm::vec3 br( radius*cos(theta2), -1.0, radius*sin(theta2) );
	glm::vec3 tr( radius*cos(theta2), 1.0, radius*sin(theta2) );
	
	glm::vec3 tc( 0.0, 1.0, 0.0 );
	glm::vec3 bc( 0.0, -1.0, 0.0 );
	
	//top
	verts2.push_back( tc );
	verts2.push_back( tr );
	verts2.push_back( tl );
	
	//bot
	verts2.push_back( bc );
	verts2.push_back( bl );
	verts2.push_back( br );
      }
    
    std::vector<glm::vec3> normals2 = make_sharp_normals( verts2 );

    verts.insert( std::begin(verts), std::begin(verts2), std::end(verts2) );
    normals.insert( std::begin(normals), std::begin(normals2), std::end(normals2) );
    
    //std::vector<glm::vec3> normals = make_sharp_normals( verts );
    std::vector<glm::vec2> uvs( verts.size() );
    std::vector<glm::vec2> indexed_uvs;
    
    indexVBO(verts, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
  }
};


//Renderes e.g. all spheres etc.
struct meshcollection
{
  mesh mymesh;
  
  GLuint ShadersProgramID;

  GLuint VAO;
  
  GLuint colorbuffer;
  GLuint pickcolorbuffer;
  GLuint vertbuffer;
  GLuint normalbuffer;
  GLuint elementbuffer;
  GLuint transformbuffer;
  
  GLuint VMatrixUniformID;
  GLuint PMatrixUniformID;
  GLuint LightID;

  size_t nitems;

  ~meshcollection()
  {
    glDeleteProgram(ShadersProgramID);
    
    glDeleteVertexArrays(1, &VAO);
    
    glDeleteBuffers(1, &colorbuffer);
    glDeleteBuffers(1, &pickcolorbuffer);
    glDeleteBuffers(1, &vertbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteBuffers(1, &transformbuffer);
  }
  
  meshcollection( const mesh& m, const std::string& vertshadername, const std::string& fragshadername )
    : mymesh( m )
  {
    
    ShadersProgramID = LoadShaders( vertshadername.c_str(), fragshadername.c_str() );
    fprintf(stdout, "Loaded shader program (%d)\n", ShadersProgramID ) ;

    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
        
    VMatrixUniformID = glGetUniformLocation(ShadersProgramID, "V");
    PMatrixUniformID = glGetUniformLocation(ShadersProgramID, "P");
    LightID = glGetUniformLocation(ShadersProgramID, "LightPosition_worldspace");
    
        
    glGenBuffers(1, &vertbuffer);
    glGenBuffers(1, &normalbuffer);
    glGenBuffers(1, &elementbuffer);
    glGenBuffers(1, &colorbuffer);
    glGenBuffers(1, &transformbuffer);


    
    
    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer);
    glBufferData(GL_ARRAY_BUFFER, m.indexed_vertices.size() * sizeof(glm::vec3), m.indexed_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, m.indexed_normals.size() * sizeof(glm::vec3), m.indexed_normals.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices.size() * sizeof(unsigned short), m.indices.data(), GL_STATIC_DRAW);
    
    glBindVertexArray(VAO);
    

    glBindBuffer(GL_ARRAY_BUFFER, vertbuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
			  0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			  3,                  // size
			  GL_FLOAT,           // type
			  GL_FALSE,           // normalized?
			  0,                  // stride
			  (void*)0            // array buffer offset
			  );
    
    glVertexAttribDivisor(0, 0);

    
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(
			  6,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			  3,                  // size
			  GL_FLOAT,           // type
			  GL_FALSE,           // normalized?
			  0,                  // stride
			  (void*)0            // array buffer offset
			  );
    
    glVertexAttribDivisor(6, 0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    
    glUnBindVertexArray();
    
  }

  
  void setup_meshes_with_data( const std::vector<rendernode>& rnodes )
  {
    std::vector<glm::mat4> transforms(rnodes.size());
    std::vector<byte4color> colors(rnodes.size());
    for( size_t x=0; x<rnodes.size(); ++x)
      {
	transforms[x] = rnodes[x].t * rnodes[x].r * rnodes[x].s ;
	colors[x] =  rnodes[x].color ;
      }
    setup_meshes_with_data( transforms, colors );
  }

  void setup_meshes_with_data( const std::vector<glm::mat4>& transforms, const std::vector<byte4color> colors )
  {
    if( transforms.size() != colors.size() )
      {
	fprintf( stdout, "Something is wrong" );
	exit(1);
      }

    nitems = transforms.size();

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(colors[0]), colors.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, transformbuffer);
    glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(transforms[0]), transforms.data(), GL_STATIC_DRAW);
    
    size_t vec4Size = sizeof( glm::vec4 );


    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (GLvoid*)0 );
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, transformbuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, (4 * vec4Size), (GLvoid*)(0 * vec4Size));

    glEnableVertexAttribArray(3); 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, (4 * vec4Size), (GLvoid*)(1 * vec4Size));

    glEnableVertexAttribArray(4); 
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, (4 * vec4Size), (GLvoid*)(2 * vec4Size));
    
    glEnableVertexAttribArray(5); 
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, (4 * vec4Size), (GLvoid*)(3 * vec4Size));
    
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    
    glUnBindVertexArray();
  }
  
  
  void render(  const glm::mat4& ViewMat, const glm::mat4& ProjMat  )
  {
    glBindVertexArray(VAO);
        
    //bind mesh shaders
    glUseProgram( ShadersProgramID );
    
    
    glUniformMatrix4fv(VMatrixUniformID, 1, GL_FALSE, &ViewMat[0][0]);
    glUniformMatrix4fv(PMatrixUniformID, 1, GL_FALSE, &ProjMat[0][0]);

    glm::vec3 lightPos = glm::vec3(1000.0, 3000.0, 1000.0);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
    
    glDrawElementsInstanced( GL_TRIANGLES, mymesh.indices.size(), GL_UNSIGNED_SHORT, 0, nitems );
    
    glUnBindVertexArray();
  }// end void render
}; //end struct meshcollection


struct renderer
{

  //REV: I had these as non-pointers, but in ctor, when I did cube=meshcollection(blah), the meshcollection's GLuint members were not being set appropriately, what the fuck?
  meshcollection* cube;
  meshcollection* sphere;
  meshcollection* cylinder;

  renderer()
  {
    cube=new meshcollection( mesh( shape_enum::CUBE ), "instanced.vertexshader", "instanced.fragmentshader" );
    sphere=new meshcollection( mesh( shape_enum::SPHERE ), "instanced.vertexshader", "instanced.fragmentshader" );
    cylinder=new meshcollection( mesh( shape_enum::CYLINDER ), "instanced.vertexshader", "instanced.fragmentshader" );
  }

  ~renderer()
  {
    delete cube;
    delete sphere;
    delete cylinder;
  }
  
  void render( std::vector<rendernode>& rnodes, const glm::mat4& ViewMat, const glm::mat4& ProjMat )
  {
    
    std::vector<rendernode> sp;
    std::vector<rendernode> cy;
    std::vector<rendernode> cu;
    
    for(size_t x=0; x<rnodes.size(); ++x)
      {
	if( rnodes[x].shape == shape_enum::SPHERE )
	  {
	    sp.push_back( rnodes[x] );
	  }
	else if( rnodes[x].shape == shape_enum::CUBE )
	  {
	    cu.push_back( rnodes[x] );
	  }
	else if( rnodes[x].shape == shape_enum::CYLINDER )
	  {
	    cy.push_back( rnodes[x] );
	  }
	else
	  {
	    fprintf(stderr, "REV: error renderer\n");
	    exit(1);
	  }
      }
    
    cube->setup_meshes_with_data( cu );
    cylinder->setup_meshes_with_data( cy );
    sphere->setup_meshes_with_data( sp );
    
    
    cube->render( ViewMat, ProjMat );
    cylinder->render( ViewMat, ProjMat );
    sphere->render( ViewMat, ProjMat );
  }
  
};
