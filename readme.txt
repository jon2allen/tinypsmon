# Tiny PS Monitor

## Planned Features
- FreeBSD/Linux support  
- Small footprint up/down monitor  
- Configure via TOML file  
- Key in on process name, `user_id`, and arguments  
- If condition (`up` or `down`) is met, execute a script based on time throttle  

### Command Line Usage
```sh
tinypsmon --pslist### Tiny PS Monitor


### Config.toml
```sj
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


## Libraries used
-this project uses Cheetah for scheduling
-https://github.com/hosseinmoein/Cheetah
-for Toml reading it will use
-https://github.com/ToruNiina/toml11


