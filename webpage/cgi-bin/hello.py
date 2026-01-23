#!/usr/bin/env python3

## open another terminal and run "python3 terminal_connection.py /dev/pts/3"
## tty to check the properly pts

import time, sys, os

# time.sleep(5)  # Sleep for 5 seconds
# print("<html><body><h1>Hello from Python CGI after sleep!</h1></body></html>")
# print("Going to sleep\n", file=sys.stderr)
time.sleep(10)
body = sys.stdin.read()


fd3 = os.open("/dev/pts/3", os.O_WRONLY)
os.write(fd3, b"\n\n========== START ==========\n\n")

os.write(fd3, body.encode())
os.write(fd3, b"\n\n========== END ==========\n\n")
os.close(fd3)
