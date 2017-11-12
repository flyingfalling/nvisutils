import sys

if( len( sys.argv ) != 3 ):
    print( "Not enough sys args" );
    exit(1);

inf = sys.argv[1];
outf = sys.argv[2];


print( "In: ", inf, " out: ", outf );

infp = open( inf, 'r' );
outfp = open( outf, 'w' );
for line in infp:
    line1 = line.split( '\n' );
    res = line1[0].split( ' ' );
    if( len( res ) != 2 ):
        print( "Res len not 2" );
        exit(1);
    outfp.write( res[1] + " " + res[0] + " spike\n" );

infp.close()
outfp.close();

