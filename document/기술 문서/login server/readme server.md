# Login Server
---
Node.js(version: 6.9.4)를 기반으로 개발된 Login Server

#### 개발 환경
---
* 개발 OS : Windows 10
* 서버 개발 Editor : Atom Editor
* 클라이언트 개발 Editor : Unity3D 5.5
* 개발 DB : MySQL 5.7.17


* 서버 OS : Windows Server 2016
* 서버 DB : MySQL 5.5.53
* 캐쉬 DB : Redis

#### 주요 기능
---
* 회원가입 (자체 DB에 저장)
* 사용자 인증 (ID를 해싱하여 AccessToken 생성)
* 회원 탈퇴
* 패스워드 암호화
* 유저 개인정보 수정
* ID 찾기 / 패스워드 초기화 (이메일 인증)
* WebGL로 빌드된 Unity3D 웹 클라이언트

#### Rest API
---
방식 | API 주소 | 기능 | 파라미터 | 리턴 값
----------|----------|----------|----------|--------
POST | /login | 사용자 로그인, 중복 로그인 체크 | ID, password |성공 : 인증 토큰, 중복 로그인 : 'already login'
POST | /forgot/id |이메일을 확인하여 아이디 찾기 | email |성공 : ID, 실패 : 'find fail'
POST | /forgot/password/request | 패스워드 초기화 요청, 인증메일 발송 | ID |성공 : 'sent email', 실패 : 'fail email sent'
POST | /forgot/password/auth | 인증토큰 확인 | 인증 UUID |성공 : 'success auth', 실패 : 'fail reset password'
POST | /forgot/password/reset | 패스워드 초기화 | ID, password |성공 : 'success'
POST | /register/overlap/email | 회원가입 시 이메일 중복 체크 | email |성공 : 'email ok', 실패 : 'email overlap'
POST | /register/overlap/id | 회원가입 시 ID 중복 체크 | ID |성공 : 'id ok', 실패 : 'id overlap'
POST | /register | 실제 회원가입 요청 | ID, password, email |성공 : 'register ok'
POST | /dashboard/logout | AccessToken을 제거하여 로그아웃 | AccessToken |성공 : 'redis delete'
POST | /dashboard/withdrawal | 회원 탈퇴 | AccessToken |성공 : 'success'
POST | /dashboard/profile | 대시보드 상에서 게임 프로필 요청 | AccessToken |성공 : {"win" : win, "lose" : lose, "rating" : rating}
POST | /dashboard/account | 대시보드 상에서 게임 계정 요청 | AccessToken |성공 : {"ID" : ID, "email" : email}
POST | /dashboard/update/email | 대시보드 상에서 이메일 수정 | AccessToken |성공 : 'update email', 실패 : 'overlap email'

#### 종속 라이브러리
---
* body-parser : 1.16.0 (Post data 파싱)
* connect-redis : 3.2.0 (Redis 연결)
* cookie-parser : 1.4.3 (쿠키 파싱)
* express : 4.14.0 (베이스 프레임워크)
* md5 : 2.2.1
* moment : 2.17.1 (달력 관련 라이브러리)
* mysql : 2.12.0 (Mysql 연결)
* nodemailer : 0.7.1 (SMTP)
* redis : 2.6.5
* sha256 : 0.2.0
* uuid : 3.0.1 (UUID 생성)
* winston : 2.3.1 (로그)
* pm2 : 2.4.0 (management loadbalancing & failover)
