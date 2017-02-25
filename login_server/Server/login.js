var express = require('express');
var bodyParser = require('body-parser');
var cookieParser = require('cookie-parser');
var mysql = require('mysql');
var redis = require('redis');
var redisClient = redis.createClient(6379, '192.168.1.201');
var md5 = require('md5');
var sha256 = require('sha256');
var uuid = require('uuid/v4');
var nodemailer = require('nodemailer');

var mysqlPool = mysql.createPool({
    connectionLimit : 20,
    host : '192.168.1.251',
    user : 'togater',
    password : 'togater',
    database : 'togater'
});

var smtpTransport = nodemailer.createTransport("SMTP", {
    service: 'Gmail',
    auth: {
        user: 'servercamp.togater',
        pass: 'togater123'
    }
});

var winston = require('winston');
var moment = require('moment');

winston.add ( winston.transports.File, {
  level: 'debug',
  json: false,
  filename: 'login.log',
  timestamp: function(){
      return moment().format("YYYY-MM-DD HH:mm:ss.SSS");
  }
});

var app = require('express')();
var server = require('http').Server(app);

server.listen(3000, function(){
  console.log('listen......');
});

app.use(express.static('public'));
app.use(cookieParser());
app.use(bodyParser.urlencoded({extended : false}));

var headers = {
  "Content-Type": "text/html",
  'ACCESS-CONTROL-ALLOW-CREDENTIALS' : 'true',
  'ACCESS-CONTROL-ALLOW-ORIGIN' : '*',
  'ACCESS-CONTROL-ALLOW-METHODS' : 'GET, POST, OPTIONS',
  'ACCESS-CONTROL-ALLOW-HEADERS' : 'Accept, X-Access-Token, X-Application-Name, X-Request-Sent-Time'
};

app.get('/', function(request, response){
  response.send('/public/index.html');
});

app.post('/login', function (request, response) {
    redisClient.get(md5(sha256(request.body.id)), function(err, reply) {
        if(reply === null){

            var userID = request.body.id;
            var encryptedPassword = sha256(request.body.password);

          mysqlPool.getConnection(function(err, connection){
              connection.query('select * from user_info where id =  BINARY(?) and password =  BINARY(?)', [userID, encryptedPassword] , function(err, secondRows){
                response.writeHead(200, headers);

                if(secondRows.length === 1)   //만약 로그인에 성공했다면
                {
                    var accessToken = md5(sha256(userID));
                    response.end('ok:' + accessToken);
                    redisClient.set(accessToken, userID);
                    winston.info('Login : ' + userID + ' : -> Success');
                }
                else if(secondRows.length === 0)  //로그인 실패
                {
                    response.end('login fail');
                    winston.info('Login : ' + userID + ' -> Can\' Find ID');
                }
                connection.release();

                if(err)
                  console.log(err);
              });
            });
        }
        else{
          winston.info('Login : ' + request.body.id + ' -> Already Connect ID');
          response.writeHead(200, headers);
          response.end('already login');
        }
    });
});

app.post('/forgot/id', function(request, response){
  mysqlPool.getConnection(function(err, connection){
      connection.query('select id from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
          response.writeHead(200, headers);

          if(rows.length === 1)   //만약 ID를 찾았다면
          {
              response.end(rows[0].id);
              winston.info('Find ID : ' + rows[0].id + ' -> Success');
          }
          else if(rows.length === 0)  //찾기 실패
          {
              response.end('find fail');
              winston.info('Find ID -> Fail');
          }
          connection.release();
          if(err)
            console.log(err);
      });
  });
});


app.post('/forgot/password/request', function(request, response){
  let id = request.body.id;
  let resetUUID = uuid();

  mysqlPool.getConnection(function(err, connection){
      connection.query('select email from user_info where id =  BINARY(?)', [id] , function(err, rows){
          if(rows.length === 1){
              let mailOptions = {
                  from: '로그인 서버 <servercamp.togater@gmail.com>',
                  to: rows[0].email,
                  subject: 'TOGATER 로그인 서버 입니다.',
                  html: '비밀번호를 재설정 하려면 아래 토큰을 해당 페이지에서 입력 후 재설정 하시기 바랍니다. <p>토큰 : ' + resetUUID + '</p>'
              };

              smtpTransport.sendMail(mailOptions, function(err, res){
                  if (err){
                      winston.info(err);
                  } else {
                      winston.info("Password Reset Email Sent -> Success");
                      response.send('sent email');
                      redisClient.set(resetUUID, id);
                  }
              });
              connection.release();
          }else{
              winston.info("Password Reset Email Sent -> Can\' Find Email");
              response.send('fail email sent');
          }
      });
  });
});

app.post('/forgot/password/auth', function(request, response){

  let uuid = request.body.uuid;

  redisClient.get(uuid, function(err, reply) {
      if(reply){
           winston.info("password Reset Email Auth -> Success");
           response.send('success auth');
           redisClient.del(uuid);
      }else{
           response.send('fail reset password');
           winston.info("password Reset Email Auth -> Fail");
      }
  });
});

app.post('/forgot/password/reset', function(request, response){
  let id = request.body.id;
  let newPassword = sha256(request.body.password);

  mysqlPool.getConnection(function(err, connection){
      connection.query('update user_info set password = BINARY(?) where id =  BINARY(?)', [newPassword, id] , function(err, rows){
          response.send('success');
          winston.info("Password Reset -> Success");
          connection.release();
      });
  });
});

app.post('/register/overlap/email', function (request, response) {
  mysqlPool.getConnection(function(err, connection){
      connection.query('select * from user_info where email =  BINARY(?)', [request.body.email] , function(err, rows){
        response.writeHead(200, headers);
        if(rows.length === 1)   //email 중복
        {
            winston.info("Email Check -> Overlap");
            response.end('email overlap');
        }
        else if(rows.length === 0)  //중복 아님
        {
            winston.info("Email Check -> Success");
            response.end('email ok');
        }
        connection.release();

        if(err){
            winston.error("Email Check -> ERROR");
        }
      });
  });
});

app.post('/register/overlap/id', function (request, response) {
  mysqlPool.getConnection(function(err, connection){
      connection.query('select * from user_info where id =  BINARY(?)', [request.body.id] , function(err, rows){
        response.writeHead(200, headers);
        if(rows.length === 1)   //id 중복
        {
            winston.info("ID Check -> Overlap");
            response.end('id overlap');
        }
        else if(rows.length === 0)  //중복 아님
        {
            winston.info("ID Check -> Success");
            response.end('id ok');
        }

        connection.release();

        if(err){
            winston.error("ID Check -> ERROR");
        }
      });
  });
});

app.post('/register', function (request, response) {
  mysqlPool.getConnection(function(err, connection){
      var userID = request.body.id;
      var password = request.body.password;
      var email = request.body.email;
      var encryptedPassword = sha256(password);

      connection.query('insert into user_info (id, password, email, win, lose, rating) values ( BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?), BINARY(?))',
      [userID, encryptedPassword, email, 0, 0, 400], function(err, rows){
          response.writeHead(200, headers);
          response.end('register ok');
          winston.info("Register -> Success");
          connection.release();

          if(err){
              winston.error("Register -> ERROR");
          }
      });
  });
});

app.post('/dashboard/logout', function(request, response){
  redisClient.del(request.body.userToken);
  response.send('redis delete');
  winston.info("Homepage Logout -> Success");
});

app.post('/dashboard/withdrawal', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('delete from user_info where id =  BINARY(?)', [reply] , function(err, rows){
                if(err)
                    winston.error("Withdrawal -> ERROR");
                else{
                    redisClient.del(request.body.userToken);
                    response.send('success');
                    winston.info("Account Withdrawal -> Success");
                }
            connection.release();
              });
          });
      }
  });
});

app.post('/dashboard/profile', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('select win, lose, rating from user_info where id =  BINARY(?)', [reply] , function(err, rows){

                  response.send({
                      "win" : rows[0].win,
                      "lose" : rows[0].lose,
                      "rating" : rows[0].rating
                  });
                  winston.info("Get Profile Info -> Success");
            connection.release();
              });
          });
      }
  });
});

app.post('/dashboard/account', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('select id, email from user_info where id =  BINARY(?)', [reply] , function(err, rows){
                  response.send({
                      "ID" : rows[0].id,
                      "email" : rows[0].email
                  });
                  winston.info("Get Account Info -> Success");
                  connection.release();
              });
          });
      }
  });
});

app.post('/dashboard/update/email', function(request, response){
  redisClient.get(request.body.userToken, function(err, reply) {
      if(reply){
          mysqlPool.getConnection(function(err, connection){
              connection.query('select email from user_info where email = BINARY(?)', [request.body.email] , function(err, rows){
                  if(rows.length === 1){    //email 중복
                      response.writeHead(200, headers);
                      response.end('overlap email');
                      winston.info("Update Email -> Overlap");
                      connection.release();
                      return;
                  }
                  connection.query('update user_info set email = BINARY(?) where id =  BINARY(?)', [request.body.email,reply] , function(err, rows){
                      response.writeHead(200, headers);
                      response.end('update email');
                      winston.info("Update Email -> Success");
                      connection.release();
                  });

              });
          });
      }
  });
});
