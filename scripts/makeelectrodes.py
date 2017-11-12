#REV: read it, separate it, use freq to make timings ;)


#REV: convert from old 7-el to 49-el, pain in the butt.
#REV: assume I have 0, 2, 4, 6, 8, 10, 12, makes 7 electrodes, at 150 spacing ugh.
#REV: I *think* 0 is x=50, 2 is x=200, 4 is x=350, 6 is x=500
#REV: Just do single correlations, if right now electrodes go from uh, 

import sys

if( len( sys.argv ) != 3 ):
    print( "Not enough sys args" );
    exit(1);

inf = sys.argv[1];
outf = sys.argv[2];

ely_sep=150;
elx_sep=150;
elx_num=7;
ely_num=7;


el_shift=50;

fromtop=False;
def comp_elpos( _eln ):
    if( _eln < 1):
        print( "SUPER ERROR, comp_elpos: el number < 1");
        exit(1);
    eln=_eln-1;
    elx = (eln % elx_num);
    ely = (eln // ely_num);
    elxp = el_shift + elx_sep * elx;
    #print( elx, ely );
    if( elx > elx_num-1 or ely > ely_num-1 or elx < 0 or ely < 0):
        print( "Big error, elx or y is outside 0 to elx/ely_num" );
        print( elx, ely );
        exit(1);
        #REV: ely should only be 0 to 7.
    maxy = ely_sep*(ely_num-1); #REV: need to -1 so that we can get 150*8 is 1200. Positions are: (0, 150, 300, 450, 600, 750, 900, 1050)
    if fromtop:
        elyp = maxy - (ely_sep * ely); #REV: need ely max to be 7*sep, not 8*sep
    else:
        elyp = el_shift + ely_sep * ely;
    return elxp, elyp;

print( "In: ", inf, " out: ", outf );


out = open(outf, 'w');


elevents=False;
if(elevents):
    i = open( inf, 'r');
    evout = open(outf + '.events', 'w');

    time=200;
    for line in i:
        line1 = line.split('\n');
        items = line1[0].split(' ');
        el = items[0];
        hz = float(items[1]);
        if( hz > 0 ):
            t=0;
            while( t < time ):
                evout.write( str(t) + ' ' + el + ' pulse\n' );
                t += (1.0/hz)*1000.0;
    i.close();
    evout.close();


nel=elx_num*ely_num; #8*8;
elhei=10;
elwid=50;
eldep=50;


#REV: fuck I need to assume that center electrode is at 500/500 -_-;
for n in range(1, nel+1):
    elnum=n-1;
    x, z = comp_elpos(n);
    y = -300;
    grpname='EL'
    out.write( 'nodeprop ' + 'el' + str(elnum) + ' pos ' + str(x) + ' ' + str(y-elhei/2) + ' ' + str(z) + ' ' +  str(x) + ' ' + str(y+elhei/2) + ' ' + str(z) + ' ' + str(elwid) + ' ' + str(eldep) + '\n');
    out.write( 'nodememb ' + grpname + ' el' + str(elnum) + '\n' );
    


out.close();
