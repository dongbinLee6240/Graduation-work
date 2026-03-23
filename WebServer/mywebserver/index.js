const express = require('express');
const app = express();
const mysql = require('mysql2');
const bcrypt = require('bcrypt');  // 비밀번호 암호화를 위한 bcrypt 모듈
const port = 3000;
const crypto = require('crypto');
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// MySQL 연결 설정
const connection = mysql.createConnection({
  host: 'localhost',  // MySQL 서버가 동작하는 주소 (로컬에서는 'localhost')
  user: 'root',   // MySQL 사용자 이름
  password: '9804',   // 해당 사용자의 비밀번호
  database: 'usertable'  // 연결할 데이터베이스 이름
});

// MySQL 서버에 연결
connection.connect(err => {
  if (err) {
    console.error('Error connecting to the database:', err);
    return;
  }
  console.log('Connected to the MySQL database.');
});

// 간단한 라우트 설정
app.get('/', (req, res) => {
  res.send('Hello, World!');
});

// 사용자 ID 해시화 함수
function hashID(id) {
  return crypto.createHash('sha256').update(id).digest('hex');
}

// 회원가입 (비밀번호 해시 저장)
app.post('/register', async (req, res) => {
  const { username, id, password } = req.body;

  try {
    const hashedPassword = await bcrypt.hash(password, 10);
    const hashedUID = hashID(id);  // ID를 해시화하여 UID로 사용

    connection.query
    (
      'INSERT INTO users (UID, username, id, password) VALUES (?, ?, ?, ?)',
      [hashedUID, username, id, hashedPassword],
      (err, results) => {
        if (err) {
          console.error("Database error: ", err);
          res.status(500).send('Database error during registration');
          return;
        }
        res.send('Registration successful!');
      }
    );
    
  } 
  catch (err) 
  {
    res.status(500).send('Error during registration');
  }
});

app.post('/character', async (req, res) => {
  try {
    const { uid,charactername, class_type} = req.body;

    if (!uid) {
      return res.status(401).json({ error: '로그인이 필요합니다' });
    }

    const hashedcid = hashID(charactername);

    connection.query(
      'INSERT INTO characters (character_id, user_id, class, charactername, job) VALUES (?, ?, ?, ?, ?)',
      [hashedcid, uid, class_type,charactername, class_type],
      (err, result) => {
        if (err) {
          console.error('캐릭터 생성 중 오류: ', err);
          return res.status(500).json({ error: '캐릭터 생성 실패.' });
        }
        res.status(200).json({ message: '캐릭터가 성공적으로 생성되었습니다.' });
      }
    );
  } catch (err) {
    console.error('서버 오류: ', err);
    res.status(500).json({ error: '서버 오류 발생' });
  }
});


// 로그인 처리 (해시된 비밀번호 검증)
app.post('/login', async (req, res) => {
  const { id, password } = req.body;

  // 데이터베이스에서 사용자 정보 확인
  connection.query(
    'SELECT * FROM users WHERE id = ?',
    [id],
    async (err, results) => {
      if (err) {
        res.status(500).send('Database error');
        return;
      }

      if (results.length === 0) {
        res.status(400).send('User not found');
        return;
      }

      // 데이터베이스에서 가져온 해시된 비밀번호
      const storedHashedPassword = results[0].password;

      // 입력된 비밀번호와 데이터베이스에 저장된 해시된 비밀번호 비교
      const isMatch = await bcrypt.compare(password, storedHashedPassword);

      if (isMatch) {
        const uid = results[0].UID;

        // 캐릭터가 존재하는지 확인
        connection.query('SELECT COUNT(*) AS characterCount FROM characters WHERE user_id = ?', [uid], (err, characterResults) => {
          if (err) {
            res.status(500).send('Database error');
            return;
          }

          const hasCharacter = characterResults[0].characterCount > 0;

          // 로그인 성공 시 UID와 캐릭터 존재 여부를 응답
          res.status(200).json({
            message: 'Login successful!',
            uid: uid,
            hasCharacter: hasCharacter // 캐릭터가 존재하는지 여부
          });
        });
      } else {
        res.status(400).send('Login failed');
      }
    }
  );
});

app.post('/get_characters', (req, res) => {
  const { uid } = req.body; // UID로 캐릭터 정보 조회

  if (!uid) {
    return res.status(400).json({ error: 'UID is required' });
  }

  // MySQL에서 해당 UID의 캐릭터 정보 가져오기
  connection.query(
    'SELECT charactername, class,character_id FROM characters WHERE user_id = ?',
    [uid],
    (err, results) => {
      if (err) {
        console.error('Database error: ', err);
        return res.status(500).json({ error: 'Database error' });
      }
      
      // 캐릭터 리스트 응답
      res.status(200).json({
        characters: results
      });
    }
  );
});

// GET 요청 (사용자 정보 가져오기)
app.get('/users/:id', (req, res) => {
  const userId = req.params.id;
  connection.query('SELECT * FROM users WHERE id = ?', [userId], (err, results) => {
    if (err) {
      res.status(500).send('Database error');
      return;
    }
    if (results.length === 0) {
      res.status(404).send('User not found');
      return;
    }
    res.json({
      UID: results[0].UID,
      username: results[0].username,
      id: results[0].id,
      created_at: results[0].created_at
    });
  });
});

// 서버 실행
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
