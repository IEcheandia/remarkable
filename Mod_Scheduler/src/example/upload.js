let http = require('http');
let https = require('https');
let formidable = require('formidable');
let fs = require('fs');

function pathFromUrl(req) {
    let newpath = req.url;
    // if it contains the file name drop it
    if (!newpath.endsWith("/")) {
        newpath = newpath.substring(0, newpath.lastIndexOf("/") + 1);
    }
    return newpath;
}

function createPath(path) {
    fs.mkdirSync(path, { recursive: true }, (err) => {
        if (err) {
            throw err;
        }
    });
}

function uploadFile(filepath, newpath, res) {
    //Copy the uploaded file to a custom folder
    fs.rename(filepath, newpath, function () {
        //Send a NodeJS file upload confirmation message
        res.write('NodeJS File Upload with POST Success!');
        res.end();
    });
}

function handlePost(req, res) {
    //Create an instance of the form object
    let form = new formidable.IncomingForm();

    //Process the file upload in Node
    form.parse(req, function (error, fields, files) {
        if (files.fileupload === undefined) {
            return;
        }
        let newpath = pathFromUrl(req);
        
        createPath(newpath);
        
        uploadFile(files.fileupload.filepath, newpath + files.fileupload.originalFilename, res)
    });
}

function handlePut(req, res) {
    createPath(pathFromUrl(req));
    
    req.pipe(fs.createWriteStream(req.url))
    req.on('end', function () {
        //Send a NodeJS file upload confirmation message
        res.write('NodeJS File Upload with PUT Success!');
        res.end();
    })
}

function handleRequest(req, res) {
    if (req.method === "POST") {
        handlePost(req, res);
    } else if (req.method === "PUT") {
        handlePut(req, res);
    }
}

http.createServer(handleRequest).listen(8080);

const options = {
  key: fs.readFileSync('key.pem'),
  cert: fs.readFileSync('cert.pem')
};

https.createServer(options, handleRequest).listen(8443);
