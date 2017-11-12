
// Include GLEW
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
#include <glm/ext.hpp>

#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards

#include <common/shader.hpp>
//#include <common/texture.hpp>
#include <controls.hpp>
#include <common/vboindexer.hpp>

#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <random>
#include <iostream>

#include <controls.hpp>

glm::mat4 camcontrol::getViewMatrix()
{
  return ViewMatrix;
}

glm::mat4 camcontrol::getProjectionMatrix()
{
  return ProjectionMatrix;
}

#include <cstdio>
//#include <cstdlib>

camcontrol::camcontrol( const glm::vec3& pos, const glm::vec3& centpos, const float& horizang, const float& vertang, const float& FoV, const float& movespeed, const float& turnspeed)
{
  position=pos;
  center=centpos; //glm::vec3(0,0,0);
  //horizontalAngle=horizang;
  //verticalAngle = vertang;

  //compute from pos is best...
  double radius = glm::length( pos - center );

  glm::vec3 c2 = pos - center;
  //glm::vec3 c = glm::normalize(pos - center);
  glm::vec3 c = glm::normalize(pos - center);

  phi = atan2(c2.x, c2.z);
  //Phi should start at PI
  
  theta = -acos(c2.y/radius) + M_PI/2;
  
  std::cout << "My vect: " << glm::to_string(c2) << std::endl;
  
  fprintf(stdout, "Got initial theta [%lf] phi [%lf]\n", theta, phi);
  
  //phi=M_PI;
  //theta=0;

  up=glm::vec3(0,1,0); //No, I need to compute "from" somewhere, shit.

  dtheta=0;
  dphi=0;
  
  initialFoV = FoV;
  speed = movespeed;
  mouseSpeed = turnspeed;
  //lastTime = glfwGetTime();

  //setMatrices( 4000, 4.0/3.0 );
}



/*
void camcontrol::setMatrices(const float& maxdist, const float& ratio)
{
    // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
		      cos(verticalAngle) * sin(horizontalAngle), 
		      sin(verticalAngle),
		      cos(verticalAngle) * cos(horizontalAngle)
		      );
	
  // Right vector
  glm::vec3 right = glm::vec3(
			      sin(horizontalAngle - 3.14f/2.0f), 
			      0,
			      cos(horizontalAngle - 3.14f/2.0f)
			      );
	
  // Up vector
  glm::vec3 up = glm::cross( right, direction );

  float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

  //fprintf(stdout, "Ratio %f HA %f  VA %f  FOV %f init %f\n", ratio, horizontalAngle, verticalAngle, FoV, initialFoV);
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  ProjectionMatrix = glm::perspective(FoV, ratio, 0.1f, maxdist );
  // Camera matrix

  ViewMatrix       = glm::lookAt(
				 position,           // Camera is here
				 position+direction, // and looks here : at the same position, plus "direction"
				 up                  // Head is up (set to 0,-1,0 to look upside-down)
				 );
  //fprintf(stdout, "POS: %f %f %f\n", position.x, position.y, position.z );
  //std::cout<<glm::to_string(ProjectionMatrix)<<std::endl;
  //std::cout<<glm::to_string(ViewMatrix)<<std::endl;
}
*/

 //double xscroll;
double yscroll;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  //xscroll=xoffset;
  yscroll=yoffset;
}

static bool clickheld=false;
void camcontrol::computeMatricesFromInputs(GLFWwindow* window, const float& maxdist, const float& ratio, const float& deltaTime )
{
  
  // glfwGetTime is called only once, the first time this function is called
  //static double lastTime = glfwGetTime();

  // Compute time difference between current and last frame
  //double currentTime = glfwGetTime();
  //float deltaTime = float(currentTime - lastTime);
  

  //REV: main issue is gimbal lock (?). I need to rotate "everything". If I go to e.g. top of it, I'll be in some kind of lock, e.g. move to "left" will only rotate...I want smooth rotation.
  //So, yea, I guess quaternion...
  if(!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
    {
      //If it was held down (i.e. if it was released)
      if( clickheld == true )
	{
	  //If it's moving when I let go, it keeps moving...
	  //Do nothing, horizontal angle/vert angle will stay same, but need to keep a "rotation" velocity going.
	}
      clickheld = false;
      
    }
  
  if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
    {
      int width, height;
      glfwGetWindowSize(window, &width, &height);

      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
      
      if( clickheld == false )
	{
	  //erase anything from when it wasn't being held.
	  yscroll = 0;
	  mousex = xpos;
	  mousey = ypos;
	  // Reset mouse position for next frame
	  //glfwSetCursorPos(window, width/2, height/2);
	  //REV: it's a new click!
	  clickheld = true;
	}
      
      // Reset mouse position for next frame
      //glfwSetCursorPos(window, width/2, height/2);
      
      // Compute new orientation.
      // REV: these are angles around object!!
      
      dphi = mouseSpeed * (xpos - mousex);
      dtheta = mouseSpeed * (ypos - mousey);
      
      mousex = xpos;
      mousey = ypos;

      
    }

  phi   += dphi;
  theta += dtheta;

  if( theta > 2*M_PI || theta < -2*M_PI )
    {
      fprintf(stderr, "theta too small/big\n");
    }
  
  if( theta > M_PI )
    {
      theta = -M_PI + (theta-M_PI);
    }
  else if( theta < -M_PI )
    {
      theta = M_PI + (theta+M_PI); //phi > M_PI, so same as how much smaller...
    }
  
  
  // Direction : Spherical coordinates to Cartesian coordinates conversion
  
  //REV: I have a current position in space. I always am defining my thing as a ?
  //REV: or even easier, just keep it at a center, and then um, translate out, and rotate. Keep the rotation going. I.e. "towards" the target? Where target is what?
  //Uh, represent it as a look at a unit location in global space. And all I do is (counter-) rotate everything.
  double radius = glm::length( position - center );

  //if phi > pi/2, we are "upside down". "up" vector needs to rotate, shit.
  //Or, just forget that "up" shit and normalize phi/theta lol

  
  //sin(angle) = opp/hyp. sin(ang)*hyp = opp.

  fprintf(stdout, "Theta [%lf]   Phi [%lf]\n", theta, phi );


  //REV: PHI is rotation around equator
  //REV: THETA is amplitude
  
  //This works fine, I just need to make sure they are always in between 0 and pi?
  float eyeX = center.x + radius * sin(phi) * cos(theta);
  
  float eyeY = center.y + radius * sin(theta);
  
  float eyeZ = center.z + radius * cos(phi) * cos(theta); //cos(phi) will be zero (zero depth when pi/2 and -pi/2). And modulate length by theta. As we get closer to pi/2, it gets
  
  //z is max when phi is pi/2, and theta is 0. sin(0) is 1.

  //leftright is phi
  //up-down is theta

  position = glm::vec3( eyeX, eyeY, eyeZ );
  
  glm::vec3 direction = glm::normalize( center - position );
    //(
		      //cos(verticalAngle) * sin(horizontalAngle), 
		      //sin(verticalAngle),
		      //cos(verticalAngle) * cos(horizontalAngle)
		      //);
	
  // Right vector
  //Fuck, assume I'm never upside down LOL
  //If theta is between PI/2 and PI, or between -PI/2 and -PI, I am "upside down"
  //Always assume "right" is to my right, after rotating PHI then THETA, and facing the middle. "up" is the problem of course lol.
  //Horizontal angle is always just uh, the angle from 0 that I'm looking from. 0 is starting point looking to model. So, horiz angle i "opposite" of it I guess. Right will always be same for same uh
  //phi, theta won't change it. So just use phi to compute it.

  
  //right vector, should point uh,
  //Wait, I need another vector, so that when we take cross product, it gives "up". Literally, my x/y/z is one vector, so I need one that is orthogonal to "right".
  //Except for going upside-down, this should only depend on phi (rotation around equator, e.g. longitude), and not latitude.From my phi, I can compute my "normal" location, which is
  //
  
  float mult=1;
  if( theta > M_PI/2 || theta < -M_PI/2 )
    {
      //mult=-1;
    }

  glm::vec3 right =   glm::vec3(
				sin(phi + mult*M_PI/2),
				0,
				cos(phi + mult*M_PI/2)
				);
  
  // Up vector
  glm::vec3 up = glm::cross( right, direction ); //cross of right and foward will give, up?

  // Move forward

  if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
    glm::vec3 posdelta = up * deltaTime * speed;
    position += posdelta;
    center += posdelta;
  }
  // Move backward
  if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
    glm::vec3 posdelta = up * deltaTime * speed;
    position -= posdelta; //direction * deltaTime * speed;
    center -= posdelta;
  }
  // Strafe right
  if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
    glm::vec3 posdelta = right * deltaTime * speed;
    position += posdelta;
    center += posdelta;
  }
  // Strafe left
  if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
    glm::vec3 posdelta = right * deltaTime * speed;
    position -= posdelta;
    center -= posdelta;
  }


  //they're scrolling?
  if( yscroll != 0 )
    {
      //fprintf(stdout, "Scrolling [%lf]\n", yscroll );
      double mult=500.0 * yscroll * deltaTime;
      glm::vec3 posdelta = direction * mult; //times a constant?
      if( glm::length( position-center ) > mult )
	{
	  position += posdelta;
	}
      else
	{
	  //position += posdelta;
	  //Recompute direction?
	  //need to rotate myself by 180...should do it automatically?
	  
	  //fprintf(stdout, "ERROR, reached max zoom\n");
	  //REV: how to make "movements" i.e. move axis forward/backwards?
	}
    }
  
  float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  ProjectionMatrix = glm::perspective(FoV, ratio, 0.1f, maxdist );
  // Camera matrix

  ViewMatrix       = glm::lookAt(
				 position,           // Camera is here
				 position+direction, // and looks here : at the same position, plus "direction"
				 up                  // Head is up (set to 0,-1,0 to look upside-down)
				 );
	
  // For the next frame, the "last time" will be "now"
  //lastTime = currentTime;
}
  



camcontrol init_proj_view_mats(  glm::mat4& ProjectionMatrix, glm::mat4& ViewMatrix, const float& maxdist, const float& ratio, const glm::vec3& position, const glm::vec3& centpos, const float& horizontalAngle, const float& verticalAngle )
{
  //float maxpos = position;
  float movespeed=maxdist/5.0; //500.0;
  float mousespeed=0.001;
  float FoV = 45.0f;
  //glm::vec3 position = glm::vec3( 0.0, 0.0, 5.0 );
  //float horizontalAngle = M_PI; //3.14f;
  //float verticalAngle = 0.0;
  
  camcontrol mycamcontrol( position, centpos, horizontalAngle, verticalAngle, FoV, movespeed, mousespeed );
    
  // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction = glm::normalize( centpos - position );
    
  // Right vector
  //The vector that goes right (From center), if mine comes straight at me.
  glm::vec3 right = glm::vec3(
			      sin(M_PI - 3.14f/2.0f), 
			      0,
			      cos(M_PI - 3.14f/2.0f)
			      );
  
  // Up vector
  glm::vec3 up = glm::cross( right, direction );
  
  ProjectionMatrix = glm::perspective( FoV, ratio, 0.1f, maxdist );
  
  // Camera matrix
  ViewMatrix   = glm::lookAt(
			     position,           // Camera is here
			     position+direction, // and looks here : at the same position, plus "direction"
			     up                  // Head is up (set to 0,-1,0 to look upside-down)
			     );


  //std::cout << "PROJ " << glm::to_string( ProjectionMatrix ) << std::endl;
  //std::cout << "VIEW " << glm::to_string( ViewMatrix ) << std::endl;


  
  return mycamcontrol;
}

void update_controls(camcontrol& mycamcontrol, glm::mat4& ProjectionMatrix, glm::mat4& ViewMatrix, GLFWwindow* window, const float& maxdist, const float& ratio, const float& deltaTime )
{
  mycamcontrol.computeMatricesFromInputs( window, maxdist, ratio, deltaTime );
  //mycamcontrol.setMatrices(maxdist, ratio);
  ProjectionMatrix = mycamcontrol.getProjectionMatrix();
  ViewMatrix = mycamcontrol.getViewMatrix();
  glfwPollEvents();
}


GLFWwindow* setup( const size_t& window_wid_pix, const size_t& window_hei_pix )
{
  // Initialise GLFW
  if( !glfwInit() )
    {
      fprintf( stderr, "Failed to initialize GLFW\n" );
      getchar();
      return NULL; //-1;
    }
  
  glfwWindowHint(GLFW_SAMPLES, 0);
  //glfwWindowHint(GLFW_SAMPLES, 4); //REV: 4x multisampling (antialiasing?)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  //size_t window_wid_pix = 2048;
  //size_t window_hei_pix = 1536;
  
  // Open a window and create its OpenGL context
  //  GLFWwindow* window = glfwCreateWindow( 1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
  GLFWwindow* window = glfwCreateWindow( window_wid_pix, window_hei_pix, "Circuit Visualization", NULL, NULL);
  
  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    getchar();
    glfwTerminate();
    return NULL;
  }
  glfwMakeContextCurrent( window );

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
    return NULL;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  size_t pointerposx = window_wid_pix/2;
  size_t pointerposy = window_hei_pix/2;
  //fprintf(stdout, "POINTER POS %ld %ld\n", pointerposx, pointerposy);
  glfwSetCursorPos(window, pointerposx, pointerposy );
  // Set the mouse at the center of the screen
  glfwPollEvents();
  //glfwSetCursorPos(window, window_wid_pix/2, window_hei_pix/2);

  

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS); 
  
  // Cull triangles which normal is not towards the camera
  glEnable(GL_CULL_FACE);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glfwSetScrollCallback(window, scroll_callback);
  
  return window;
    
}

