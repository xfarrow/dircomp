# dircomp
## Name
dircomp - A directory comparison tool

## Synopsis
`dircomp directory1 directory2 [-rvfdh]`

## Description
dircomp is a program for evaluating whether two directories are equivalent.

Two directories are considered to be equivalent if and only if these conditions apply:
* They have the same number of files and folders;
* For each file in a directory, there is the same file in the other. Two files are considered to be the same 
if they have the same name, including extension, and same content;
* For each folder in a directory, there is a folder with the same name in the other.

If the recursive option is applied, then another condition must hold true:
* For each folder in a directory, there is an equivalent folder in the other, with the same name.

The options are as follows:
```
  -r             Recursive
  -v             Verbose
  -f             Fast. Halt as soon as the directories are found to be not equal             
  -d             Comparison by digest
  -h             Print this help and quit
```

## Correctness
You can have a look at the code and at its [mathematical proof](https://github.com/xfarrow/dircomp/wiki/Mathematical-correctness-of-dircomp) to validate its correctness.
