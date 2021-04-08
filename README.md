# bunsh
A simple Bourne-style shell written as an exercise in IPC mechanisms in Linux.
Supports pipelines, stdin/stdout redirection to files, and background jobs.


## Installation

```sh
make
```
The shell depends on [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) for tab completion and history navigation. Most distros seem to ship it, but on a Debian-like system it could be installed with
```sh
sudo apt-get install libreadline-dev
```

## Example
```
~/bunsh$ ./bin/bunsh
/home/user/bunsh> help

 Usage:

   command [ | command ]*

 where command is an absolute path or something that can be found through $PATH.

 Redirections such as '< infile' and '> outfile' may appear after a command.
 Appending an '&' to a line will run the job in the background.

 Commands defined internally:
  cd [dir]
  exit
  help

/home/user/bunsh> cd src/
/home/user/bunsh/src> ls . | xargs wc -l | grep total > ../lines
/home/user/bunsh/src> cd ..
/home/user/bunsh> tee < lines | tr [:lower:] [:upper:]
  728 TOTAL
/home/user/bunsh>
```

