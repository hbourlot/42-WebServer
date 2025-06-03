#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()  # Enable error reporting

print("Content-Type: text/html")
print()  # End of headers

form = cgi.FieldStorage()
name = form.getvalue("name", "World")

print(f"""
<html>
  <head><title>Hello</title></head>
  <body>
    <h1>Hello, {name}!</h1>
  </body>
</html>
""")