/*

Name: Jake Seawell
Date: 08/10/19
Class: CS-344
Description: This program is the client side of a
one-time-pad (OTP) encription. This client connects
to a server on the same port, and sends the server
a msg, and a key. The server encripts that msg, and
sends it back to this client, who prints it to stdout

*/

/* LIBRARIES */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

//biggest file is 69333 chars long
#define BUFFER_SIZE 70005

/*
This function counts how many chars are
in a file
*/
int countCharsInFile(char* filename)
{
	FILE* filePTR;
	char ch;
	int count = -1;
	
	//open file
	filePTR = fopen(filename, "r");

	if (filePTR == NULL)
		printf("Error\n");
	else{
		while ((ch = fgetc(filePTR)) != EOF){
			//count chars
			count++;
		}
	}
	//close file
	fclose(filePTR);

	//return count
	return count;
}
/*********************************************/

/*
This function checks a file for any bad chars
*/
int checkFileForBadChars(char* filename)
{
	int bad = 0;
	int number;
	FILE* filePTR4;
	char ch4;
	
	//open file
	filePTR4 = fopen(filename, "r");

	if (filePTR4 == NULL){
		fprintf(stderr, "Error: %s does not exist\n", filename);
		exit(1);
	}
	else{
		while ((ch4 = fgetc(filePTR4)) != EOF){
			number = (int) ch4;
			//if bad char detected
			if ((number < 65 || number > 90) && number != 32 && number != 10){
				bad = 1;
				break;
			}	
		}
	}
	//close file
	fclose(filePTR4);

	//return 1 if bad, 0 if all good
	return bad;
}
/******************************************/

/*
This function sends an err msg to stderr
*/
void error(const char *msg) { fprintf(stderr, "%s\n", msg); }

/**********************************************/




//                                       MAIN FUNCTION
/*
This is the main function
*/
int main(int argc, char *argv[])
{
	//Declare/Initialize vars

	int socketFD, portNumber, charsWritten, charsRead;
	
    	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	
    	char buffer[BUFFER_SIZE];
	char otherBuffer[BUFFER_SIZE];
	
    	int plainCount = -5;
	int keyCount = -5;
	int checkSend = -5;
	int charsReadSum = 0;
	
    	FILE* filePTR2;
	FILE* filePTR3;   
 
	//If incorrect arguments
	if (argc < 4) { fprintf(stderr,"USAGE: %s msg key port\n", argv[0]); exit(1); }


/*
CONNECT TO SERVER ON SOCKET
*/

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { error("CLIENT_ENC: ERROR, no such host"); exit(2); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) { error("CLIENT_ENC: ERROR opening socket"); exit(2); }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("CLIENT_ENC: ERROR connecting");
		exit(2);
	}

/********************************************************/


/*
Make sure connection is with ENC server, not DEC
*/	
	//reset buffer
	memset(buffer, '\0', sizeof(buffer));		
	//Receive special msg
	charsRead = recv(socketFD, buffer, 7, 0); // Read data from the socket
	if (strcmp(buffer, "Encoder") != 0) { error("CLIENT_ENC: Must connect to SERVER_ENC only"); exit(2); }

/**********************************************/
    
/*
Check plaintext and key for bad chars
*/

	if (checkFileForBadChars(argv[1]) == 1){
		fprintf(stderr, "%s has 1 or more bad characters\n", argv[1]);
		exit(1);
	}
	else if (checkFileForBadChars(argv[2]) == 1){
		fprintf(stderr, "%s has 1 or more bad characters\n", argv[2]);
		exit(1);
	}

/**********************************************/

    
//GET SIZE OF PLAINTEXT

	plainCount = countCharsInFile(argv[1]);
	
//GET SIZE OF KEY

	keyCount = countCharsInFile(argv[2]);

//if key is too small

	if (keyCount < plainCount) { error("Error. Key is too small"); exit(1); }
	
/**********************************************/
 
    
/*
SEND SIZE OF PLAINTEXT
*/

	charsWritten = send(socketFD, &plainCount, sizeof(int), 0);
    	//If error
	if (charsWritten < 0) error("CLIENT_ENC: ERROR writing to socket");
	// Make sure whole thing is sent
	checkSend = -5;
    	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_ENC: ioctl error on plainCount");  // Check if we actually stopped the loop because of an error
    
/*****************************************************/    

/*
SEND PLAINTTEXT
*/
	//reset buffer
	memset(buffer, '\0', sizeof(buffer));		

	//Open/Read plaintext, store contents in buffer
	filePTR2 = fopen(argv[1], "r");
	fgets(buffer, BUFFER_SIZE -1, filePTR2);
	
	//close file
	fclose(filePTR2);

	//remove the trailing \n that file adds
	buffer[strcspn(buffer, "\n")] = '\0'; 
/**************************************************/
	
/*
Send message to server
*/

	charsWritten = send(socketFD, buffer, plainCount, 0); // Write to the server

	//If error
	if (charsWritten < 0) error("CLIENT_ENC: ERROR writing to socket");
	else if (charsWritten < strlen(buffer)) error("CLIENT: WARNING: Not all data written to socket!");
    
	// Make sure whole thing is sent
	checkSend = -5;
    	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_ENC: ioctl error on plaintext");  // Check if we actually stopped the loop because of an error

	//reset buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

/************************************************/
    
/*    
SEND SIZE OF KEY
*/
	
    	charsWritten = send(socketFD, &keyCount, sizeof(int), 0);
    	//If error
	if (charsWritten < 0) error("CLIENT_ENC: ERROR writing to socket");
   	 // Make sure whole thing is sent
	checkSend = -5;
    	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	
	//If error
	if (checkSend < 0) error("CLIENT_ENC: ioctl error on keyCount");  // Check if we actually stopped the loop because of an error
	
/************************************************/
  
/*	
SEND KEY
*/

	//reset buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse		
    
	//Open/Read key, store contents in buffer
	filePTR3 = fopen(argv[2], "r");
	fgets(buffer, BUFFER_SIZE - 1, filePTR3);
	
	//close file
	fclose(filePTR3);

	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that file adds

/********************************************/
	
/*
Send message to server
*/

	charsWritten = send(socketFD, buffer, keyCount, 0); // Write to the server
	//If error
	if (charsWritten < 0) error("CLIENT_ENC: ERROR writing to socket");
	else if (charsWritten < strlen(buffer)) error("CLIENT_ENC: WARNING: Not all data written to socket!");
    	//Make sure whole msg sent
	checkSend = -5;  // Holds amount of bytes remaining in send buffer
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_ENC: ioctl error on key");  // Check if we actually stopped the loop because of an error

/********************************************/    
    
/*
Receive encoded message from server
*/	
	//Reset buffers
	memset(buffer, '\0', sizeof(buffer));
	memset(otherBuffer, '\0', sizeof(otherBuffer));
	//Loop until whole msg is sent
	charsReadSum = 0;
	do {
		charsRead = recv(socketFD, buffer, plainCount, 0); // Read data from the socket
		if (charsRead < 0) 
			error("CLIENT: ERROR reading from socket");
		else
			charsReadSum = charsReadSum + charsRead;
		strcat(otherBuffer, buffer);
		memset(buffer, '\0', sizeof(buffer));
	} while (charsReadSum < plainCount);
	
/********************************************/
    
/*    
Output encoded msg to stdout
*/
	printf("%s\n", otherBuffer);

	//Reset buffer
	memset(otherBuffer, '\0', sizeof(otherBuffer));

/*********************************************/  
  
//CLOSE SOCKET AND EXIT

	close(socketFD); // Close the socket
	
	//Exit function
	return 0;
}
