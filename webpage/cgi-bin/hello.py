#!/usr/bin/env python3

import time

# time.sleep(5)  # Sleep for 5 seconds
# print("<html><body><h1>Hello from Python CGI after sleep!</h1></body></html>")
import sys, os
# print("Going to sleep\n", file=sys.stderr)
time.sleep(10);
length = 104857600
body = sys.stdin.read(length)

# def eprint(*args, **kwargs):
#     print(*args, file=sys.stderr, **kwargs)

# print("\n\n============================ START\n\n", file=sys.stderr)

# print(body, file=sys.stderr)
# print("\n\n============================\n\n")