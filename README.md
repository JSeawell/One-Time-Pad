# One-Time-Pad  

## Description: 
This program simulates a one-time-pad encryption and decryption between a server and a client. The *otp_enc_d* server opens a socket for listening on a port, and the *otp_enc* client connects to the server on the port. Then, the client sends a message and a key to the server. The server will encript the message using the key, and send the encripted message back to the client. Then the process is repeated - *otp_dec* client sends the encripted message and the same key to the *otp_enc_d* server, which will decode the message using the key, and send the decoded message back to the client. The server runs in a continuos loops, and can accept up to 5 concurrent client connections.  

## What I learned:
> 1. Network communication  
> 2. Clients and server architectures  
> 3. Security, encription, and permissions  
> 4. Network layer model  
> 5. HTTP/TCP/UDP/IP  
> 6. Sockets, ports, and pipes  
> 7. Concurrency (apparent vs. real)

## How to compile:   
To compile this program, download the files in this repository, and put them in the same bash directory. Give the files `compileall` & `keygen` executable permissions by running the commands: `chmod +x compileall` & `chmod +x keygen`. The compileall program will compile all the other C++ files into the necessary executables. There will be some errors thrown by the compiler, but these can be ignored. The keygen program will be used later.  

## How to run:  
1. Start the daemons (servers). This is done by running the commands: `otp_enc_d PORT# &` and `otp_dec_d PORT# &` with 2 different valid port#'s.  

2. Create a message: Move into or create a file in the directory, with whatever message you want to encode. Your message needs to be made up of uppercase letters and spaces, but no lowercase letters or other characters.  

3. Creake a key: This is done by running the `keygen` program: `keygen # > keyFile`. In this case, `#` is the size of key you want to create, and `keyFile` is the name of the file where you want to store the key. In order for this program to work, the key needs to be the same # of characters (or more) as the message you will be encripting.  

4. Encript the message: This can be done by typing: `otp_enc messageFile keyFile PORT# > encriptedFile` where `messageFile` is the name of the file where your message is stored, `keyFile` is where your key is stored, and the `PORT#` needs to be the same port that you ran *otp_enc_d* on in step #1. Your encripted message will be stored in a new file called `encriptedFile`.  

5. Decode the message: Do this by typing: `otp_dec encriptedFile keyFile PORT# > decodedMessage`, where the `PORT#` needs to be the same as that of *otp_dec_d* in step #1, and `decodedMessage` is a new file where your decoded message will be stored.  

## Example Run (after moving all files into bash directory):  
> `$ chmod +x compileall`  
> `$ chmod +x keygen`  
> `$ compileall`  
> `$ cat messageFile`  
> HELLO WORLD  
> `$ opt_enc_d 54321 &`  
> `$ opt_dec_d 56789 &`  
> `$ keygen 15 > keyFile`  
> `$ opt_enc messageFile keyFile 54321 > encriptedFile`  
> `$ cat encriptedFile`  
> F JIWKSO ABROQV  
> `$ opt_dec encriptedFile keyFile 56789 > decodedMessage`  
> `$ cat decodedMessage`  
> HELLO WORLD  
