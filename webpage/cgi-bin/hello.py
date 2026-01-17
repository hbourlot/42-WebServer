#!/usr/bin/env python3

import time

# time.sleep(5)  # Sleep for 5 seconds
# print("<html><body><h1>Hello from Python CGI after sleep!</h1></body></html>")
import sys, os
length = 10240
body = sys.stdin.read(length)

# def eprint(*args, **kwargs):
#     print(*args, file=sys.stderr, **kwargs)

print(body)