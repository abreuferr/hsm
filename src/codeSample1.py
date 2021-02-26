#!/usr/bin/python

import socket
from struct import *

# Change HSM IP and Port below to match deployment
TCP_IP = '10.20.60.167'
TCP_PORT = 1500
BUFFER_SIZE=1024

# paste command example within inverted commas below:
#e.g.:
COMMAND = '0000NO00'

# 1st two bytes must be command length
SIZE=pack('>h',len(COMMAND))

# join everything together
MESSAGE = SIZE + COMMAND

# Create socket and connect
hsmSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
hsmSocket.connect((TCP_IP, TCP_PORT))

# send MESSAGE
hsmSocket.send(MESSAGE)

# receive
data = hsmSocket.recv(BUFFER_SIZE)

# close socket
hsmSocket.close()
print "sent data (ASCII) : ", MESSAGE
print "sent data (HEX) : ", MESSAGE.encode('hex')
print "received data (ASCII): ", data
print "received data (HEX) : ", data.encode('hex')
