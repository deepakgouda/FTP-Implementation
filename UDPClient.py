from socket import *

serverIP = '127.0.0.1'
serverPort = 8080

clientSocket = socket(AF_INET, SOCK_DGRAM)
message = input("Enter the message : ")
clientSocket.sendto(message.encode(), (serverIP, serverPort))

response, serverAddr = clientSocket.recvfrom(2048)
print(response.decode())
clientSocket.close()
