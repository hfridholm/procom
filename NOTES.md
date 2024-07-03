# Notes
- Remove '\n' from socket_read and buffer_read, and add it to socket_write and buffer_write

- first try to create client socket,
  if it doesn't work, it means that no server is running,
  then create server socket.
  (no "server" argument is needed)
