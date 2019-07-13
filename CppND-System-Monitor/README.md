# CppND-System-Monitor

System Monitor Project in the C++ OOP course. 

## To setup and run:

1. Install `ncurses` package
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```
2. Compile and run
```
g++ -std="c++17" main.cpp -lncurses
./a.out
```
3. In case of error that looks like the following: 
```
root@77e30fca8a01:/home/workspace/CppND-Object-Oriented# ./a.out
*** %n in writable segment detected ***
                                      Aborted (core dumped)
```
just keep trying `./a.out` and it should work eventually!

## Reference
[System-Monitor repository from Udacity C++ Nanodegree course](https://github.com/udacity/CppND-System-Monitor)
