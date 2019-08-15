/*

Name: Jake Seawell
Date: 08/10/19
Class: CS-344
Description: This program is the server side of a
one-time-pad (OTP) decription. This server receives
connection from a client on the same port, then receives 
an encripted msg, and a key. This server decripts that msg, and
sends it back to the client, who prints it to stdout

*/

/* LIBRARIES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

//Biggest file is 69333 chars long
#define BUFFER_SIZE 70005

//int plainArr[BUFFER_SIZE];
int keyArr[BUFFER_SIZE]; 
int cyphArr[BUFFER_SIZE];
int plain2Arr[BUFFER_SIZE];


/*
This function counts the number of
chars in a file
*/
int countCharsInFile(char* filename)
{
	int numCharsInFile = 0;
	FILE* filePTR;
	char ch;

	filePTR = fopen(filename, "r");

	if (filePTR == NULL)
		printf("Error\n");
	else{
		while ((ch = fgetc(filePTR)) != EOF)
		numCharsInFile++;
	}

	fclose(filePTR);
	numCharsInFile--;
	return numCharsInFile;
}
/*********************************************/


/*
This function takes chars from a buffer,
converts each to an integer, and then
puts those ints into an array
*/
void bufferToInt(char* buffer, int* whichArr)
{
	int i = 0;
	for (i=0; i<strlen(buffer); i++){
		if (buffer[i] == ' ')
			whichArr[i] = 26;
		else
			whichArr[i] = ((int) buffer[i]) - 65;
	}
}
/*********************************************/


/*
This function takes an array of cyph ints, and an
array of key ints, and decodes into an arr of ints
*/
void decodeArrays(int* cyphArr, int* keyArr, int size)
{
	int i = 0;
	for (i=0; i<size; i++){
		plain2Arr[i] = (cyphArr[i] - keyArr[i] + 27) % 27;	
	}
}
/*********************************************/


/*
This function takes an int arr, converts each to a char,
and prints those chars to a buffer
*/
void printIntToChar (char* buff, int* arr, int size)
{
	char c = ' ';
	int num;

	int i = 0;
	for (i=0; i<size; i++){
		num = arr[i] + 65;
		if (num == 26 || num == 91){
			c = ' ';
		}
		else{
			c = (char) num;
		}
		sprintf(buff + strlen(buff), "%c", c);
	}
	sprintf(buff + strlen(buff), "\n");
}
/*********************************************/


/*
This function reads an int from a file,
and returns it
*/
int read_ints (const char* file_name)
{
	FILE* file = fopen (file_name, "r");
  	int num = 0;
	fscanf (file, "%d", &num);    
  	fclose (file);
	return num;        
}
/*********************************************/


/*
This function sends an err msg to stderr
*/
void error(const char *msg) { fprintf(stderr, "%s\n", msg); } // Error function used for reporting issues

/*********************************************/



//                                       MAIN FUNCTION

/*
This is the main function
*/
int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	
    	socklen_t sizeOfClientInfo;
	
    	char buffer[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];
	char otherBuffer[BUFFER_SIZE];

    	int childPidArr[5];
	pid_t childPid = -5;
	pid_t fifthChildPid = -5;
    	int childExitMethod = -5;
	int checkSend = -5;
    	int numChildren = 0;
	int charsReadSum = 0;
	
/*********************************************/
    
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) { error("SERVER_DEC: ERROR opening socket"); exit(1); }

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to port
		error("SERVER_DEC: ERROR on binding");
		exit(1);
	}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

/*********************************************/

	//Run this server forever (or until killed)
	while(1){	
		
	//If there are 5 children already
		if (numChildren > 4){
	        	waitpid(-1, &childExitMethod, 0); //wait for ANY child to finish before moving on
            		numChildren --;
        	}
        	//Otherwise, if ANY child exists
       	 	else if (waitpid(-1, &childExitMethod, WNOHANG) != 0){ //clean-up ANY child, but dont wait
		      	numChildren --;  
        	}			

        
/*********************************************/     
        
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) { 
			error("SERVER_DEC: ERROR on accept"); 
			//exit(1); 
		}			

/*********************************************/
		
		//Fork new child process to handle encription
		numChildren++;	
		//printf("%d\n", numChildren);
		childPid = fork();

/*********************************************/
        
        //error in fork
		if (childPid == -1){
			error("Error in fork");
		}
	
/*********************************************/
        
        //child process runs this code
		else if (childPid == 0){
			
			//printf("CHILD: Attempting to connect to client\n");		
		
			//Send special message to let client know this is Encoder
			memset(buffer, '\0', BUFFER_SIZE);
			strcat(buffer, "Decoder");
			charsRead = send(establishedConnectionFD, buffer, 7, 0); 

			//printf("CHILD: Connected CLIENT at PORT: %d\n", ntohs(clientAddress.sin_port));

            /*********************************************/
            
			// Get size of cypher from the client
			int cyphCount = -5;
			charsRead = recv(establishedConnectionFD, &cyphCount, sizeof(int), 0);
			//printf("dec: cyph count received: %d\n", cyphCount);
			//If error
			if (charsRead < 0) error("SERV_DEC: ERROR reading cypher count from socket");

            /*********************************************/

			// Get the cypher from the client
			memset(buffer, '\0', BUFFER_SIZE);
			charsReadSum = 0;
			do {
				charsRead = recv(establishedConnectionFD, buffer, cyphCount, 0); // Read the client's message from the socket
				if (charsRead < 0) 
					error("SERV_DEC: ERROR reading cypher from socket");
				else
					charsReadSum = charsReadSum + charsRead;
				strcat(otherBuffer, buffer);
				memset(buffer, '\0', BUFFER_SIZE);
			} while (charsReadSum < cyphCount);
			
            /*********************************************/
            
			//convert cypher string to int array
			bufferToInt(otherBuffer, cyphArr);
			//reset otherBuffer
            		memset(otherBuffer, '\0', BUFFER_SIZE);

            /*********************************************/
            
			// Get size of key from the client
			int keyCount = -5;
			charsRead = recv(establishedConnectionFD, &keyCount, sizeof(int), 0);		
			//printf("dec: key count received: %d\n", keyCount);
			//If error
			if (charsRead < 0) error("SERV_DEC: ERROR reading key count from socket");

            /*********************************************/

			// Get the key from the client
			memset(buffer, '\0', BUFFER_SIZE);
			charsReadSum = 0;
			do {
				charsRead = recv(establishedConnectionFD, buffer, keyCount, 0); // Read the client's message from the socket
				if (charsRead < 0) 
					error("SERV_DEC: ERROR reading key from socket");
				else
					charsReadSum = charsReadSum + charsRead;
				strcat(otherBuffer, buffer);
				memset(buffer, '\0', BUFFER_SIZE);
			} while (charsReadSum < keyCount);
			
            /*********************************************/
            
			//Convert key string to int array
			bufferToInt(otherBuffer, keyArr);

			//decode cypher using key
			decodeArrays(cyphArr, keyArr, cyphCount);

			//Print contents of cypher array as chars: store in buffer2
			printIntToChar(buffer2, plain2Arr, cyphCount);
			
            /*********************************************/
            
			// Send a decoded message back to the client
			charsRead = send(establishedConnectionFD, buffer2, cyphCount, 0); // Send success back
			if (charsRead < 0) error("SERV_DEC: ERROR writing to socket");			
            		//Make sure whole thing sent
			checkSend = -5;
			do
			{
				ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
				//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
			} while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
			if (checkSend < 0) error("SERVER_DEC: ioctl error on decoded msg");  // Check if we actually stopped the loop because of an error
	
            /*********************************************/
            
            //End child process
			close(establishedConnectionFD);
			exit(0);
		}

/*********************************************/
        
		//parent process runs this code
		else{
			//do nothing, loop back to top	
		}
		
/*********************************************/
        
        
	}//END OF WHILE LOOP
	
	close(listenSocketFD); // Close the listening socket

	return 0;	 
}

