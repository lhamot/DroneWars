# Neutrynos - DroneWars
**Neutrynos - DroneWars** is a browser game, online and multiplayer, where you have to conquer the galaxy by coding.

## License

Copyright Loïc HAMOT, 2018

Distributed under the Boost Software License, Version 1.0.

See ./LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt

## Requirements

### For the C++ backend

* C++ >= c++11 (tested with Visual Studio and gcc)
* boost >= 1.53
* poco.data >= 1.6.1
* lua 5.2
* thrift (for C++ and python) >= 0.9.1

### For the python (web) frontend

* Python >= 2.7
* django
* google.blockly
* bootstrap

### Other

* A MySQL database

## How to install it?

### For the backend

* Install and compile all dependencies
* Use **cmake** to compile DroneWars
* Create a user and a database for it in the MySQL database
* Add a **dronewars.ini** file of this form:
```C
host = localhost
port = 3306
database = dronewars
user = dronewars
password = ???????????
min_round = 10
```

### For the frontend

* Add the **"DRONEWARSPATH/src"** path in the **PYTHONPATH**
* Launch it by the **frontend/runserver.sh**(or bat) script
* The HTTP server is by default on the port 81.

## Future of the project?

* Improve the game accessibility 
	* Better tutorial
	* More and better messages about code result
	* Better representation of the empire
* Improve the backend efficiency
	
### Main missing features are:

* Fleet fight scripting

