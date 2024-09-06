#!/usr/bin/python3

import cgi
import cgitb
import os

cgitb.enable()  # Enable CGI traceback for debugging

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
first_name = form.getvalue('first_name')
last_name = form.getvalue('last_name')

# Print the HTTP headers
print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
print("\r\n", end='')

# Start the HTML output
print("<html>")
print("<head>")
print("<title>Hello World - Second CGI Program</title>")
print("</head>")
print("<body>")

print(f"<h2>Hello {first_name} {last_name}</h2>")
if not first_name:
    print("<h2>Error: No input for first name</h2>")
if not last_name:
    print("<h2>Error: No input for last name</h2>")

# End the HTML output
print("</body>")
print("</html>")