/*

Name: Jake Seawell
Date: 08/10/19
Class: CS-344
Description: This program is the client side of a
one-time-pad (OTP) decription. This client connects
to a server on the same port, and sends the server
an encripted msg, and a key. The server decripts that msg, and
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
/***********************************/

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

	//return 1 for bad, 0 for all good
	return bad;
}
/**************************************/


/*
This function sends an err msg to stderr
*/
void error(const char *msg) { fprintf(stderr, "%s\n", msg); } // Error function used for reporting issues

/**************************************/



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
	
    	int cyphCount = -5;
	int keyCount = -5;   
	int checkSend = -5;
	int charsReadSum = 0;
	
    	FILE* filePTR2;
	FILE* filePTR3;
 
	//If incorrect arguments
	if (argc < 4) { fprintf(stderr,"USAGE: %s msg key port\n", argv[0]); exit(1); } // Check usage & args


/*
CONNECT TO SERVER ON SOCKET
*/
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { error("CLIENT_DEC: ERROR, no such host"); exit(2); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) { error("CLIENT_DEC: ERROR opening socket"); exit(2); }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("CLIENT_DEC: ERROR connecting");
		exit(2);
	}

/******************************************/    
    
/*
Make sure connection is with DEC server, not ENC
*/	
	//reset buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse		
	//Receive special msg
	charsRead = recv(socketFD, buffer, 7, 0); // Read data from the socket, leaving \0 at end
	if (strcmp(buffer, "Decoder") != 0) { error("CLIENT_DEC: Must connect to SERVER_DEC only"); exit(2); }

/*******************************************/  
    
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

/*************************************/    
    

//GET SIZE OF CYPHER

	cyphCount = countCharsInFile(argv[1]);
	
//GET SIZE OF KEY

	keyCount = countCharsInFile(argv[2]);
	
//if key is too small

	if (keyCount < cyphCount) { error("CLIENT_DEC: Error. Key is too small"); exit(1); }
	
/************************************/
    
/*    
SEND SIZE OF cypher
*/
	charsWritten = send(socketFD, &cyphCount, sizeof(int), 0);
	//If error
	if (charsWritten < 0) error("CLIENT_DEC: ERROR writing cypher count to socket");
	//Make sure whole thing sent
    	checkSend = -5;
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_DEC: ioctl error on cypher count");  // Check if we actually stopped the loop because of an error

/**************************************/  
    
/*
SEND CYPHER
*/
	//reset buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse		

	//Open/Read cypher, store contents in buffer
	filePTR2 = fopen(argv[1], "r");
	fgets(buffer, BUFFER_SIZE -1, filePTR2);
	
	//close file
	fclose(filePTR2);

	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that file adds

/*************************************/

/*	
Send message to server
*/
	charsWritten = send(socketFD, buffer, cyphCount, 0); // Write to the server
	
	//If error
	if (charsWritten < 0) error("CLIENT_DEC: ERROR writing cypher to socket");
	else if (charsWritten < strlen(buffer)) error("CLIENT: WARNING: Not all cypher data written to socket!");

	//Make sure it all sent
    	checkSend = -5;
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_DEC: ioctl error on cypher");  // Check if we actually stopped the loop because of an error
	
	//reset buffer
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

/**************************************/
    
/*
SEND SIZE OF KEY
*/
	charsWritten = send(socketFD, &keyCount, sizeof(int), 0);

	//If error
	if (charsWritten < 0) error("CLIENT_DEC: ERROR writing key count to socket");
	//Make sure it all sent
    	checkSend = -5;
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_DEC: ioctl error on key count");  // Check if we actually stopped the loop because of an error
	
/************************************/    
    
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

/***********************************/

/*	
Send message to server
*/
	charsWritten = send(socketFD, buffer, keyCount, 0); // Write to the server
	
	//If error
	if (charsWritten < 0) error("CLIENT_DEC: ERROR writing key to socket");
	else if (charsWritten < strlen(buffer)) error("CLIENT_DEC: WARNING: Not all key data written to socket!");

    	//Make sure it all sent
	checkSend = -5;  // Holds amount of bytes remaining in send buffer
	do
	{
		ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		//printf("checkSend: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
	}
	while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
	if (checkSend < 0) error("CLIENT_DEC: ioctl error on key");  // Check if we actually stopped the loop because of an error

/**********************************/  
    
/*
Get decoded message from server
*/	
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	//Loop until whole msg is sent
	charsReadSum = 0;
	do {
		charsRead = recv(socketFD, buffer, cyphCount, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) 
			error("CLIENT_DEC: ERROR reading decoded msg from socket");
		else
			charsReadSum = charsReadSum + charsRead;			
		//concet buffer onto other buffer
		strcat(otherBuffer, buffer);
		//reset buffer
		memset(buffer, '\0', sizeof(buffer));
	} while (charsReadSum < cyphCount);
	
/*******************************/
    
/*    
Output decoded msg to stdout
*/
	printf("%s\n", otherBuffer);

/*******************************/

//Close socket and exit function

	close(socketFD); // Close the socket
	
	//Exit function
	return 0;
}
