
#pragma once

#include <glm/glm.hpp>

#include <visdefs.hpp>
#include <byte4color.hpp>
#include <property.hpp>
#include <string_tokenizer.hpp>


struct eventfunct
{
  std::string type;
  
  //virtual double exec( const double& d ) = 0;
  //well fuck, this was causing the error.
  virtual double exec( const double& d )
  {
    //fprintf(stdout, "EXECING BASE\n");
    return 0;
  }
};

double clamp01( const double& d )
{
  double d2=d;
  if( d < 0 ) { d2=0; }
  else if( d > 1 ) { d2=1; }
  return d2;
}

float clamp01( const float& d )
{
  float d2=d;
  if( d < 0 ) { d2=0; }
  else if( d > 1 ) { d2=1; }
  return d2;
}


struct exp_eventfunct : public eventfunct
{
  double tau;

  exp_eventfunct( const double& _tau )
    : tau( _tau )
    {}
  
  double exec( const double& d )
  {
    //fprintf(stdout, "Err, exec?\n");
    return exp( -d * (1/tau) );
  }
};

struct lin_eventfunct : public eventfunct
{
  double rate;

  lin_eventfunct( const double& _rate )
    : rate( _rate )
    {}
    
  double exec( const double& d )
  {
    //fprintf(stdout, "Err, exec lin?\n");
    //return clamp01(1.0 + ( rate * d ));
    return 1.0 + ( rate * d );
  }
};


struct colormap
{
  std::vector<glm::vec3> keypts;
  double kpspacing=1.0;
  
  colormap( const std::vector<glm::vec3>& kp )
    : keypts( kp )
  {
    if(kp.size() < 2)
      {
	fprintf(stderr, "Error, kp<2\n");
	exit(1);
      }
    kpspacing = 1.0 / (kp.size()-1);
  }
  
  byte4color map( const double& d )
  {
    size_t kpbin = (size_t)((d-1e-9) / kpspacing); //FLOOR. 0.2 with 5, will make 0.2 / 0.2 = 1;

    //REV: rofl need to interpolate between them...
    //d must be zero to one
    float mult = (d - (kpbin*kpspacing)) / kpspacing;
    //fprintf(stdout, "D=%lf, bin is %ld, mult is %lf (spacing %lf)\n", d, kpbin, mult, kpspacing );
    float r= clamp01( (keypts[kpbin+1].r - keypts[kpbin].r) * mult + keypts[kpbin].r );
    float g= clamp01( (keypts[kpbin+1].g - keypts[kpbin].g) * mult + keypts[kpbin].g );
    float b= clamp01( (keypts[kpbin+1].b - keypts[kpbin].b) * mult + keypts[kpbin].b ); //e.g. will go from 0 to 1. This will be um, -1. -1 + 1 = 0.
    //fprintf(stdout, "Blue between min [%f] and max [%f], got [%f] (diff was %f)\n", keypts[kpbin].b, keypts[kpbin+1].b, b, (keypts[kpbin+1].b - keypts[kpbin].b) );
    //wait, this means e.g. if I do, -1, and min is 1, at mult is 0, I get 1.0, and mult is um 1, I get -1 + 1 = 0
    return byte4color( glm::vec3(r,g,b) );
  }

  /*float r0, g0, b0, r1, g1, b1;

  colormap( const float& _r0, const float& _g0, const float& _b0,
	    const float& _r1, const float& _g1, const float& _b1 )
    : r0(_r0), g0(_g0), b0(_b0), r1(_r1), g1(_g1), b1(_b1)
  {  }

  byte4color map( const double& d )
  {
    float r = clamp01((r1-r0) * d + r0);
    float g = clamp01((g1-g0) * d + g0);
    float b = clamp01((b1-b0) * d + b0);
    
    //fprintf(stdout, "%f %f %f\n", r, g, b);
    return byte4color( glm::vec3(r, g, b) );
    }*/
  
};

struct scalemap
{
  float scale0, scale1;
  scalemap( const float& s0, const float& s1 )
    : scale0(s0), scale1( s1 )
  { }

  float map( const double& d )
  {
    return ((scale1-scale0) * d + scale0);
  }
};

struct translatemap
{
  float x0, y0, z0, x1, y1, z1;
  translatemap( const float& _x0, const float& _y0, const float& _z0,
		const float& _x1, const float& _y1, const float& _z1 )
    : x0( _x0 ), y0( _y0 ), z0( _z0 ), x1( _x1 ), y1( _y1 ), z1( _z1 )
  { }

  glm::vec3 map( const double& d )
  {
    float x = (x1-x0) * d + x0;
    float y = (y1-y0) * d + y0;
    float z = (z1-z0) * d + z0;
    return glm::vec3( x, y, z );
  }
};
  
op_enum opfromstring( const std::string& s )
{
  if( s.compare( "set" ) == 0 )
    {
      return op_enum::SET;
    }
  else if( s.compare( "add" ) == 0 )
    {
      return op_enum::ADD;
    }
  else if( s.compare( "sub" ) == 0 )
    {
      return op_enum::SUB;
    }
  else
    {
      fprintf(stderr, "Error in op enum from string, [%s] is not one of set, add, sub\n", s.c_str() );
      exit(1);
    }
}




eventfunct* ffromstring( const std::string& s, const std::vector<std::string>& parsed, size_t& pos )
{
  if( s.compare( "exp" ) == 0 )
    {
      ++pos;
      if( parsed[pos].compare( "tau" ) == 0 )
	{
	  ++pos;
	  double tau = doublefromstring( parsed[pos] );
	  eventfunct* e = new exp_eventfunct( tau );
	  return e;
	}
      else
	{
	  fprintf(stderr, "Expected tau\n"); exit(1);
	}
      
      
    }
  else if( s.compare( "lin" ) == 0 )
    {
      if( parsed[pos].compare( "rate" ) == 0 )
	{
	  ++pos;
	  double rate = doublefromstring( parsed[pos] );
	  eventfunct* e = new lin_eventfunct( rate );
	  return e;
	}
      else
	{
	  fprintf(stderr, "Expected rate\n"); exit(1);
	}
      
    }
  else
    {
      fprintf(stderr, "Expected exp or lin, got [%s]\n", s.c_str());
      exit(1);
      
    }
}


glm::vec3 vec3fromstring( const std::vector<std::string>& parsed, size_t& pos )
{
  if( pos+3 > parsed.size() )
    {
      fprintf( stdout, "Vec to short to get vec3 from\n" ); exit(1);
    }
  
  glm::vec3 result;

  for(size_t x=0; x<3; ++x)
    {
      result[x] = doublefromstring( parsed[pos] ); 
      ++pos;
    }
  
  return result;
}

scalemap* smapfromstring( const std::string& s, const std::vector<std::string>& parsed, size_t& pos )
{
  scalemap* c;
  double min, max;
  if( s.compare( "map" ) == 0 )
    {
      ++pos;
      if( parsed[pos].compare( "min" ) == 0 )
	{
	  ++pos;
	  min = doublefromstring( parsed[pos] );
	}
      else
	{
	  fprintf(stderr, "Error didnt get min\n");
	  exit(1);
	}
      ++pos;

      if( parsed[pos].compare( "max" ) == 0 )
	{
	  ++pos;
	  max = doublefromstring( parsed[ pos ] );
	}
      else
	{
	  fprintf(stderr, "Error didnt get max\n");
	  exit(1);
	}

      c = new scalemap( min, max );
      return c;
    }
  else
    {
      fprintf(stderr, "Error expected map but got [%s]\n", s.c_str() ); exit(1);
    }
}


colormap* cmapfromstring( const std::string& s, const std::vector<std::string>& parsed, size_t& pos )
{
  colormap* c;
  //glm::vec3 min, max, mid;
  std::vector<glm::vec3> keypts;
  size_t nkeypts;
  
  if( s.compare( "map" ) == 0 )
    {
      ++pos;
      if( parsed[pos].compare( "keypoints" ) == 0 )
	{
	  ++pos;
	  nkeypts = std::stoi( parsed[pos] );
	}
      else
	{
	  fprintf(stderr, "Didn't get expected 'keypoints' [%s]\n", parsed[pos].c_str() );
	  exit(1);
	}

      ++pos;
      size_t cnt=0;
      while( keypts.size() < nkeypts )
	{
	  if( parsed[pos].compare( "p" ) == 0 )
	    {
	      ++pos;
	      glm::vec3 p = vec3fromstring( parsed, pos );
	      keypts.push_back( p );
	    }
	}
      
      /*
      if( parsed[pos].compare( "min" ) == 0 )
	{
	  ++pos;
	  min = vec3fromstring( parsed, pos );
	}
      else
	{
	  fprintf(stderr, "Error didnt get min\n");
	  exit(1);
	}

            
      if( parsed[pos].compare( "max" ) == 0 )
	{
	  ++pos;
	  max = vec3fromstring( parsed, pos );
	}
      else
	{
	  fprintf(stderr, "Error didnt get max\n");
	  exit(1);
	}

	c = new colormap( min.x, min.y, min.z, max.x, max.y, max.z );
      */

      c = new colormap( keypts );
      return c;
    }
  else
    {
      fprintf(stderr, "Error expected map but got [%s]\n", s.c_str() ); exit(1);
    }
}

translatemap* tmapfromstring( const std::string& s, const std::vector<std::string>& parsed, size_t& pos )
{
  translatemap* c;
  glm::vec3 min, max;
  if( s.compare( "map" ) == 0 )
    {
      ++pos;
      if( parsed[pos].compare( "min" ) == 0 )
	{
	  ++pos;
	  min = vec3fromstring( parsed, pos );
	}
      else
	{
	  fprintf(stderr, "Error didnt get min\n");
	  exit(1);
	}
      
      if( parsed[pos].compare( "max" ) == 0 )
	{
	  ++pos;
	  max = vec3fromstring( parsed, pos );
	}
      else
	{
	  fprintf(stderr, "Error didnt get max\n");
	  exit(1);
	}
      
      c = new translatemap( min.x, min.y, min.z, max.x, max.y, max.z );
      return c;
    }
  else
    {
      fprintf(stderr, "Error expected map but got [%s]\n", s.c_str() ); exit(1);
    }
}

struct event_trajectory
{
  //Times are stored in "simulation" (model) time. Assume it is same. It must use a conversion to frame (or wall) time.
  //double eventtime;
  double deleteafter=(-1e9); //1 ms or whatever.
  
  
  op_enum colorop = op_enum::SET;
  op_enum scaleop = op_enum::SET;
  op_enum transop = op_enum::SET;
  
  eventfunct* colorfunct = NULL;
  eventfunct* scalefunct = NULL;
  eventfunct* translatefunct = NULL;
  
  colormap* colmap = NULL;
  scalemap* scamap = NULL;
  translatemap* transmap = NULL;

  

  
  //needs a function to return value for N seconds (?) past event time.
  //std::string descrip; //e.g. "add exp tau=30 delete=100 color... Ugh, need an options parser fuck. e.g heigt is linear but color exp, due to eye nonlinear. Too complex, just return a fucking raw color etc..
  //iterate through 'settings'
  void set_from_settings( const std::string& s )
  {
    std::vector<std::string> tokenized = tokenize_string( s, " " );
    set_from_settings( tokenized );
  }

  void set_from_settings( const std::vector<std::string>& parsed )
  {
    //fprintf(stdout, "Setting event from settings!\n");
    size_t pos=0;
    while( pos < parsed.size() )
      {
	if( parsed[pos].compare( "color" ) == 0)
	  {
	    ++pos;
	    colorop = opfromstring( parsed[pos] );
	    
	    ++pos;
	    colorfunct = ffromstring( parsed[pos], parsed, pos );
	    //fprintf(stdout, "--------COLORFUNCT: %p\n", colorfunct );

	    ++pos;
	    colmap = cmapfromstring( parsed[pos], parsed, pos );

	    //++pos;
	  }
	
	else if( parsed[pos].compare( "scale" ) == 0 )
	  {
	    ++pos;
	    scaleop = opfromstring( parsed[pos] );

	    ++pos;
	    scalefunct = ffromstring( parsed[pos], parsed, pos );

	    ++pos;
	    scamap = smapfromstring( parsed[pos], parsed, pos );
	    
	    ++pos;
	  }
	
	else if( parsed[pos].compare( "translate" ) == 0)
	  {
	    ++pos;
	    transop = opfromstring( parsed[pos] );

	    ++pos;
	    translatefunct = ffromstring( parsed[pos], parsed, pos );

	    ++pos;
	    transmap = tmapfromstring( parsed[pos], parsed, pos );
	    
	    //++pos;
	  }
	else if( parsed[pos].compare( "deleteafter" ) == 0 )
	  {
	    ++pos;
	    deleteafter = std::stod( parsed[pos] );
	    ++pos;
	  }
	else
	  {
	    fprintf(stderr, "Synatix error in set_from-settings event traj, unexpected [%s] at pos [%ld]\n", parsed[pos].c_str(), pos);
	    exit(1);
	  }
      }
  }
  
  // color add exp tau 30 delete 100 map min r=0.5 g=0.5 b=0.5 max r=1.0 g=0.5 b=0.5 scale exp tau 30 delete 100 map min 1.0 max 2.0 translate exp tau 30 delete 100 map min x=0 y=0 z=0 max x=0 y=100 z=0
  //std::function<double(double)> colortraj;

  ~event_trajectory()
  {
    /*if( colorfunct ) { delete colorfunct; }
    if( scalefunct ) { delete scalefunct; }
    if( translatefunct ) { delete translatefunct; }

    if( colmap ) { delete colmap; }
    if( scamap ) { delete scamap; }
    if( transmap ) { delete transmap; }*/
  }

  byte4color get_color( const double& d )
  {
    
    if( colorfunct == NULL || colmap == NULL )
      {
	fprintf(stderr, "Color doesn't exist\n");
	exit(1);
      }
    //fprintf(stdout, "Exec color Function\n");
    //fprintf( stdout, "%p\n", colorfunct);
    float functval = colorfunct->exec( d );
    //fprintf(stdout, "Exec color Function [%lf]\n", functval);
    byte4color a = colmap->map( functval );
    //fprintf(stdout, "Got col %d %d %d\n", a.r, a.g, a.b);
    return a;
  }

  float get_scale( const double& d )
  {
    if( scalefunct == NULL || scamap == NULL )
      {
	fprintf(stderr, "Scale doesn't exist\n");
	exit(1);
      }
    float functval = scalefunct->exec( d );
    return scamap->map( functval );
  }
  
  glm::vec3 get_translate( const double& d )
  {
    if( translatefunct == NULL || transmap == NULL )
      {
	fprintf(stderr, "Translate doesn't exist\n");
	exit(1);
      }
    float functval = translatefunct->exec( d );
    return transmap->map( functval );
  }
  
  
  
    
  void setproperty( property* p, const double& d )
  {
    
    if( d<0 )
      {
	fprintf(stderr, "Error in event setproperty, d<0 (must be >=0, events can have no effect before their fire time for...reasons ;D d=[%lf])\n", d);
	exit(1);
      }
    if( NULL == p )
      {
	fprintf(stderr, "Wtf, null ptr?\n");
	exit(1);
      }
    if( colorfunct && colmap )
      {
	//fprintf(stdout, "Attempting to set colorop\n");
	p->colorop =  colorop;
	//fprintf(stdout, "Attempting to set color\n");
	p->set_color( get_color( d ) );
      }
    if( scalefunct && scamap )
      {
	//fprintf(stdout, "Attempting to set scale\n");
	float myscale = get_scale(d); //scale all equal I guess?
	p->scaleop = scaleop;
	p->set_scaling( glm::vec3( myscale, myscale, myscale ) );
      }
    if( translatefunct && transmap )
      {
	//fprintf(stdout, "Attempting to set translation\n");
	p->transop = transop;
	p->set_translation( get_translate( d ) );
      }
  }

};


void test_event_trajectory()
{
  event_trajectory et;

  //REV: crap, scaling won't work if I 'add' hahahaha. Scale 2 + scale 3 = scale 5...not scale 6. Hum.
  std::string setting = "color add exp tau 30 map min 0.5 0.5 0.5 max 1.0 0.5 0.5 scale add exp tau 50 map min 1 max 2 translate add exp tau 100 map min 0 0 0 max 0 100 0";

  et.set_from_settings( setting );

  property p;

  et.setproperty( &p, 10.0 );

  fprintf(stdout, "Printing after 10 ms\ncolor should be around 200 r, 128, 128\nscale should be 1.8\ntranslate should be 0 90 0\n");
  p.enumerate();
}



struct setting
{
  std::string target; //target. E.g. "neurons", "group1"

  std::string function; // set_X, add_X, subtract_X, X={color, height, size}. These are absolute set, versus relative add, relative subtract. We can't multiply unfortunately.
  std::string convolution; // permanent (just changes once, no "off"), box_funct (changes to target, then changes back later, ignores what happens in between), linear_interp (goes from start to end with a
  // linear rate per time, then turns off), exponential (adds value, then exponentially decays with rate tau, then turns off at end time). Note "scale" is "amount" to add per guy. In raw terms, e.g.
  // rgb, or scale, or raw Y height.
  // Note we could change multiple things! Do they all change at the same rate?
  //Assume everything always interacts linearly (fuck it).
  //So, I just store a "scalar" thing to represent the event, and then that is translated to color/etc. via the "setting", colormapping, heightmapping, scalemapping, etc.
  //In other words, each event is turned into a single value, and that is turned to stuff. Which is always just add/remove etc. Only case where it matters is color, where I only start adding after R is full.
  //Returns a color. What if I want to track Vm for example? I specify some center, and etc., and it does the scaling itself.
  //Or what if I want height on logarythmic scale etc.
  
  byte4color* color; //always make all r/g/b channels more of same? E.g. to get more blue, it gets more uh, "bright" blue. I want it to go from red to yellow to white or something...so just add R/G? No,
  //problem is, it would need to get rid of the "red" before the yellow hah. Meh, need a "mapping" from a single value, to a color or blah. It's still "there", just clamped. Whatever. Down is green->blue->black?
  
  glm::vec3* scale;
  glm::vec3* translation;
  
  //setting
};





//Do scale, color, height separately always, even if they are part of same transform rofl. Nah make them part of same? But what if they're separated. Don't let them separate them. Could do multiple event.
//Things. OK. For "target". FUCK I may modify some "other" graphical item when something happens to "this" graphical item. Oh wow. E.g. pring "spiked"! when this one spikes. Fuuuuu. So, one at a time.
//"Neuron#34 spiked! in some console or something. Fuck it, for self graphics, this is this. Give type, traj, params, and conversion.





//A setting for how certain items, groups, etc. should be rendered. E.g. color, shape.
//it adds (permanent) properties to the appropriate node(s).
//this will handle dynamic too I guess...
//REV: I just realized I have an issue, I may want to "add" to color as spikes happen, e.g. some ghetto low pass filter rofl...in other words, I might MODIFY the color if it spiked? By taking current color.
//REV: all events (adding things) must be sorted before applying them for sure. Otherwise we have an issue, as we might modify color from spike, then later find it should have been different in the first place.
//REV: How about, I need to have different types of transforms. Some may be just a straight transform? But overall transform should be known, no? Unless I add them all together? E.g. my thing must know,
//when there's a spike, I add something, otherwise I decay it. Need to know "current" (previous) value, and multiply by a constant based on simulation/visualization speed. Biggest question, do I have
//access to "stored" value, or, can I access "previous" time step kind of thing? When I go processing/adding events, I assume I always add all events from start to end...no but it will do for each
//spike separately I guess. Or I could do some bullshit with um, blah. Need some kind of bullshit normalization too heh. Shit, since it happens by timestep, hum, But, I assume I pre-compute it, ugh.
//Would have to "Find" most recent event of that guy, fuck that. Is it the same freaking event? Or I could do LPF of VM? Fuck that. do LPF of spikes obviously. This is the only really hard thing, is
//a case where color change will represent something interacting over time. Adding is time, but it would need to decay linearly? I don't want individual spikes interacting (or, do I?) Each one could modify
//independently, hum. Exponent would sum linearly, which is fine. So would linear. That's fine too. Main problem is that uh, decay is always same, need to add in order. Might e.g. increase size or something
//whacky like that rofl. Uh, and decay exponentially (linearly?). Higher it is, longer it takes to disappear? Anyway, problem there is that spikes interact, that's the only issue. Normally, it starts/ends.
//So straight setting is fine.
//Just ensure everything is in order for the fucking simulator, e.g. spikes are in order. If that happens, then it works fine right?
//for each cell, I process all spikes or something. E.g. just timings. Just fucking convolve it over time, that's what I want rofl. Yep, just make it an "offset", and specify convolution. Is it "absolute"
//or not. Also, after end of convolution, it cuts off after some small value.

//give a corpus of functions to use. To modify height, color, or size, and some free vars. Either allow "permanent set" (which is just an "on" event at beginning). Problem is, it needs to um, know when to
//"overwrite" properties (in order I guess). Oh shit, what if higher level guys do something too? fuck it.

//There is an "on", and an "off" of a modification, and a way to modulate it in some way, via interpolation. Specify interpolation method, and its parameters.
//Specify iterpolation I guess? What if I want to have same cell go from red to blue? Have a "median". Grey. In the middle. That's fine. So, if RGB, we have red (255,0,0) and go to (0,0,255) as min/max
//values I specify. And it's always linear I guess, fuck. I could allow some other interpolation, but won't work if other value is not zero? Anyway, all elements interpolate linearly.
//The "event" could just be the value of the function thing at tha timepoint.
//REV: an event is a spike, etc. Or a change in value (wat?). That's fine. First, make the "shape" from the event. Then, make that convolution on the actual thing, on a per-timestep basis. I.e. keep same
//"transform" as a pointer or something, and modify it in that way. Or add new one at each timestep, meh. That's fine. Have a representation of event (time), convolve it with appropriate thing, and bam.
//Ah, for single cell, leave as a single modification I guess. 0 to 1 or something. Fine.
//So, for every event "type", for every "item" that could have it, we have our "representation", which is converted.
//Problem is if we change the speed, we need a way to interpolate better into frames. If slow framerate, i.e. fast simulation, I may change it more times per frame than I need (waste). So, I need a
//timesteps per framerate thing. Framerate_model, and framerate_wallclock. Just assume all model time is "normal" realtime, even if it wouldn't actually run in that. Then we can slow it down how we want.
//Make a warning if model time is diverging. E.g. run 30 ms -> visualize, etc. We may slow down the model to some extent so that we can visualize things.
//OR we may "save" all model output and input it to visualization later and replay it. So event convolutions are repesented in an "event struct"
