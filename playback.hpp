#pragma once

#include <renderer.hpp>
#include <event.hpp>
#include <GLFW/glfw3.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>

#include <opencv2/opencv.hpp>

//REV: does the advancement, update of V/P matrices, and processes events.
//Of a given "scene" thing (and a given "renderer" target thing)?


//advances, writes to appropriate buffers, etc.
//Events don't have a name. Should they? E.g. "spike"
//Specify a renderbuffer too?

//First, I specify an event *set* which is just the set of definitions for how certain events should be handled.
//This, plus a list of event (timings) with the correct names.
//event_trajectory is the event definition (what to do when there is an event of that "type" );
//So, now I provide "types"

//For every group and type, I make an ET, and set_from_setting
//It may be on a per-thing basis?

struct event
{
  //std::string item;//REV: this should be a node pointer...
  //std::string type;
  node* item=NULL;
  double time;
  
  double payload;

  //This keeps track of which prop was added for "me" so I can delete it when I "end". There will always only be one property in any given time step for me.
  property* myproperty=NULL;

  event( node* i, const double& t, const double& pl=0 )
    : item(i), time(t)
  {
    
  }

  
  property* make_property()
  {
    myproperty = new property;
    myproperty->ispermanent = false;
    return myproperty;
  }

  ~event()
  {
  }
};


//An event_traj bound to a certain group, and a list of event timings for all members of that group.
//Keep around "event type" (e.g. spike, vm, etc.) just for shits.

//REV: need a way to cleanup in cases where a "rewind" etc. happened.
struct bound_event
{
  //How do I know if they're uh, "still active"?
  //All events will have same ongoing trajectory.
  std::vector< event > myevents;
  event_trajectory et;
  node* group=NULL;
  std::string eventtype;
  bool init=false;
  

  bound_event(){}
  
  bound_event(  const std::string& setting, node_forest& nf  )
  {
    init_event_traj(setting, nf );
  }

  void init_event_traj(  const std::string& setting, node_forest& nf )
  {
    std::vector<std::string> tokenized = tokenize_string( setting, " " );
    std::string groupname = tokenized[0];
    eventtype = tokenized[1];
    group = nf.find_unique( groupname );
    if( !group )
      {
	fprintf(stderr, "Couldn't find grp [%s]\n", groupname.c_str() );
	exit(1);
      }
    
    if( tokenized.size() < 2 )
      {
	fprintf( stdout, "ERROR, unrecognized setting line [%s]\n", setting.c_str() );
	exit(1);
      }
    
    std::vector<std::string> eventsetting = std::vector<std::string>( std::begin(tokenized)+2, std::end(tokenized) );
    fprintf(stdout, "SETTING ET FROM SETTINGS\n");

    et.set_from_settings( eventsetting );
  }
  

  void fill_events( const std::vector<std::string>& eventlist, node_forest& nf )
  {
    for( size_t x=0; x<eventlist.size(); ++x ) 
      {
	fill_event( eventlist[x], nf );
      }
    
    //sort the events (by time)
    //REV: [] is a simple lambda funct
    //std::sort( std::begin( myevents ), std::end( myevents ), [](const event& a, const event& b) { return ( a.time < b.time ); } );
  }

  void fill_event( const std::string& tevent, node_forest& nf )
  {
    init=false;
    std::vector<std::string> tes = tokenize_string( tevent, " " );
    if( tes.size() < 3 )
      {
	fprintf(stderr, "Error event size too small [%s]\n", tevent.c_str() );
	exit(1);
      }
	
    std::string time = tes[0];
    double timef = std::stod( time );
    std::string item = tes[1]; //this is the item. I must determine appropriateness of item by finding if it is a descendant of gropuname.
    node* itemnode = nf.find_unique( item );
    //fprintf(stdout, "(Attempting) Adding event [%s]\n", tevent.c_str() );
    if( NULL == itemnode )
      {
	//fprintf(stdout, "Failed to find item [%s]\n", item.c_str() );
	return;
	//fprintf(stderr, "Wtf GONNNEEEE\n");
	//exit(1);
      }
    if( NULL == group )
      {
	fprintf(stderr, "Huh, grp wasn't found\n");
	exit(1);
      }
    std::string type = tes[2];
    
    //this event matches. REV: FUCK FUCK FUCK, how do I include the VM value in the argument? It needs to be reproduced at every timepoint, e.g. needs to be a "new" event_trajectory. not gonna work out.
    //Just try events now, whatever.
    bool isdec = itemnode->is_descendant_of( group );
    //fprintf(stdout, "Is descendant of [%s]?\n", group->identifier.c_str() );
    //if( isdec ) { fprintf(stdout, "YES\n"); }
    bool istype = (type.compare( eventtype ) == 0);
    //fprintf(stdout, "Is [%s] equal to [%s]?\n", eventtype.c_str(), type.c_str() );
    //if( istype ) { fprintf(stdout, "YES\n"); }
    
    if( isdec && istype )
      {
	//fprintf(stdout, "Adding event [%s]\n", tevent.c_str() );
	double pl=0;
	if( tes.size() > 3 )
	  {
	    double pl = std::stod(tes[3]);
	  }
	event ev( itemnode, timef, pl);
	    
	    
	myevents.push_back( ev );
      }
    //REV: what about "optional" things? E.g. Vm? Process based on type...? Shit, how do I include that info in the "event"???? I need a field for it (e.g. value)
    
    //sort the events (by time)
    //REV: [] is a simple lambda funct
    //std::sort( std::begin( myevents ), std::end( myevents ), [](const event& a, const event& b) { return ( a.time < b.time ); } );
  }

  
  //Takes the setting for an event trajectory, and also? Note, default should be *ADD* for some guys.
  //Assume the eventlist is already filtered for "group" and "type"
  //No, setting includes all that info. Eventlist contains *all* events
  void events_from_setting( const std::string& setting, const std::vector<std::string>& eventlist, node_forest& nf )
  {
    init_event_traj( setting , nf );
    
    //literal "TIME EVENTTYPE ITEMNAME (optionals)". Shit, for example, I need to find all sub-elements of GRP? Shit. Just attach it go "grp" and any descendant
    //of grp will have it applied if they are the one who the event applies to. (any at all? Shit? What if e.g. only axon should go? How do I specify "only axon of grp"?) Worry about that later.
    //Nah, just have all event types at same time...?
    //REV: really messy to add/remove repeatedly...make a nice way to keep them around (set to zero or something) ;) If it will be used again.
    //Just make eventtype the value (e.g. v1).
    //Separate into "event" types first (?). Assume they will all come in different files? Yea...I guess. Let it separate them anyway.
    //Need a way to separate them automatically from unique types I guess?

    fill_events( eventlist , nf );
    
  }

  
  
  //Process event for "this" time step? Pre-add all properties, or wait until their first time? I guess pre-add is the "best" way, but then they will all be updated every fucking time step?
  //Wait, what, time is added now, fine. And they're sorted. But some may have stuff "before" time?

  //To set the value of a property, I do event_traj.set_property( p, timediff ), where timediff is simtime - eventtime
  //I need to determine when to delete each property based on strength of function at a given time? Iterate through all events, check whether function value is not zero, and add property now.
  //How do I make events be "active" then inactive? E.g. spikes disappear... Let user set max time?

  //cleanup to simt, with dt
  void cleanup( const double& simt, const double& dt )
  {
    init=true;
    std::sort( std::begin( myevents ), std::end( myevents ), [](const event& a, const event& b) { return ( a.time < b.time ); } );
    
    //double del_items_start_t = simt - dt - et.deletetime; //this may be in the future?
    double del_items_end_t = simt - et.deleteafter;
    
    double add_items_start_t = simt - dt;
    double add_items_end_t = simt;

    double upd_items_start_t = del_items_end_t;
    double upd_items_end_t = add_items_start_t;
    

    for(size_t t=0; t<myevents.size(); ++t)
      {
	double myt = myevents[t].time;
	if( /* myt > del_items_start_t && */ myt <= del_items_end_t )
	  {
	    //delete it...
	    //myevents[t].item->properties.remove( myevents[t].myproperty ); //remove all that are equal to this pointer.
	    myevents[t].item->remove_property( myevents[t].myproperty );
	  }
	else if( myt > upd_items_start_t && myt <= add_items_end_t )
	  {
	    if( myevents[t].myproperty )
	      {
		myevents[t].item->remove_property( myevents[t].myproperty );
		//myevents[t].item->properties.remove( myevents[t].myproperty );
	      }
	    //else
	    //{
	    //	myevents[t].myproperty = new property(); //do I need parens?
	    //}

	    //property p;
	    //property* p = new property();
	    property* p = myevents[t].make_property();
	    myevents[t].item->add_property( p );
	    et.setproperty( myevents[t].myproperty, simt - myevents[t].time ); //meh copy from above, whatever.
	  }
	else
	  {
	    myevents[t].item->remove_property( myevents[t].myproperty );
	    //myevents[t].item->properties.remove( myevents[t].myproperty ); //remove all that are equal to this pointer.
	  }
      }
  }

  void process( const double& simt, const double& dt )
  {
    if( !init )
      {
	fprintf(stdout, "WARNING: eventtraj not init, will init now using current values\n");
	cleanup( simt, dt );
      }
    //find the first event /e/ s.t. [old guys] ( e.time < simt-dt ) && ( e.time + eventtraj.deletetime >= simt ), go until [new guys] (e.time >= simt-dt) && (e.time < simt)

    //double del_items_start_t = simt - dt - et.deletetime; //this may be in the future?
    double del_items_end_t = simt - et.deleteafter;
    
    double add_items_start_t = simt - dt;
    double add_items_end_t = simt;

    double upd_items_start_t = del_items_end_t;
    double upd_items_end_t = add_items_start_t;
    

    size_t t=0;
    //fprintf(stdout, "My events size: %ld\n", myevents.size() );
    while( t < myevents.size() && myevents[t].time <= add_items_end_t )
      {
	double myt = myevents[t].time;
	if( /*myt > del_items_start_t &&*/ myt <= del_items_end_t )
	  {
	    //delete it
	    myevents[t].item->remove_property( myevents[t].myproperty );
	    //myevents[t].item->properties.remove( myevents[t].myproperty ); //remove all that are equal to this pointer.
	    //delete myevents[t].myproperty; //why not.
	  }
	else if( myt > upd_items_start_t && myt <= upd_items_end_t )
	  {
	    et.setproperty( myevents[t].myproperty, simt - myevents[t].time );
	    //update it
	  }
	else if( myt > add_items_start_t && myt <= add_items_end_t )
	  {
	    
	    //myevents[t].myproperty = new property(); //do I need parens?
	    //fprintf(stdout, "ADDING A PROPERTY!\n");
	    property* p = myevents[t].make_property();
	    myevents[t].item->add_property( p );
	    et.setproperty( myevents[t].myproperty, simt - myevents[t].time ); //meh copy from above, whatever.
	    //add it
	  }
	
	++t;
      }
    
  } //all properties should be properly updated now. For this bound event.
  
  
};

struct playback
{
  //Takes a renderer and an event queue (?)
  renderer myrenderer;
  node_forest nf;
  std::vector<bound_event> events;

  std::list<node*> active_render_grps;
  double playback_rate=5.0; //25.0; //5 ms takes one second.
  double simt=0;
  size_t framerate_s=25;

  double lasttime=0;

  glm::mat4 ViewMat;
  glm::mat4 ProjMat;

  camcontrol mycamcontrol;
  float maxviewdist=5000;
  float windowratio=1.33;
  size_t windowwidth=2000;
  size_t windowheight=1500;

  playback()
  {
  }

  void nf_from_file( const std::string& fname )
  {
    std::ifstream in(fname);
    if(!in)
      {
	fprintf(stderr, "Can't fine file [%s]\n", fname.c_str() );
	exit(1);
      }
    
    std::vector<std::string> vec;
    for( std::string line; std::getline( in, line ); )
      {
	//fprintf(stdout, "Pushing back [%s]\n", line.c_str());
	vec.push_back(line);
      }
    
    node_forest_from_config( vec );
    //fprintf(stdout, "Done\n");
  }

  void event_trajs_from_file( const std::string& fname )
  {
    std::ifstream in(fname);
    if(!in)
      {
	fprintf(stderr, "Can't fine file [%s]\n", fname.c_str() );
	exit(1);
      }
    for( std::string line; std::getline( in, line ); )
      {
	fprintf(stdout, "Creating bound event from [%s]!\n", line.c_str() );
	bound_event be( line, nf );
	events.push_back(be);
      }
  }

  void fill_events_from_file( const std::string& fname )
  {
    std::ifstream in(fname);
    if(!in)
      {
	fprintf(stderr, "Can't fine file [%s]\n", fname.c_str() );
	exit(1);
      }
    for( std::string line; std::getline( in, line ); )
      {
	for( bound_event& b : events )
	  {
	    b.fill_event(line, nf);
	  }
      }
  }
  
  //void init_camera( const float& _maxviewdist, const float& _windowratio, const glm::vec3& camstartpos, const float& camhorizrot, const float& camvertrot )
  void init_camera( const float& _maxviewdist, const size_t& winwid, const size_t& winhei, const glm::vec3& camstartpos, const glm::vec3& camcenter, const float& camhorizrot, const float& camvertrot )
  {
    windowwidth=winwid;
    windowheight=winhei;
    windowratio = (double)winwid/(double)winhei;
    maxviewdist=_maxviewdist;
    mycamcontrol = init_proj_view_mats( ProjMat, ViewMat, maxviewdist, windowratio, camstartpos, camcenter, camhorizrot, camvertrot );
  }
  
  void node_forest_from_config( const std::vector<std::string>& nodeconfig )
  {
    nf.tree_from_config( nodeconfig );
    nf.reset_render_context();
  }

  void update_my_controls( GLFWwindow* window )
  {
    double currentTime = glfwGetTime();
    double delta = currentTime - lasttime;
    
    update_controls( mycamcontrol, ProjMat, ViewMat, window, maxviewdist, windowratio, delta );
  }
    
  void full_render( )
  {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    std::vector<rendernode> rnodes = advance();
    myrenderer.render( rnodes, ViewMat, ProjMat );
  }
  
  //adds a bound event, 
  void add_events( const std::string& setting, const std::vector<std::string>& eventlist )
  {
    bound_event be;
    be.events_from_setting( setting, eventlist, nf );
    events.push_back( be );
  }
  
  void add_render_grp( const std::string& grpname )
  {
    node* n = nf.find_unique( grpname );
    active_render_grps.remove( n );
    active_render_grps.push_back( n );
  }
  
  std::vector<rendernode> advance( const bool& realtime=true )
  {
    //handle events, this will add "event" properties (changes in position, color etc) to the existing group/item properties (position, color etc).

    for(size_t e=0; e<events.size(); ++e)
      {
	events[e].process( simt, playback_rate );
      }
    
    fprintf(stdout, "SIMTIME: [%5.3f]\n", simt);

    //re-rendering is insane (obviously)
    std::vector<rendernode> rnodes;
    for( node* g : active_render_grps )
      {
	nf.generate_render_targets( g, rnodes );
      }

    simt += playback_rate; //assume this is dt/second. And we do 30 frames per second.


    
    double currentTime = glfwGetTime();
    double delta = currentTime - lasttime;
    lasttime = currentTime;
    //what units are time in? Seconds.
    double targtime = 1.0 / (double)(framerate_s);
    double sleept_s = targtime - delta;
    
    if( realtime && sleept_s > 0 )
      {
	usleep( sleept_s * 1e6 ); //usleep is specified in microseconds, e.g. second * 1e6
      }
    return rnodes;
  }

  //reads the back buffer.
  //I guess I could subsample in some way, but meh.
  std::vector<std::uint8_t> read_pixel_buffer(  )
  {
    std::vector<std::uint8_t> pixdata(windowwidth*windowheight*3);
    glReadBuffer(GL_BACK);
    glReadPixels( 0, 0, windowwidth, windowheight, GL_BGR, GL_UNSIGNED_BYTE, pixdata.data() );
    return pixdata;
  }

  void write_to_file( const std::string& fname )
  {
    std::vector< std::uint8_t > pix = read_pixel_buffer();
    //cv::Mat mat( windowwidth, windowheight, CV_8UC4 );
    cv::Mat mat( windowheight, windowwidth, CV_8UC3 );
    mat.data = pix.data();

    cv::Mat flipped;
    cv::flip( mat, flipped, 0 ); //bc I guess opengl reads from top-left instead of bottom-left?
    
    bool success = cv::imwrite( fname, flipped );
    if(!success)
      {
	fprintf(stderr, "REV: error in write_to_file [%s]\n", fname.c_str() );
      }
  }

  std::vector<std::uint8_t> read_offscreen_buffer( )
  {
    
  }


  void reset( const double& starttime, const double& dt, const size_t& fr )
  {
    framerate_s = fr;
    playback_rate = dt; //e.g. 1.0 means 1 ms (?) per second!
    simt = starttime;
    
    for(size_t x=0; x<events.size(); ++x)
      {
	events[x].cleanup( simt, dt );
      }
    lasttime = glfwGetTime();
  }

  
  

};
