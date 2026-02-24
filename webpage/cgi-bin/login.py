#!/usr/bin/env python3

import os
import sys

def parse_multipart_form_data():
    """
    Manually parses a multipart/form-data body from stdin.
    This is a simplified parser for demonstration.
    """
    form_data = {}
    
    # 1. Get the boundary from the CONTENT_TYPE environment variable
    content_type = os.environ.get("CONTENT_TYPE", "")
    boundary = None
    if "boundary=" in content_type:
        boundary = "--" + content_type.split("boundary=")[1]

    if not boundary:
        return {}

    # 2. Read the entire body from stdin
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(content_length)

    # 3. Split the body into parts using the boundary
    parts = body.split(boundary)

    # 4. Process each part
    for part in parts:
        if 'Content-Disposition: form-data; name="' in part:
            # Extract the name of the form field
            try:
                header, value = part.split('\\r\\n\\r\\n', 1)
                name_part = header.split('name="')[1]
                name = name_part.split('"')[0]
                
                # Clean up the value
                value = value.strip('\\r\\n--')
                form_data[name] = value
            except ValueError:
                # Skip malformed parts
                continue
                
    return form_data

def main():
    data = parse_multipart_form_data()

    username = data.get("username", "")
    password = data.get("password", "")

    # --- Hardcoded credentials for demonstration ---
    if username == "admin" and password == "1234":
        # On success, send a custom header with the username and a Location hint.
        # Your C++ server will see these headers, create a session, and then
        # send the actual redirect response to the browser.
        print("Status: 200 OK")
        print(f"X-Authenticated-User: {username}")
        print("Location: /dashboard") # The page to redirect to after login
        print("Content-Type: text/html")
        print() # This creates the necessary blank line between headers and body
        print("<html><body><h1>Login Successful, redirecting...</h1></body></html>")
    else:
        # On failure, redirect back to the home page.
        print("Status: 302 Found")
        print("Location: /") # Redirect back to the home/login page
        print()

if __name__ == "__main__":
    main()
