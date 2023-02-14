# Geometric Algorithm demos
interactive 'GUI'/demo for some basic geometric algorithms, graphics using SFML

Made with [SFML 2.5](https://www.sfml-dev.org/tutorials/2.5). On Linux, Install SFML with:
```
sudo apt-get install libsfml-dev
```
Each folder is a separate and independent algorithm/demo with their own (mostly ientical) makefiles. 
[Commands for compiling](https://www.sfml-dev.org/tutorials/2.5/start-linux.php) are in the makefile. 
Although note that this will NOT [link it statically](https://www.sfml-dev.org/faq.php#build-link-static). 
The graphical interface for most executables will not give hints toward how to operate the demo. 
Read the corresponding readme for that demo to see what the expected behaviour is. 
If not specified, a common theme might be left-mouse-clicks place the input on the screen and a right click will calculate and display whatever the algorithm is supposed to do. 
A middle-mouse-button click might clear the canvas (reset).

Build and run using: (respectively)
```
make exe
./exe
```
or the equivalent
```
make run
```
