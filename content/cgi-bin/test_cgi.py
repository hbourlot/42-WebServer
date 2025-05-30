#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/plain\r\n");
print("---- CGI Environment ----");
for key, value in os.environ.items():
	print(f"{key}={value}");
 
print("\n---- Request Body ----");
body = sys.stdin.read();
print(body);
 