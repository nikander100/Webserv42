import os
from http import cookies
import cgi, cgitb

cgitb.enable()

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
key = form.getvalue('key')

# Create a cookie
cookie = cookies.SimpleCookie()
if 'HTTP_COOKIE' in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])

# Prepare the response body
if key and key in cookie:
    response_body = f"The Value of Cookie '{key}' is {cookie[key].value}"
else:
    response_body = "Cookie was not found!"

# Print headers
print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/plain", end='\r\n')
print("\r\n", end='')  # This prints the blank line separating headers from the body

# Print body
print(response_body, end='')