This directory contains an example HTTP server (nodejs) which supports everything our scheduler needs for http connections.

To install (on Debian based systems):

    sudo apt install nodejs npm

Install npm modules:

    npm install formidable
    npm install fs

To generate the requires SSL certificates do:

    openssl genrsa -out key.pem
    openssl req -new -key key.pem -out csr.pem
    openssl x509 -req -days 9999 -in csr.pem -signkey key.pem -out cert.pem
    rm csr.pem

Now one can run the server with:

    node upload.js

The server opens port 8080 (http) and 8443 (https).
