/* HippoPlayer - Stop/continue playing */

options results

address 'HIPPOPLAYER'

get play
if result == 1 then do
   stop
end
else do
   cont
end