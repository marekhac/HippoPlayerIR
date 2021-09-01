/* HippoPlayer - Jump 10 modules forward on playlist */

options results
address 'HIPPOPLAYER'
get cfil 

jumpForward = result+10
get nfil

if jumpForward > result then
   choose result
else  
   choose jumpForward
