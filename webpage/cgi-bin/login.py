#!/usr/bin/env python3

import json
import os
import re
import sys
import urllib.parse

def parse_multipart_form_data():
    form_data = {}

    content_type = os.environ.get("CONTENT_TYPE", "")
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))

    if content_length <= 0:
        return form_data

    body_bytes = sys.stdin.buffer.read(content_length)

    if "application/x-www-form-urlencoded" in content_type:
        parsed = urllib.parse.parse_qs(body_bytes.decode("utf-8", errors="replace"), keep_blank_values=True)
        for key, values in parsed.items():
            form_data[key] = values[0] if values else ""
        return form_data

    if "multipart/form-data" not in content_type:
        return form_data

    boundary_match = re.search(r'boundary=([^;]+)', content_type)
    if not boundary_match:
        return form_data

    boundary = b"--" + boundary_match.group(1).encode("utf-8", errors="replace")
    parts = body_bytes.split(boundary)

    for part in parts:
        if b"Content-Disposition" not in part:
            continue

        headers_raw, separator, value_raw = part.partition(b"\r\n\r\n")
        if not separator:
            continue

        name_match = re.search(br'name="([^"]+)"', headers_raw)
        if not name_match:
            continue

        name = name_match.group(1).decode("utf-8", errors="replace")
        value = value_raw.rstrip(b"\r\n-")
        form_data[name] = value.decode("utf-8", errors="replace")

    return form_data

def main():
    data = parse_multipart_form_data()

    username = data.get("username", "")
    password = data.get("password", "")



    # --- Hardcoded credentials for demonstration ---
    if username == "admin" and password == "1234":
        print("Status: 200 OK")
        print(f"X-Authenticated-User: {username}")
        print("Content-Type: application/json; charset=UTF-8")
        print()
        print(json.dumps({
            "success": True,
            "message": "Login successful",
            # "redirect": "/dashboard"
        }))
    else:
        print("Status: 200 OK")
        print("Content-Type: application/json; charset=UTF-8")
        print()
        print(json.dumps({
            "success": False,
            "message": "Invalid username or password"
        }))

if __name__ == "__main__":
    main()
