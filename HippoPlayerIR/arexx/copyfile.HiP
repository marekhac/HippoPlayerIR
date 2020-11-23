/* HippoPlayer - Copy file
**
** This script parses the filename of the played module and then
** copy it to Liked: volume. 
**
** Written by Marek Hac (inspired by Marcel Doering's - copyfilereq.HIP)
*/

options results

address 'HIPPOPLAYER'

get cnam
CNAM = result

get name
NAME = result

dest='LikedMods:'

/* Liked: volume NOT present */
/* call pragma to avoid system request */

call pragma('W','n')
IF ~EXISTS(dest) then do
say ''
say 'FAILED: Volume LikedMods: not present'
call pragma('W','w')

exit
end

/* Liked: volume present */

ADDRESS COMMAND 'COPY "'cnam'" "'dest'"'

exit
