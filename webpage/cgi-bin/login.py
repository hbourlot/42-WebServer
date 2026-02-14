#!/usr/bin/env python3

import os
import sys
import urllib.parse

def parse_post():
    content_length = os.environ.get("CONTENT_LENGTH")
    if not content_length:
        return {}

    length = int(content_length)
    body = sys.stdin.read(length)

    return urllib.parse.parse_qs(body)

def main():
    data = parse_post()

    username = data.get("username", [""])[0]
    password = data.get("password", [""])[0]

    if username == "admin" and password == "1234":
        print("Status: 200 OK")
        print("Content-Type: text/plain")
        print("\r\n\r\n")
        print("Login successful")
    else:
        print("Status: 401 Unauthorized")
        print("Content-Type: text/plain")
        print("\r\n\r\n")
        print("Invalid credentials")

if __name__ == "__main__":
    main()
