#!/usr/bin/python3

from http import cookies
import os

def show_cookie(c):
    for a, item in c.items():
        print(f"<h1>{a}: {item.value}</h1><br>")

cookie = cookies.BaseCookie()
print("HTTP/1.1 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n\r\n')

if 'HTTP_COOKIE' in os.environ:
    cookie.load(os.environ["HTTP_COOKIE"])
    show_cookie(cookie)
else:
    print("No Cookies Set Yet!")