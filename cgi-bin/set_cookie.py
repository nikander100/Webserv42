#! /usr/bin/python3

import os
from http import cookies
# Import modules for CGI handling 
import cgi, cgitb 

cgitb.enable()

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
key = form.getvalue('key')
value  = form.getvalue('value')

# Create a cookie
cookie = cookies.SimpleCookie()

# Prepare the response body
if key and value:
    cookie[key] = value
    response_body = f"<html><body>\n<p>Key: {key}</p>\n<p>Value: {value}</p>\n</body></html>"
else:
    response_body = "<html><body><p>Error: Missing key or value</p></body></html>"


print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
print(cookie.output(), end='\r\n')
print("\r\n", end='')

print(response_body, end='')