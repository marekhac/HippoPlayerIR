# Changelog

### [1.6] - 2023-04-26
#### What's new
- Introduced step-down converter to power up Arduino using serial port (be ECO - save cables!)
- Improved app initialization. Now you don't need to switch on HippoPlayerIR before switching on Amiga (if you still using external power)
- Removed unnecessary script file executed by IconX
- Fixed wrong I/O request length for serial read operation

### [1.5] - 2022-01-10
#### What's new
- Add new actions:
	- Set/unset favorite module
	- Toggle playlist (between regular, favorite modules, file browser) 

### [1.4] - 2021-09-01
#### What's new
- Full synchronization with HippoPlayer! Now you can use both Amiga keyboard and
remote conntroller to control HippoPlayer (without losing consistency).
- Improved reaction time for remote controller commands

### [1.3] - 2020-11-22
#### Added
- Async serial port communication
- "Copy to LikedMods" command - copy selected module to a directory with favourite modules.
- Installer (can be used to update HippoPlayerIR)

### [1.2] - 2020-11-11 
#### Added
- Improve way of calling rx scripts
- PROGDIR as path for HippoPlayerIR.config

### [1.1] - 2019-09-23 
#### Added
- A proper way of serial port setup (now it doesn't relay on default settings from Preferences)
- Change log

### [1.0] - 2019-09-17
- First release of the HippoPlayerIR
