

#include <sstream>
#include <iomanip>


#include <playback.hpp>
#include <dendmaker.hpp>

//node_forest make_test_nf()
std::vector<std::string> make_test_nf_config()
{
  fprintf(stdout, "\n\nTESTING NODE FOREST\n\n");
  //node_forest nf;

  //REV: these are "permanent" settings. Sometimes I want to "add" etc. Assume these are all set.
  std::vector<std::string> config;
  config.push_back( "nodeprop 1" );
  config.push_back( "nodeprop 2" );
  config.push_back( "nodeprop 3" );
  config.push_back( "nodememb g1 1 2 3" );
  config.push_back( "nodeprop g1 color 1 0 0 shape cylinder" );
  config.push_back( "nodeprop g2 color 0 0 1" );
  config.push_back( "nodeprop g2 shape sphere" );
  config.push_back( "nodememb g2 4 5" );
  config.push_back( "nodeprop 4 pos 0 0 0 0 1 0 1 1" );
  config.push_back( "nodeprop 1 pos 1 0 0 1 1 0 1 1" );
  config.push_back( "nodeprop 2 pos 2 0 0 2 1 0 1 1" );
  config.push_back( "nodeprop 3 pos 0 0 1 0 1 1 1 1" );
  config.push_back( "nodeprop 5 pos 5 0 0 6 0 0 1 1" );
  config.push_back( "nodememb neurs 1 2 3 4 5 6" );

  //nf.tree_from_config( config );

  //nf.reset_render_context();
  
  //return nf;
  return config;
}

std::string make_test_event_config()
{
  //std::string conf = "g1 spike color add exp tau 30 map min 0.5 0.5 0.5 max 1.0 0.5 0.5 scale add exp tau 50 map min 1 max 2 translate add exp tau 100 map min 0 0 0 max 0 100 0 deleteafter 20";
  //std::string conf = "g1 spike color add exp tau 30 map min 0.5 0.5 0.5 max 1.0 0.5 0.5 scale add exp tau 50 map min 1 max 2 deleteafter 20";
  //std::string conf = "g1 spike color add exp tau 30 map min 0.5 0.5 0.5 max 1.0 1.0 1.0 scale add exp tau 50 map min 1 max 2 deleteafter 20";
  //std::string conf = "g1 spike color add exp tau 30 map keypoints 3 p 0 0 1 p 0 0 0 p 0 0 1 scale add exp tau 50 map min 1 max 2 deleteafter 20";
  //std::string conf = "g1 spike color set exp tau 30 map keypoints 3 p 0 0 1 p 0 0 0 p 1 0 0 scale add exp tau 50 map min 1 max 2 deleteafter 50";
  //std::string conf = "g1 spike color set exp tau 30 map keypoints 3 p 0.2 0.2 1 p 0.2 0.2 0.2 p 1 0.2 0.2 scale add exp tau 50 map min 1 max 2 deleteafter 50";
  //REV: FUCK, need a nicer way to figure out "set" or "add"...e.g. if it is already red, what to do to it?
  std::string conf = "g1 spike color set exp tau 30 map keypoints 3 p 0.3 0.0 0.0 p 1 0 0 p 1 1 0 scale add exp tau 50 map min 1 max 2 deleteafter 50";
  return conf;
}


std::vector<std::string> make_test_events()
{
  std::vector<std::string> conf;
  conf.push_back( "10.0 1 spike" );
  conf.push_back( "70.0 1 spike" );
  conf.push_back( "20.0 2 spike" );
  conf.push_back( "100.0 2 spike" );
  conf.push_back( "50.0 3 spike" );
  conf.push_back( "55.0 3 spike" );

  return conf;
}


int main( int argc, char** argv )
{
  bool doreprint=false;//true;//true;

  std::string fname = "../visutils/testspks/circuit0.circuit.neurons";
  std::string ofname = "../visutils/examples/morphneurons";
  if( argc > 1 )
    {
      for(int arg=1; arg<argc; ++arg)
	{
	  std::string cmd = std::string(argv[arg]);
	  if( cmd.compare("-genmorph" ) == 0 )
	    {
	      doreprint = true;
	      if( argc <= arg+2 )
		{
		  fprintf(stderr, "Not enough args for reprint\n");
		}
	      fname = argv[arg+1];
	      ofname = argv[arg+2];
	      arg+=2;
	      
	    }
	  else
	    {
	      fprintf(stderr, "Unrecognized cmd [%s]\n", cmd.c_str() );
	      exit(1);
	    }
	}
    }
  
  if( doreprint )
    {
      
      std::vector<std::string> nnames;
      std::vector<std::string> grps;
      std::vector<glm::vec3> locs;
  
      std::ifstream ifs( fname );
      for( std::string line; std::getline( ifs, line ); )
	{
	  fprintf(stdout, "LINE: [%s]\n", line.c_str() );
	  std::vector<std::string> parsed = tokenize_string( line, " " );
	  nnames.push_back( parsed[0] );
	  grps.push_back( parsed[1] );
	  locs.push_back( glm::vec3( std::stod(parsed[2]), std::stod(parsed[3]), std::stod(parsed[4])) );
	}

  
      //build the neuron, and translate it by that amount...
      //output soma...and axon as simple cylinder going down...

      //REV: FUCK this won't work because
      std::ofstream ofs( ofname );

      double bf=0.85;
      double hei=10;
      double hei2=1.5;
      size_t nfv_nneur = 500;
      size_t hor_nneur = 500;
      for(size_t x=0; x<nnames.size(); ++x)
	{
	  if( grps[x].compare( "FIBER" ) != 0 && ( nfv_nneur > 0 || hor_nneur > 0 ) )
	    {
	      
	      	  
	      //generate dend.
	      if( grps[x].compare( "HORIZ" ) == 0 && hor_nneur > 0)
		{
		  --hor_nneur;
		  size_t mynum=0;
		  std::string myname = nnames[x]+"_"+std::to_string(mynum);
		  //output soma
		  ofs << "nodeprop " << myname << " pos " << locs[x].x << " " << locs[x].y-hei/2 << " " << locs[x].z << " " << locs[x].x << " " << locs[x].y+hei/2 << " " << locs[x].z << " " << hei << " " << hei << std::endl;
		  ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		  ofs << "nodememb " << grps[x] << " " << nnames[x] << std::endl;

		  
		  std::vector<glm::vec3> right = generate_carrier_points_from_cone_uniform( 20, 50.0, glm::vec3(locs[x].x+200, locs[x].y, locs[x].z), glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> left = generate_carrier_points_from_cone_uniform( 20, 50.0, glm::vec3(locs[x].x-200, locs[x].y, locs[x].z), glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> axon = generate_carrier_points_from_cone_uniform( 10, 5, glm::vec3(locs[x].x, locs[x].y-300, locs[x].z), glm::vec3(locs[x].x, locs[x].y, locs[x].z) );

		  std::vector<glm::vec3> rightd = build_dend_tree_carrier_points( right, bf, glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> leftd = build_dend_tree_carrier_points( left, bf,  glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> axond = build_dend_tree_carrier_points( axon, bf,  glm::vec3(locs[x].x, locs[x].y, locs[x].z) );

		  for(size_t a=0; a<rightd.size(); a+=2)
		    {
		      ++mynum;
		      myname = nnames[x]+"_"+std::to_string(mynum);
		      ofs << "nodeprop " << myname << " pos " << rightd[a].x << " " << rightd[a].y << " " << rightd[a].z << " " << rightd[a+1].x << " " << rightd[a+1].y << " " << rightd[a+1].z << " " << hei2 << " " << hei2 << std::endl;
		      ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		    }
		  for(size_t a=0; a<leftd.size(); a+=2)
		    {
		      ++mynum;
		      myname = nnames[x]+"_"+std::to_string(mynum);
		      ofs << "nodeprop " << myname << " pos " << leftd[a].x << " " << leftd[a].y << " " << leftd[a].z << " " << leftd[a+1].x << " " << leftd[a+1].y << " " << leftd[a+1].z << " " << hei2 << " " << hei2 << std::endl;
		      ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		    }
		  for(size_t a=0; a<axond.size(); a+=2)
		    {
		      ++mynum;
		      myname = nnames[x]+"_"+std::to_string(mynum);
		      ofs << "nodeprop " << myname << " pos " << axond[a].x << " " << axond[a].y << " " << axond[a].z << " " << axond[a+1].x << " " << axond[a+1].y << " " << axond[a+1].z << " " << hei2 << " " << hei2 << std::endl;
		      ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		    }
		}//generate dend.

	      if( grps[x].compare( "NFV" ) == 0 && nfv_nneur > 0 )
		{
		  --nfv_nneur;

		  size_t mynum=0;
		  std::string myname = nnames[x]+"_"+std::to_string(mynum);
		  //output soma
		  ofs << "nodeprop " << myname << " pos " << locs[x].x << " " << locs[x].y-hei/2 << " " << locs[x].z << " " << locs[x].x << " " << locs[x].y+hei/2 << " " << locs[x].z << " " << hei << " " << hei << std::endl;
		  ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		  ofs << "nodememb " << grps[x] << " " << nnames[x] << std::endl;

	      
		  //std::vector<glm::vec3> right = generate_carrier_points_from_cone_uniform( 50, 50.0, glm::vec3(locs[x].x+200, locs[x].y, locs[x].z), glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> dend = generate_carrier_points_from_sphere_uniform( 20, 50, glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> axon = generate_carrier_points_from_cone_uniform( 10, 5, glm::vec3(locs[x].x, locs[x].y-300, locs[x].z), glm::vec3(locs[x].x, locs[x].y, locs[x].z) );

		  std::vector<glm::vec3> dendd = build_dend_tree_carrier_points( dend, bf, glm::vec3(locs[x].x, locs[x].y, locs[x].z) );
		  std::vector<glm::vec3> axond = build_dend_tree_carrier_points( axon, bf, glm::vec3(locs[x].x, locs[x].y, locs[x].z) );

		  for(size_t a=0; a<dendd.size(); a+=2)
		    {
		      ++mynum;
		      myname = nnames[x]+"_"+std::to_string(mynum);
		      ofs << "nodeprop " << myname << " pos " << dendd[a].x << " " << dendd[a].y << " " << dendd[a].z << " " << dendd[a+1].x << " " << dendd[a+1].y << " " << dendd[a+1].z << " " << hei2 << " " << hei2 << std::endl;
		      ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		    }
		  for(size_t a=0; a<axond.size(); a+=2)
		    {
		      ++mynum;
		      myname = nnames[x]+"_"+std::to_string(mynum);
		      ofs << "nodeprop " << myname << " pos " << axond[a].x << " " << axond[a].y << " " << axond[a].z << " " << axond[a+1].x << " " << axond[a+1].y << " " << axond[a+1].z << " " << hei2 << " " << hei2 << std::endl;
		      ofs << "nodememb " << nnames[x] << " " << myname << std::endl;
		    }
		}

	    } 
	}
      ofs.close();
  
      exit(0);
    }
  
  float maxdist = 7000.0;
  
  size_t winwid = 2000;
  size_t winhei = 1500;
  
  float windowratio = (double)winwid / (double)winhei; //4.0f/3.0f;
    
  GLFWwindow* window = setup( winwid, winhei );
  
  if( window == NULL )
    {
      fprintf(stderr, "No window, exiting\n");
      return -1;
    }
  
  //glm::mat4 ViewMat, ProjMat;
  
  //camcontrol mycamcontrol = init_proj_view_mats(ProjMat, ViewMat, maxdist, windowratio );

  //playback handles the timing, updates, etc. Cam and view of the scene should also be there...
  playback mypb;
  std::vector<std::string> nfconf = make_test_nf_config();
  std::string econf = make_test_event_config();
  std::vector<std::string> events = make_test_events();

  //mypb.node_forest_from_config( nfconf );
  //mypb.nf_from_file( "../visutils/examples/morphneurons" );
  mypb.nf_from_file( "../visutils/examples/mexhat_morph.neurons" );
  //mypb.nf_from_file( "../visutils/examples/mexhat.neurons" );
  mypb.nf_from_file( "../visutils/examples/neuronprops" );
  //mypb.nf_from_file( "../visutils/examples/neuronpropsstatic" );
  //mypb.nf_from_file( "../visutils/examples/neurons" );
  mypb.nf_from_file( "../visutils/examples/elects_shift" );
  mypb.nf_from_file( "../visutils/examples/elects2" );
  
  
  //2 and 6
  std::string eln = "6"; //"4"
  std::string cond = "single";

  bool showspks=true;
  bool devents=true;
  if(devents)
    {
      mypb.event_trajs_from_file( "../visutils/examples/eventsettings" );
      mypb.event_trajs_from_file( "../visutils/examples/electeventsettings" );
      //mypb.fill_events_from_file( "../visutils/examples/20018.spk" );
      //mypb.fill_events_from_file( "../visutils/examples/singlept_trial0_el6.spk" );
      //mypb.fill_events_from_file( "../visutils/examples/singlept_trial0_el4.spk" );
      //mypb.fill_events_from_file( "../visutils/examples/multipt_trial0_el0.spk" );
      

      if(showspks)
	{
	  std::string spkfname = "../visutils/examples/" + cond + "pt_trial0_el" + eln + ".spk";
	  mypb.fill_events_from_file( spkfname );
	}
      
      

      if( cond.compare( "single" ) ==0)
	{
	  std::string elfname = "../visutils/examples/trial0_el" + eln + ".events" ;
	  mypb.fill_events_from_file( elfname );
	}
      else if( cond.compare( "multi" ) ==0)
	{
	  std::string elfname1 = "../visutils/examples/trial0_el" + eln + ".events" ;
	  mypb.fill_events_from_file( elfname1 );

	  //6 is always other one on it?
	  std::string elfname2 = "../visutils/examples/trial0_el6.events" ;
	  mypb.fill_events_from_file( elfname2 );
	}
      else
	{
	  fprintf(stderr, "Not single or multi\n");
	  exit(1);
	}
      //mypb.fill_events_from_file( "../visutils/examples/elects.events" );
    }

  //glm::vec3 startpos( 500, 500, 0);
  //glm::vec3 center( 500, 0 , 0 );
  glm::vec3 startpos( 300, 400, -800);
  glm::vec3 center( 300, -300 , 500 );
  //mypb.init_camera( maxdist, windowratio, glm::vec3(500,500,2000), M_PI, -M_PI/12 );
  mypb.init_camera( maxdist, winwid, winhei, startpos, center, M_PI, -M_PI/12 );
  //mypb.init_camera( maxdist, winwid, winhei, glm::vec3(500,500,-1500), M_PI, -M_PI/12 );
  //mypb.init_camera( maxdist, windowratio, glm::vec3(0,3,10), M_PI, -M_PI/4 );
  
  //mypb.add_events( econf, events );

  mypb.add_render_grp( "EL" );
  mypb.add_render_grp( "NFV" );
  mypb.add_render_grp( "HORIZ" );
  //mypb.add_render_grp( "g1" );
  //mypb.add_render_grp( "g2" );

  double starttime = 0.0;
  double playratedt = 0.5; //1.0;
  size_t fr = 25;
  mypb.reset( starttime, playratedt, fr );

  size_t idx=0;
  do
    {
      mypb.update_my_controls( window ); //update controls, and poll events from GLFW


      mypb.full_render( ); //clear gl, advance one timestep, and render

      bool write_to_movie = true;
      if( write_to_movie )
	{
	  std::stringstream ss;
	  ss << std::setw(4) << std::setfill('0') << idx;
	  std::string s = ss.str();
	  mypb.write_to_file( "movie_" + s + ".png" );
	  ++idx;
	}
      glfwSwapBuffers(window); // Swap buffers
      
    
    } // Check if the ESC key was pressed or the window was closed
  while
    ( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
      glfwWindowShouldClose(window) == 0 );
  
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  
  return 0;
}




/*
int main2( void )
{
  float windowratio = 4.0f/3.0f;
  float maxdist = 4000.0;
  
  GLFWwindow* window = setup();
  
  if( window == NULL )
    {
      fprintf(stderr, "No window, exiting\n");
      return -1;
    }
  
  glm::mat4 ViewMat, ProjMat;
  
  camcontrol mycamcontrol = init_proj_view_mats(ProjMat, ViewMat, maxdist, windowratio );
    
  
  double lastTime = glfwGetTime();

  
  renderer myrenderer;

  node_forest nf = make_test_nf();
  
  std::vector<rendernode> rnodes;
  
  node* g1 = nf.find_unique( "g1" );
  node* g2 = nf.find_unique( "g2" );
  fprintf(stdout, "g1 num children [%ld]\n", g1->children.size() );
  fprintf(stdout, "g2 num children [%ld]\n", g2->children.size() );
  
  nf.generate_render_targets( g1, rnodes );
  nf.generate_render_targets( g2, rnodes );

  
  do
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      double currentTime = glfwGetTime();
      double delta = currentTime - lastTime;
      lastTime = currentTime;
      
      update_controls( mycamcontrol, ProjMat, ViewMat, window, maxdist, windowratio );
      
      //////////////////////////////////
      //REV: actually do all the drawing
      //////////////////////////////////
      myrenderer.render( rnodes, ViewMat, ProjMat );
      
      // Swap buffers
      glfwSwapBuffers(window);
      glfwPollEvents();
    
    } // Check if the ESC key was pressed or the window was closed
  while
    ( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
      glfwWindowShouldClose(window) == 0 );
  
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  
  return 0;
}



*/
