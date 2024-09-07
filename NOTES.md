# Notes
- Remove '\n' from socket_read and buffer_read, and add it to socket_write and buffer_write
  (don't read the new line character, but write it after the buffer)
