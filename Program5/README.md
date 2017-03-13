CSS 432 Winter 2017
Final Assignment (FTP Client)

The program (ftp.cpp) is a simple ftp client.

How to compile:
Compile with command g++ -o ftp ftp.cpp

Running the program:
- This program takes one argument, the hostname of the ftp server.

Run with command:
./ftp servername 

The servername I ran and tested this program with is ftp.tripod.com.

FTP Client commands
- open : opens connection to ftp server (must run this command first to run other commands properly)
- close : closes connection to ftp server
- quit : closes connection to ftp server and stops the client
- cd subdir : changes directory to given subdirectory
- ls : lists all files and subdirectories in current working directory
- get filename: gets filename file from server and places copy in local directory
- put: Requests a filename and a new filename, Puts copy of filename file in local directory
	   in ftp server with new filename
- help: prints out list of ftp commands