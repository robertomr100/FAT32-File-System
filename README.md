-------------------------------------------------------
-------------------------------------------------------
COP4610 Spring 2019            
Project 3 - FAT32 File System  
Due Date: 4/26/2019            
-------------------------------------------------------
-------------------------------------------------------
*Group Members*

    - Roberto Mora
    - Rahul Khatwani
-------------------------------------------------------
-------------------------------------------------------
*Project Files (tar contents)*

1. README.md
        Project details, members, 
        run instructions, features, 
        and progress information

2. Makefile
        Compile instructions for files

3. main.c
        Contains shell, parsing handling, 
        execution, output, all utility 
        functions, and implementation
--------------------------------------------------------
--------------------------------------------------------
*Running the Files*

1. Building the executable: '$ make'

2. Running the executable:  '$ ./exec.out fat32.img'

3. Cleaning:                '$ make clean'
--------------------------------------------------------
--------------------------------------------------------
*List of Available Shell Commands*

1.  exit 
2.  info 
3.  ls DIRNAME 
4.  cd DIRNAME 
5.  size FILENAME 
6.  creat FILENAME 
7.  mkdir DIRNAME 
8.  open FILENAME MODE 
9.  close FILENAME 
10. read FILENAME OFFSET SIZE 
11. write FILENAME OFFSET SIZE STRING 
12. rm FILENAME 
13. rmdir DIRNAME 
--------------------------------------------------------
--------------------------------------------------------

--------------------------------------------------------
--------------------------------------------------------
*Known Bug(s) and Incomplete Part(s)*

- creat built-in: if the directory where it 
is being created is full, our program doesn't
create the new cluster.
--------------------------------------------------------
--------------------------------------------------------
*Program Working Environment*

- Linprog server
--------------------------------------------------------
--------------------------------------------------------