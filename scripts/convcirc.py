import sys

if( len( sys.argv ) != 3 ):
    print( "Not enough sys args" );
    exit(1);

inf = sys.argv[1];
outf = sys.argv[2];


print( "In: ", inf, " out: ", outf );

infp = open( inf, 'r' );
outfp = open( outf, 'w' );

nheight = 10.0;
nhr = nheight/2;

for line in infp:
    line1 = line.split( '\n' );
    res = line1[0].split( ' ' );
    if( len( res ) != 5 ):
        print( "Res len not 5" );
        exit(1);
    #outfp.write( res[0] + " " + res[1] + " SPIKE\n" );
    n = res[0];
    grp = res[1];
    posx = float(res[2]);
    posy = float(res[3]);
    posz = float(res[4]);
    outfp.write( "nodeprop " + str(n) + " pos " +  str(posx) + " " + str(posy-nhr) + " " + str(posz) + " " +  str(posx) + " " + str(posy+nhr) + " " + str(posz) + " " + str(nheight) + " " + str(nheight) + '\n' );
    outfp.write( "nodememb " + grp + " " + str(n) + '\n' );

infp.close()
outfp.close();

