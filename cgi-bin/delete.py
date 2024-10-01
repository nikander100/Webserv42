#!/usr/bin/env python3

import os
import cgi
import cgitb

cgitb.enable()

def delete_file(path):
    try:
        if os.path.isfile(path):
            os.remove(path)
            return True
        else:
            return False
    except Exception as e:
        print(f"<html><body><h1>Error: {e}</h1></body></html>")
        return False

def delete_directory(path):
    try:
        if os.path.isdir(path):
            shutil.rmtree(path)
            return True
        else:
            return False
    except Exception as e:
        print(f"<html><body><h1>Error: {e}</h1></body></html>")
        return False

def main():
    # Print headers
    print("HTTP/1.1 200 OK", end='\r\n')
    print("Content-Type: text/html", end='\r\n')
    print("\r\n", end='')  # This prints the blank line separating headers from the body

    form = cgi.FieldStorage()
    method = form.getvalue('_method')

    if method == 'DELETE':
        # Get the current working directory
        current_dir = os.getcwd()
        # Construct the target path relative to the current directory
        target_path = os.path.join(current_dir, "cgi-bin", "tmp", "wood.png")
        if delete_file(target_path):
            print("<html><body><h1>Deletion successful</h1></body></html>")
        else:
            print(f"<html><body><h1>Deletion failed: Target does not exist {target_path}</h1></body></html>")
    elif method == 'DELETEALL':
        current_dir = os.getcwd()
        if delete_directory(current_dir):
            print("<html><body><h1>Deletion successful</h1></body></html>")
        else:
            print(f"<html><body><h1>Deletion failed: Target does not exist {current_dir}</h1></body></html>")
    else:
        print(f"<html><body><h1>Invalid method: {method}</h1></body></html>")

if __name__ == "__main__":
    main()