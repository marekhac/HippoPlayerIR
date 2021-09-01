/* HippoPlayer - Volume DOWN */

options results

address 'HIPPOPLAYER'

get volu
if result > 0 then do
  newValue = result-4
  if newValue <=0 then
    volume 0
  else
   volume newValue
end