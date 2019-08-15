#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) 
{ 
    	//seed random number - call this only once per program
	srand(time(0));

	//If more than 1 argument is given, err to stderr
	if (argc != 2){
		fprintf(stderr, "Wrong # of arguments given:\n1 expected\n%d given\n", argc-1);
	}
	else{
		//vars needed
		int i = 0;
		int numChars = 0;
		int randNum = 0;
		char randChar = ' ';
	 
		numChars = atoi(argv[1]);
		
		//Get random number [0-26]
		for (i=0; i<numChars; i++){
			randNum = rand() % 27;
			//26 = ' ' 
			if (randNum == 26)
				randChar = ' ';
			//add 65 to get ASCII uppercase
			else{ 
				randNum = randNum + 65;
				//cast into char
				randChar = (char) randNum;
			}
			//print to stdout
			printf("%c", randChar);
		}
		//add newline to end
		printf("\n");
	}

   	return 0; 
}
