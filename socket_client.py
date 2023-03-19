#!/usr/bin/env python3

"""

References:
- https://docs.python.org/3/library/socket.html

"""

import socket

"""

import socket
s1, s2 = socket.socketpair()
b1 = bytearray(b'----')
b2 = bytearray(b'0123456789')
b3 = bytearray(b'--------------')
s1.send(b'Mary had a little lamb')
s2.recvmsg_into([b1, memoryview(b2)[2:9], b3])
[b1, b2, b3]

"""

HOST = 'localhost'
PORT = 9091


def main():
  """

    Main Function

  """
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.connect((HOST, PORT))
    client.sendall(b'Hello World')
    data = client.recv(1024)
  print('Received: ', repr(data))





if __name__ == '__main__':
  main()

