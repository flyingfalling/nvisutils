#pragma once


#include <map>
#include <list>
#include <vector>
#include <glm/ext.hpp>

#include <property.hpp>
//#include <visdefs.hpp>


//REV: note to self, re-implement with unordered_map, parents/children as iterators, check for uniqueness first, etc.

std::string shape_enum_str( const shape_enum& s )
{
  if( s == shape_enum::SPHERE )
    {
      return "SPHERE";
    }
  else if( s== shape_enum::CUBE )
    {
      return "CUBE";
    }
  else if( s == shape_enum::CYLINDER )
    {
      return "CYLINDER";
    }
  else
    {
      fprintf(stderr, "REV: error shape enum str, not known type\n");
      exit(1);
    }
  
}

template<typename T>
bool has_duplicates( const std::vector<T>& vec )
{
  fprintf(stderr, "REV: don't use has duplicates, too slow as-is\n");
  exit(1);
  for(size_t a=0; a<vec.size(); ++a )
    {
      for(size_t b=a+1; b<vec.size(); ++b )
	{
	  if( vec[a] == vec[b] )
	    {
	      fprintf(stdout, "Dupl! [%ld] and [%ld]\n", a, b);
	      return true;
	    }
	}
    }
  return false;
}

//A single node, which has properties, parents, and children, and an identifier (name)
struct node
{
  std::list<node*> parents;
  std::list<node*> children;
  //std::unordered_map<std::string, node*> parents; //do I ever find parents by name? rofl.
  //std::unordered_map<std::string, node*> children; //do I ever find parents by name? rofl.
  
  std::list<property*> properties;

  std::string identifier;

  bool marked=false;
  
  size_t is_descendant_of( const node* n )
  {
    size_t nfound=0;
    if( this == n )
      {
	++nfound;
      }
    else
      {
	for( node* p : parents )
	  {
	    nfound += p->is_descendant_of( n );
	  }
      }
    return nfound;
  }
  
  
  bool is_root()
  {
    if( parents.size() == 0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  bool is_leaf()
  {
    if( children.size() == 0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  std::vector<node*> get_descendent_leaves()
  {
    std::vector<node*> ret;
    for( node* n : children )
      {
	if( n->is_leaf() )
	  {
	    ret.push_back( n );
	  }
	else
	  {
	    std::vector<node*> desc = n->get_descendent_leaves();
	    ret.insert( std::end( ret ), std::begin(desc), std::end( desc ) );
	  }
      }
    return ret;
  }

  //FUUUUUUUUUUUCK
  //std::vector<std::vector<node*>> get_descendent_leaves_p( const std::vector<node*>& toroot )
  void get_descendent_leaves_p( const std::vector<node*>& toroot, std::vector<std::vector<node*>>& ongoing )
  {
    for( node* n : children )
      {
	std::vector<node*> t = toroot;
	t.push_back( n );
	if( n->is_leaf() )
	  {
	    ongoing.push_back( t );
	  }
	else
	  {
	    n->get_descendent_leaves_p( t, ongoing );
	  }
      }
  }
  
    
  ~node()
  {
    //std::list<node*>::iterator iter;
    //for( iter = std::begin( parents ); iter < std::end( parents ); ++iter )
    for( node* np : parents )
      {
	//delete the children pointer to "this" of all my parents pointing to "this" in their children.
	//REV: can't search by value.
	//Fuck, it has to be a map iterator, that way I can get .first and .second rofl.
	np->children.remove( this );
      }

    for( node* np : children )
      {
	//delete the parent pointer to "this" of all my children pointing to "this" in their parents.
	np->parents.remove( this );
      }

    /*for( property* p : properties )
      {
	delete p;
	}*/
  }

  node()
  {
  }
  
  node( const std::string& s )
    : identifier( s )
  {
    
  }
  
  void add_my_parent( node*& np )
  {
    parents.push_back( np );
    np->children.push_back( this );
  }

  void add_my_child( node*& np )
  {
    children.push_back( np );
    np->parents.push_back( this );
  }

  void add_property( property* p ) //properties might exist elsewhere...shit.
  {
    properties.push_back( p );
  }

  //Rofl hack since I don't have time to recode this as something better now.
  void remove_property( property* p ) 
  {
    properties.remove( p );
  }


  node* subnode_by_path( const std::string& path )
  {
    std::vector<std::string> tokenized = tokenize_string( path, "/" );

    node* result = subnode_by_path_v( tokenized );

    return result;
  }

  
  node* get_child_by_name( const std::string& name ) const
  {
    node* c = NULL;
    for( node* np : children )
      {
	if( np->identifier.compare( name ) == 0 )
	  {
	    if( c == NULL )
	      {
		c = np;
	      }
	    else
	      {
		fprintf( stderr, "REV: error in get_child_by_name, more than one child have identifier [%s]\n", name.c_str() );
		exit(1);
	      }
	  }
      }
    return c;
  }
  
  node* subnode_by_path_v( const std::vector<std::string>& pathv )
  {
    node* target = NULL;
    if( pathv.size() == 0 )
      {
	return this;
      }
    else
      {
	std::string next = pathv[0];
	node* c = get_child_by_name( next );
	if( c == NULL )
	  {
	    return c;
	  }
	else
	  {
	    std::vector<std::string> therest( std::begin(pathv)+1, std::end(pathv) );
	    c = c->subnode_by_path_v( therest );
	    return c;
	  }
      }
  }
  
}; //END struct node




template<typename T>
void pop_front(std::vector<T>& vec)
{
    assert(!vec.empty());
    vec.erase(vec.begin());
}



struct nf_render_context
{
  //std::vector<bool> marked;
  std::vector<node*> atoms;

  void clear()
  {
    for( node* n : atoms )
      {
	n->marked = false;
      }
    //marked.clear();
    atoms.clear();
  }
};

struct rendernode
{
  glm::mat4 t;
  glm::mat4 s;
  glm::mat4 r;

  byte4color color;

  shape_enum shape;

  void enumerate()
  {
    fprintf(stdout, "Enumerating rendernode:\n COLOR:\n");
    fprintf(stdout, "  %d %d %d %d\n", color.r, color.b, color.g, color.a );
    fprintf(stdout, " SHAPE: [%s]\n", shape_enum_str( shape ).c_str() );
    fprintf(stdout, " ROTATION:\n");
    std::cout << glm::to_string(r) << std::endl;

    fprintf(stdout, " TRANSLATION:\n");
    std::cout << glm::to_string(t) << std::endl;

    fprintf(stdout, " SCALING:\n");
    std::cout << glm::to_string(s) << std::endl;
    fprintf(stdout, "\n");
  }
  
  rendernode()
  {
    
  }
  
  void apply_property( property* p )
  {
    if( NULL==p )
      {
	fprintf(stderr, "Error in rendernode apply property, p is null\n");
	exit(1);
      }
    
    if( p->translation )
      {
	if( p->transop == op_enum::SET )
	  {
	    t = p->get_translation();
	  }
	else if( p->transop == op_enum::ADD )
	  {
	    t = p->get_translation() * t;
	  }
      }
    
    if( p->scaling )
      {
	if( p->scaleop == op_enum::SET )
	  {
	    s = p->get_scaling();
	  }
	else if( p->scaleop == op_enum::ADD )
	  {
	    s = p->get_scaling() * s;
	  }
      }

    //rotation is always added.
    if( p->rotation )
      {
	r = p->get_rotation() * r;
      }

    if( p->shape )
      {
	shape = *(p->shape);
      }

    if( p->color )
      {
	if( p->colorop == op_enum::SET )
	  {
	    byte4color c= *(p->color);
	    byte4color oc = color;
	    color = c; //*(p->color);
	    //fprintf(stdout, "SETTING color: %d %d %d from original %d %d %d (result %d %d %d)\n", c.r, c.g, c.b, oc.r, oc.g, oc.b, color.r, color.g, color.b );

	  }
	else if( p->colorop == op_enum::ADD )
	  {
	    byte4color c = *(p->color);
	    byte4color oc = color;
	    color += c; //*(p->color);
	    //fprintf(stdout, "ADDING color: %d %d %d to original %d %d %d (result %d %d %d)\n", c.r, c.g, c.b, oc.r, oc.g, oc.b, color.r, color.g, color.b );
	  }
      }
  }
};

//A forest of nodes, which have multiple parents/multiple children.
//Just a list of nodes with some functions for manipulating them.
struct node_forest
{
  //std::list<node*> nodes;
  std::map<std::string, node*> nodes;

  //causes segfault, i guess C++11 calls destructor through pointer rofl.
  /*~node_forest()
  {
    for( node* n : nodes )
      {
	delete n;
      }
      }*/
  
  std::vector<node*> find( const std::string& sprime )
  {
    std::vector<node*> ret;

    std::vector<std::string> tokenized = tokenize_string( sprime, "/" );
    if( tokenized.size() < 1 )
      {
	fprintf(stderr, "Error in find, input is [%s], tokenized size 0\n", sprime.c_str());
	exit(1);
      }
    std::string s = tokenized[0];
    pop_front( tokenized );
    
    auto it = nodes.find( s );
    if( it != std::end(nodes) )
      {
	node* n = it->second;
	node* res = node_by_path_from_node_v( tokenized, n );
	if( res != NULL )
	  {
	    ret.push_back( res );
	  }
      }
    /*
    for( node* n : nodes )
    {
      //fprintf(stdout, "Checking [%s]\n", n->identifier.c_str() );
      if( n->identifier.compare( s ) == 0 )
	{
	  //const node* n2 = &n;
	  node* res = node_by_path_from_node_v( tokenized, n );
	  if( res != NULL )
	    {
	      ret.push_back( res );
	    }
	}
	}*/
    
    //fprintf(stdout, "Done find check\n");
    return ret;
  }

  node* find_unique( const std::string& s )
  {
    std::vector<node*> nv = find( s );
    if( nv.size() == 1 )
      {
	//fprintf(stdout, "Found a unique one!\n");
	return nv[0];
      }
    else if( nv.size() == 0 )
      {
	//fprintf(stdout, "Didn't find, returning NULL\n");
	return NULL;
      }
    else
      {
	fprintf(stderr, "REV: error find_unique, [%s] was not unique (there were [%ld])\n", s.c_str(), nv.size() );
	exit(1);
      }
  }
  

  node* add_node( const std::string& id )
  {
    node* n = new node( id );
    //nodes.push_front( n );
    //check whether id exists.
    size_t nfound = nodes.count( id );
    if( nfound > 0 )
      {
	fprintf(stdout, "REV: error add_node in node_forest, node with id [%s] already exists\n", id.c_str() );
	exit(1);
      }
    nodes[id] = n;
    return n;
  }
  
  //Should store this full
  std::vector<node*> get_roots()
  {
    std::vector<node*> roots;
    //for( node* n : nodes )
    for (auto const& x : nodes)
      {
	if( x.second->is_root() )
	  {
	    roots.push_back( x.second );
	  }
      }
    return roots;
  }

  //Should store this full
  std::vector<node*> get_leaves()
  {
    std::vector<node*> leaves;
    //for( node* n : nodes )
    for (auto const& x : nodes)
      {
	if( x.second->is_leaf() )
	  {
	    leaves.push_back( x.second );
	  }
      }
    return leaves;
  }

  //REV: not easy to add/remove roots if its a vector. Better to store as a list if we hold it around permanently.
  node* find_type( const std::string& targtype )
  {
    node* result = NULL;
    std::vector<node*> roots = get_roots();
    for( size_t x=0; x<roots.size(); ++x )
      {
	if( targtype.compare( roots[x]->identifier ) == 0 )
	  {
	    if( result == NULL )
	      {
		result = roots[x];
	      }
	    else
	      {
		//already found one of type, 2 root nodes of same type, error
		fprintf(stderr, "REV: vis2.hpp, find_type in node_forest: found more than one root node of type [%s], erroring\n", targtype.c_str() );
		exit(1);
	      }
	  }
      }
    return result;
  }

  node* node_by_path_from_node( const std::string& path, node* start_node )
  {
    std::string tmp = path;
    std::vector<std::string> pathv = tokenize_string( tmp, "/" );

    return node_by_path_from_node_v( pathv, start_node );
  }

  node* node_by_path_from_node_v( const std::vector<std::string>& pathv, node* start_node )
  {
    node* target = start_node->subnode_by_path_v( pathv );
    return target;
  }
      
  node* node_from_path( const std::string& path )
  {
    std::string tmp = path;
    std::vector<std::string> tokenized = tokenize_string( tmp, "/" );
    if( tokenized.size() < 1 )
      {
	fprintf( stderr, "REV: attempting to find node from empty path\n");
	exit(1); //or return null.
      }
    std::string root_name = tokenized[0];
    node* start_node = find_type( root_name );
    std::vector<std::string> therest( std::begin( tokenized ) + 1, std::end( tokenized ) );
    
    return node_by_path_from_node_v( therest, start_node );
  }


  //line is a single line.
  void parse_line( const std::vector<std::string>& line )
  {
    //item, grp-memb, grp-prop. What is diff between item and group? There is none.
    //so, just memb and prop.
    if(line.size() < 1 )
      { fprintf(stderr, "Line is too short! Returning\n"); return; }
    size_t pos=0;

    if( line[pos].compare( "nodeprop" ) == 0 )
      {
	++pos;
	if( pos >= line.size() )
	  {
	    fprintf(stderr, "NODEPROP, line not long enough\n");
	    exit(1);
	  }
	std::string ident = line[pos];
	//fprintf(stdout, "Node is [%s]\n", ident.c_str());
	++pos;
	
	node* n = find_unique( ident );
	if( n == NULL )
	  {
	    //fprintf(stdout, "\t(doesn't exist, creating)\n");
	    //add node.
	    n = add_node(ident); //new node( ident );
	  }
	std::vector<std::string> props( std::begin(line) + pos, std::end( line ) );

	//fprintf(stdout, "N ident:\n");
	//fprintf(stdout, "N idnent [%s]\n", n->identifier.c_str() );
	if( n == NULL )
	  {
	    fprintf(stderr, "WTF\n");
	    exit(1);
	  }
	
	if( props.size() > 0 )
	  {
	    //property p( props );

	    property* p = new property( props ); //this is where it adds :)
	    n->add_property( p );
	  }
      }
    else if( line[0].compare( "nodememb" ) == 0 )
      {
	//fprintf(stdout, "Parsing NODE MEMBERS\n");
	++pos;
	if( pos >= line.size() )
	  {
	    fprintf(stderr, "NODEMEMB, line not long enough\n"); exit(1);
	  }
	std::string ident = line[pos];
	++pos;

		

	//fprintf(stdout, "Node is [%s]\n", ident.c_str() );
	node* n = find_unique( ident );
	if( n == NULL )
	  {
	    //fprintf(stdout, "\t(doesn't exist, creating)\n");
	    //add node
	    n = add_node(ident); //new node( ident );
	  }
	
	//set members of node (children).
	//Note: you can *add* as children nodes which do not exist yet. It will just make empty.
	while( pos < line.size() )
	  {
	    
	    ident = line[pos];
	    //	    fprintf(stdout, "\t\tAdd as member [%s]\n", ident.c_str() );
	    
	    node* n2 = find_unique( ident );
	    if( n2 == NULL )
	      {
		//fprintf(stdout, "\t\t\t(didn't exist, creating)\n");
		n2 = add_node(ident); //new node( ident );
	      }
	    
	    node* exists = n->get_child_by_name( ident );
	    if( exists == NULL )
	      {
		//fprintf(stdout, "\t\t\tWasn't a child, adding\n");
		n->add_my_child( n2 );
	      }
	    else
	      {
		//fprintf(stdout, "\t\t\tWas already a child, weird?\n");
	      }
	    ++pos;
	    //else, it already exists as a child of n, wtf.
	  }
      }
  }

  void tree_from_config( const std::vector<std::string>& lines )
  {
    for(size_t l=0; l<lines.size(); ++l)
      {
	//fprintf(stdout, "Parsing line [%s]\n", lines[l].c_str() );
	std::string line = lines[l];
	std::vector<std::string> parsed = tokenize_string( line, " " );
	parse_line( parsed );
      }
  }
  
  //Generates render targets of all leaves, going from specified root nodes. Assume I will never have
  //overlap? Need a way of "marking" or something rofl.
  //Always goes, "Bottom to top".

  nf_render_context render_context;
  
  void reset_render_context()
  {
    render_context.clear();
    render_context.atoms = get_leaves();
    //render_context.marked = std::vector<bool>( render_context.atoms.size(), false );
  }
  
  void generate_render_targets( node* root, std::vector<rendernode>& rnodes )
  {

    std::vector<node*> paths( {root} );
    std::vector<std::vector<node*>> descendent_leaves_pathways;
    //fprintf(stdout, "Finding leaves...\n");
    //fprintf(stdout, "About to get desc leaves\n");
    root->get_descendent_leaves_p( paths, descendent_leaves_pathways );
    //fprintf(stdout, "Got desc leaves\n");
    std::vector<node*> descendent_leaves( descendent_leaves_pathways.size() );

    //fprintf(stdout, "Filling desc leaves from pathways\n");
    for(size_t x=0; x<descendent_leaves_pathways.size(); ++x)
      {
	descendent_leaves[x] = descendent_leaves_pathways[x][ descendent_leaves_pathways[x].size() - 1 ];
	//fprintf(stdout, "Leaf: [%s]\n", descendent_leaves[x]->identifier.c_str() );
      }

    //just make sure it's not marked twice...

    /*fprintf(stdout, "Checking for duplicates\n");
    if( has_duplicates( descendent_leaves ) )
      {
	fprintf(stderr, "REV: error, leaves have dupls\n");
	exit(1);
      }
    */

    
    //fprintf(stdout, "Iterating to create nodespans\n");
    //REV: problem, right now I go down-to-up, so I reset to "group" color after the fact. First, I should apply all "permanent" settings, and then apply all "event" settings top-to-bottom?
    for( size_t n=0; n<descendent_leaves.size(); ++n )
      {
	if( !descendent_leaves[n]->marked )
	  {
	    //I should know the "correct" path, fuck. Remember it when I go down I guess.
	    //Anyway, do it. Follow the path and build properties to an ongoing "rendernode"
	    //Optimize later: only update them when they "change". Later.

	    //Final property to render, including model space matrix and color (marker for selected/not)
	    rendernode r;
	    std::vector<node*> nodespan = descendent_leaves_pathways[n];
	    //fprintf(stdout, "Going through descendent leaves [%ld] (depth is [%ld])!\n", n, nodespan.size() );
	    //for( size_t x=nodespan.size()-1; x>0; --x )
	    //REV: point of going bottom-to-top first is for translation, to get correct position.
	    for( size_t xp=0; xp<nodespan.size(); ++xp )
	      {
		size_t x = nodespan.size() - xp - 1;
		//fprintf( stdout, "climbing node: [%ld], ([%ld] depth of [%ld])\n", n, x, nodespan.size()-1 );
		for( property* p : nodespan[x]->properties )
		  {
		    if( p->ispermanent )
		      {
			r.apply_property( p );
		      }
		  }
	      }
	    
	    for( size_t x=0; x<nodespan.size(); ++x )
	      {
		//fprintf( stdout, "climbing node: [%ld], ([%ld] depth of [%ld])\n", n, x, nodespan.size()-1 );
		for( property* p : nodespan[x]->properties )
		  {
		    if( false == p->ispermanent )
		      {
			r.apply_property( p );
		      }
		  }
	      }

	    rnodes.push_back( r );
	  }
      }
    //fprintf(stdout, "Finished making nodespans\n");
  } //end generate render targets.
  
}; //end struct node_forest


//REV: I can technically render this rofl.
void test_node_forest()
{
  fprintf(stdout, "\n\nTESTING NODE FOREST\n\n");
  node_forest nf;

  //REV: these are "permanent" settings. Sometimes I want to "add" etc. Assume these are all set.
  std::vector<std::string> config;
  config.push_back( "nodeprop 1" );
  config.push_back( "nodeprop 2" );
  config.push_back( "nodeprop 3" );
  config.push_back( "nodememb g1 1 2 3" );
  config.push_back( "nodeprop g1 color 1 0 0 shape sphere" );
  config.push_back( "nodeprop g2 color 0 0 1" );
  config.push_back( "nodeprop g2 shape cylinder" );
  config.push_back( "nodememb g2 4 5" );
  config.push_back( "nodeprop 4 pos 1 0 0 2 0 0 1 1" );
  config.push_back( "nodeprop 1 pos 1 1 0 2 1 0 1 1" );
  config.push_back( "nodeprop 2 pos 1 2 0 2 2 0 1 1" );
  config.push_back( "nodeprop 3 pos 1 3 0 2 3 0 1 1" );
  config.push_back( "nodememb neurs 1 2 3 4 5 6" );

  nf.tree_from_config( config );

  nf.reset_render_context();
  node* g1 = nf.find_unique( "g1" );
  fprintf(stdout, "g1 num children [%ld]\n", g1->children.size() );
  std::vector<rendernode> rnodes;
  nf.generate_render_targets( g1, rnodes );
    
  fprintf(stdout, "Done\n");
}


//REV: create a node forest from a configuration file. Configuration files have 1) unique names for each "atom" (but they may be made from more complex sets of atoms, rofl, fuck).
//These can be combined into named groups by lines such as "group1" "item" "item" "item". However, these may also refer to groups, e.g. "neurons" "group1" "group2". Furthermore, in the specification,
//I can specify e.g. which groups it is a part of? I don't want to waste space. Just use text whatever.
//Uh, specify (tightly) per-line, which they are part of. E.g. group axon, etc.. In general, user won't programatically sepcify anything but it will be read from a created circuit. Fine.
//Oh shit sometimes it will be done programmatically directly from simulator heh.
//How about, all low level things must be shapes? That's fine. No need to store in memory though heh.

//Make from individual guys. Somehow, by ID. I guess it must build by order. So, I initially e.g.
//"pieces" are just pieces. Those must be turned into "neurons" or something. OK, do raw pieces initially
//1 pos x1 x2 wid hei color 1 0 0 shape sphere
//versus e.g.
//1 pos x1 x2 wid hei
//2 pos x1 x2 wid hei
//3 pos x1 x2 wid hei
//n1 1
//n2 2
//n3 3
//g1 n1
//g2 n2 n3
//neurons g1 g2 #REV: these are GROUPS wtf.
//neurons n1 n2 n3
//neurons shape sphere
//g2 color 1 0 0
//g1 color 0 0 1

//Every thing *must* have a string name, that I search for globally. Fuck, some may be named e.g. "axon". Or will it? If it has no name, it doesn't return it. If I want to return to specifi guy, it gets,
//then it finds child named X.
//OK, so use unordered map, which is great. Um, problem is uh, constant time is fine. Fuck, how do I find things? Key might change. E.g. directory, blah/blah2/blah3. Ahahaha children is an unordered map
//of fucking pointers to other nodes, wow. Brilliant. Faster, I guess. Need to find the "guy who points to me" in it. Can't search by key. Whatever. Better as fuck make sure there are no duplicates.
//It's only "axon" from within me rofl. Why use opengl when I do all the CPU shit over here, rofl. How do I ensure uniqueness? Make sure when add it's unique. That's fine. And only allow e.g. n32-axon.
//"Type" may be axon, which is fucking fine. Or e.g. grp3-axon or shit.

//e.g. g1/neurons/ etc.?
//What if I need intersections of groups to set things? E.g. only if it is an axon AND in group1. Fuuuuuck.
//I could make "commands". Like "set group" or some shit. I think I already do that. But those are um, named groups or some shit. Worry about intersections later I guess.
//At any rate, format is just that, a fucking format. Who cares. Setting a property is still just exactly those. It gets a "node" and does that.
//That's fine.
//Have a separate command that specifies I'm making/adding a group, etc. I don't know how many there are...?
//Add a neuron to a group or something how about that.
//The group must be a root group?
//Can I use hierarchy search to find it otherwise (from a root group).
//Searches always happen from a root group, or an ident? Shit, how do I find neurons initially? I need to declare it initially? Nah screw that... Just make identifiers unique or some shit.
//We might not start off with um, atoms. It may be complex, and add more children or some shit. Hm.

//REV: DECISION
//raw guys are lowest level, e.g. 11, etc.
//Specify it that way.
//E.g. item

//GRPNAME must *always* be accessible via raw path from root? No, just by groupname. Doesn't need to be a root. That's fine. What is a "type". An identifier? Whole idea was to do e.g. "axon" or something.
//I guess I could store items elsewhere (permanently/quickly?). Just give a fast way to (randomly?) access elements? A hash or something.

//item [itemnum] properties [props]
//grp [grpname] properties [props]
//grp [grpname] children [children list]

//Turn on/off rendering of certain groups. Hope they are all "contiguous" in space or something? Just make alpha 100%?

//Make a way to render.






 //REV: RESULT 20 jul 2017
  //Need a way to like, binary search, to find which "node" corresponds to an "atom". I will have vector with each VBO to atoms for fast locating of object/etc.
  //REV: there may be e.g. a sub-node that is the axons of it, but it is not a sub of the thing unless specified as a "type". So if I make it as axon/soma/dendrite, and put them together?
  //Or, just raw specify parts of it? Things like axons are "types", shared by cells. Might want to specify axon afterwards, but would I literally directly link those pieces to "axon", or would I make a
  //sub-node "axon", and link it to "axon"? I might want to make modifications...so I need to "add" a node, shit. However, it does not need to (can NOT be) a sub-node of "neuron".
  //Crap, I need a way of finding e.g. individual neurons when I want to find them? E.g. for firing purposes. So, all "named" things are named. Some guys don't have "axons" etc. We can add things.
  //How can I specify a subset of (atom) nodes etc., are part of a group or type (pass raw pointer list?). Or e.g. that neurons are part of a group after the fact? Yea, get the list I guess.
  //Situation of e.g. "electrodes". What is an electrode? Doesn't matter rofl. Situation where I add parts to make a whole, e.g. axon + dendrite + soma = neuron. In this case, there are nodes...?
  //REV: fuck, what about where I have a 'selection' group (node) going temporarily. Whatever, just add it rofl. At any rate, that's fine. I e.g. iterate through all 'axons' of it, or all 'dendrites' of it
  //or all 'super' groups it is part of? In situations where I will select e.g. intersection of target and axons. All "groups" it is part of. Is it intersection? Or all?
  //Um, fuck it. Sometimes I will have no "axon" groups, that's fine. It can either be uh, global type (root), or it can be uh, a local node, which has a name. That's fine. So, those are two types of things.
  //I can select things from hierarchy. If I do e.g. neurons/neuron34/axon, will it know to look for "global" type intersections, if it can't find a type? Or just error out? What's the point. Some sub-nodes,
  //goal is just to um, avoid storing strings in every single guy if i don't need to. And to handle situations where some guys have multiple "types". OK, types are fine. Or groups. Or e.g. uh, electrodes.
  //Like, ok, in this, all electrodes (types) are rendered as cubes. Or all guys of group X are rendered as cylinders. Or e.g. blah/axon are as cylinders, blah/dend are squares, etc.. What if there's overlap?
  //I need to specify the "size" as well of course -_-;
  //Fuck it, why did I have issues before with multiple types? Fuck the global group things, just do blah. Eventually I can have global pointers pointing to all guys of same types (to ease search). That's good.
  //E.g. neurons/blah39 lets me find it easier in a "sub-tree". However, in case of e.g. axons, the name is just "axon", so I'd have to go through to make sure it's an axon of a given neuron or something.
  //To get an axon, I could do by name, where certain children/parents have names? Just go through children to find ones of correct name. Ah. Great. However, um, neurons are under neurons. Or e.g. there
  //are groups, of cells. Which may likewise be top-level. I could select only axons in a group? Intersect of axons group and groupA group. Good.
  //If I select blah/blah, it must match exactly. I guess I could do e.g. grp1/axons, and it could either 1) skip levels and find only those of type "axon" or 2) do intersection of axon top-level
  // (found by tabulating nodes with type "axon" in the first place). Probably 2 is best (same). Other options are e.g. locating things by location (shit). E.g. what if I only want to select neurons within an
  // area, or highlight neurons within a certain quadrant of the screen, or in model space. Assuming none are transformed, I'd need to somehow either tabulate it beforehand, or make some (dense) groups,
  //e.g. not named "top-left", "bottom2x3" etc., but rather an array that is not named. But, that could be arbitrarily difficult...just make the names after all rofl.
