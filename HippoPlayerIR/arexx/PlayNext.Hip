
/*
*** HippoPlayer - plays next song from list
*** Written 1997 by Nils Goers (no rights reserved)
*/

OPTIONS RESULTS

address HIPPOPLAYER


GET NFIL			/* Maximale Anzahl der Module */
maxmod = result

GET CFIL			/* Wo befindet man sich im Moment? */
current = result

CHOOSE current+1

IF current+1 > maxmod then exit

play
/*EOS*/



