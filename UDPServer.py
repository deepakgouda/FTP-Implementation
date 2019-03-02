from socket import *

serverPort = 8080

serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', serverPort))
print("Server online...")

while True:
	message, clientAddr = serverSocket.recvfrom(2048)
	response = "Server's message : Hello "+ \
							message.decode()+"!"
	serverSocket.sendto(response.encode(), clientAddr)

# import socket

# # hostName = socket.gethostname()
# # hostIP = socket.gethostbyname(hostName)
# hostName = 'www.intranet.iitg.ernet.in'
# hostIP = socket.gethostbyname(hostName)
# print(hostName, hostIP)