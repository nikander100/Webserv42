#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()

# Get filename here
fileitem = form['filename']

# Directory to save the uploaded file
upload_dir = os.path.join(os.getcwd(), 'cgi-bin', 'tmp')

# Ensure the directory exists
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

# Test if the file was uploaded
if fileitem.filename:
   open(os.getcwd() + '/cgi-bin/tmp/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp' + 'and uploaded'
   print("HTTP/1.1 201 CREATED", end='\r\n')
else:
   print("HTTP/1.1 500 INTERNAL SERVER ERROR", end='\r\n')
   message = 'Uploading Failed'


print("Content-Type: text/html", end='\r\n\r\n')
print("<H1> " + message + " </H1>")
