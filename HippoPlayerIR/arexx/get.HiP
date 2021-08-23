/* HippoPlayer - Get some information from HiP */

options results

address 'HIPPOPLAYER'

get play
if result == 1 then do
	say "HiP is playing"
end
else do
	say "HiP is not playing"
end

get hids
if result == 1 then do
	say "Window is hidden"
end
else do
	say "Window is not hidden"
end

get name
say "Loaded module:" result

get dura
say "Duration of loaded module:" result "seconds"

get fnam
say "Full name of loaded module:" result

get type
say "Type of loaded module:" result

get size
say "Size of loaded module:" result

get comm
say "File comment of loaded module:" result

get curr
say "Number of loaded module:" result

get csng
say "Current song:" result

get nsng
say "Number of songs:" result

get cspo
say "Current song position:" result

get mspo
say "Maximum song position:" result

get cfil
say "Chosen file number:" result

get cnam
say "Chosen file name:" result

get nfil
say "Files in list:" result

get filt
if result == 1 then do
	say "Filter is on"
end
else do
	say "Filter is off"
end

get volu
say "Volume: " result