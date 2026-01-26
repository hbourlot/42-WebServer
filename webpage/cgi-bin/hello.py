#!/usr/bin/env python3

## open another terminal and run "python3 terminal_connection.py /dev/pts/3"
## tty to check the properly pts

import time, sys, os

# time.sleep(5)  # Sleep for 5 seconds
# print("<html><body><h1>Hello from Python CGI after sleep!</h1></body></html>")
# length = 10240
body = sys.stdin.read()

print(body[:100])  # Print only first 100 chars to avoid hanging on large bodies