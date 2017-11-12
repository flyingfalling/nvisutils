#ifndef CONTROLS_HPP
#define CONTROLS_HPP



struct camcontrol
{
  glm::mat4 ViewMatrix;
  glm::mat4 ProjectionMatrix;
    
  // Initial position : on +Z
  glm::vec3 position;// = glm::vec3( 0, 0, 3000 );

  glm::vec3 center;
  glm::vec3 up;

  double mousex, mousey;
  double theta, phi;
  double dtheta, dphi;
  
  // Initial horizontal angle : toward -Z
  float horizontalAngle;// = 3.14f;
  // Initial vertical angle : none
  float verticalAngle;// = 0.0f;
  // Initial Field of View
  float initialFoV;// = 45.0f;

  float speed;// = 100.0f; // 3 units / second
  float mouseSpeed;// = 0.01f;

  double lastTime;

  camcontrol(){}
  camcontrol( const glm::vec3& pos, const glm::vec3& centpos, const float& horizang, const float& vertang, const float& FoV, const float& movespeed, const float& turnspeed);
  void computeMatricesFromInputs(GLFWwindow* window, const float& maxdist, const float& ratio, const float& deltaTime);
  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();
  void setMatrices(const float& maxdist, const float& ratio);
};


extern double xscroll;
extern double yscroll;

 //crap do I need to bind this function or something? Ugh. I don't want to set globals -_-;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);



camcontrol init_proj_view_mats(  glm::mat4& ProjectionMatrix, glm::mat4& ViewMatrix, const float& maxdist, const float& ratio, const glm::vec3& position, const glm::vec3& centpos, const float& horizontalAngle, const float& verticalAngle );

void update_controls(camcontrol& mycamcontrol, glm::mat4& ProjectionMatrix, glm::mat4& ViewMatrix, GLFWwindow* window, const float& maxdist, const float& ratio, const float& deltaTime );

GLFWwindow* setup( const size_t& window_wid_pix, const size_t& window_hei_pix );

#endif
