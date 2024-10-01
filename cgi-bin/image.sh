#!/bin/bash

# Determine the script's directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Construct the relative path to the image
IMAGE_PATH="$SCRIPT_DIR/tmp/wood.png"

# Check if the image file exists
if [ -f "$IMAGE_PATH" ]; then
	echo -e 'HTTP/1.1 200 OK'
	echo -e 'Content-Type: image/jpeg\r\n\r\n'
    # Output the image content
    cat "$IMAGE_PATH"
else
    # Print an error message if the file does not exist
    echo -e 'HTTP/1.1 404 Not Found'
    echo -e 'Content-Type: text/plain\r\n\r\n'
    echo 'Error: File not found'
fi