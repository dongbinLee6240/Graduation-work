-- 1. 데이터베이스 생성
CREATE DATABASE usertable;
USE usertable;

-- 2. users 테이블 생성
CREATE TABLE users (
    UID VARCHAR(64) PRIMARY KEY,
    username VARCHAR(50),
    id VARCHAR(50) UNIQUE,
    password VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 3. characters 테이블 생성\
CREATE TABLE characters (
    character_id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(64),
    class VARCHAR(50),
    charactername VARCHAR(50),
    job VARCHAR(50),
    FOREIGN KEY (user_id) REFERENCES users(UID)
);

show databases;

use usertable;

show tables;

describe users;

desc characters;

select * from users;
select * from characters;
