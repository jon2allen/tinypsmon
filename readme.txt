### Tiny PS Monitor

planned - freebsd/linux support

small footprint up/down monitor 

configure via toml file

key in on process name, user_id, args

if condition ( up or down ) meet - execute script based on time-throttle 

cmdline - tinypsmon --pslist -> shows how the program sees the process list


### Config.toml
```
# This is a TOML document for tinypsmon.
# Author:  Jon Allen
#
# program section describes problem 
# one wants to montor
# all 3 must be true for it to activate
# proram, user, and one parms
# parm is a string search 
# so it will be match a word in args   

[program]

pgm= "bash"
parms = ""
user = "jon2allen"
interval_seconds = 8 
status = "down"
# status can be up - meanint it starts running
# status down mesns - it should be running and is down.

###################################
# options is a full string of parms
# to pass to script

[script]
location = "."
pgm = "alarm.sh"
options = "logger"
throttle_seconds = 60
# throttle_minutes = how many minutes before script is
# executed after condition is met.


# end of file
```

this project uses Cheetah for scheduling

https://github.com/hosseinmoein/Cheetah

for Toml reading it will use

https://github.com/ToruNiina/toml11


