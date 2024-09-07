# Notes
- core concept: (don't send \n characters via socket or fifo)
- Remove '\n' from socket_read and buffer_read, and add it to socket_write and buffer_write
  (don't read the new line character, but write it after the buffer)
- implement mutex locks on stdin_running and stdout_running
