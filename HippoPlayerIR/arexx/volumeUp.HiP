/* HippoPlayer - Volume UP */

options results

address 'HIPPOPLAYER'

get volu
if result <= 64 then do
 newValue = result+4
 if newValue >= 64 then
   volume 64
 else
   volume newValue
end