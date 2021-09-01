/* HippoPlayer - Jump 10 modules backward on playlist */

options results
address 'HIPPOPLAYER'
get cfil

if result > 1 then do 
 jumpBack = result-10
 if jumpBack <= 0 then
   choose 1
 else  
   choose jumpBack
end