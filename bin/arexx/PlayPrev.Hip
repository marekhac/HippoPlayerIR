
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

IF current-1 < 0 then exit

CHOOSE current-1



play
/*EOS*/



