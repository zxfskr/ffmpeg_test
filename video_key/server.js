'use strict'

var express = require('express');
var serveIndex = require('serve-index');
var http = require('http');
var https = require('https');
var fs = require('fs');

var socketIo = require('socket.io');

var log4js = require('log4js');
var USERCOUNT = 3;

log4js.configure({
    appenders: {
        stdout: { //控制台输出
            type: 'console',
        },
        file: {
            type: 'file',
            filename: './logs/app.log',
            // layout: {
            //     type: 'pattern',
            //     pattern: '%r %p - %m',
            // }
        }
    },
    categories: {
        default: {
            appenders: ['stdout', 'file'],
            level: 'debug'
        }
    }
});

var logger = log4js.getLogger();

var app = express();
app.use(serveIndex('./videos'));
app.use(express.static('./videos'));
// app.use(serveIndex('./public/mediasoup'));
// app.use(express.static('./public/mediasoup'));
//http server
// var http_server = http.createServer(app);
// http_server.listen(3000, '0.0.0.0');

// //https server
let options = {
    key: fs.readFileSync('./cert/example.com+5-key.pem'),
    cert: fs.readFileSync('./cert/example.com+5.pem')
};
var https_server = https.createServer(options, app);

https_server.listen(3000, '0.0.0.0');
logger.info("listen 3000");