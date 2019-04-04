# vizier_cpp

This reposistory containes a C++ implementation of the Vizier framework (https://github.com/robotarium/vizier).  

# Tested on 

* Ubuntu 16.04
* Ubuntu 17.10

# Required Libraries

This repo requires the libraries
* mosquitto c mqtt library
* pthread

To install mosquitto run
```
	sudo apt-get install mosquitto
```

The pthread library should already be installed.  Bazel automatically pulls in the other dependencies on compilation.

# Compilation

## Bazel

This project is compiled with Bazel.  For more information, see [https://bazel.build/](Bazel).

## Instructions

Navigate to the home directory and run 
```
	bazel build //vizier...
```


