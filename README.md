Microshell is a very simple Linux shell written in C language. It's a project required for the Operating System Course at my University.


```
Supported commands: 
CD
Exit
Help
cp
mv
run
Colors handling
```
#Description

Write a simple ANSI C shell program with a name of your choice. This program should accept commands as input, and then perform actions in accordance with their content. Such a coating should:

(a) display a [{path}] prompt, where {path} is the path to the current working directory (1 point)

(b) support the cd shell command, which works analogously to that known to us from the bash shell

(c) support the exit shell command, exit shell program termination should take one optional parameter, which is the exit status returned to the parent process

(d) support the help shell command displaying on the screen information about the author of the program and the functionalities offered by it

(e) support two other shell commands of your choice

(f) accept commands that refer by name to scripts and programs
located in the directories of the described ̨a values of the PATH environment variable and allow these scripts and programs to be called with arguments

(g) accept commands referring by relative and absolute paths to scripts and programs located on the computer disk, and also enable their invocation with arguments

(h) print an error message when the command cannot be interpreted correctly
