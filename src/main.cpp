#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <ctime>
#include <climits>

using namespace std;

// Haris
// ============ data ============

const int MAX_USERS = 100;
const int MAX_FOUND = 200;
const int MAX_LOST = 200;
const int MAX_CLAIMS = 400;
const int MAX_LOGIN_ATTEMPTS = 3;
const int MAX_CLAIM_ATTEMPTS = 3;
const int MIN_PASSWORD_LENGTH = 6;

const std::string STATUS_AVAILABLE = "Tersedia";
const std::string STATUS_RETURNED = "Dikembalikan";

const int REG_OK = 0;
const int REG_EMPTY_FIELD = 1;
const int REG_USERNAME_HAS_SPACE = 2;
const int REG_DUPLICATE = 3;
const int REG_PASSWORD_SHORT = 4;
const int REG_PHONE_INVALID = 5;
const int REG_FULL = 6;

const int LOGIN_NOT_FOUND = -1;
const int LOGIN_WRONG_PASSWORD = -2;
const int LOGIN_LOCKED = -3;

const int ITEM_OK = 0;
const int ITEM_EMPTY_FIELD = 1;
const int ITEM_INVALID_DATE = 2;
const int ITEM_FUTURE_DATE = 3;
const int ITEM_FULL = 4;

const int CLAIM_APPROVED = 0;
const int CLAIM_WRONG_DETAIL = 1;
const int CLAIM_TOO_MANY_ATTEMPTS = 2;
const int CLAIM_ITEM_NOT_FOUND = 3;
const int CLAIM_NOT_AVAILABLE = 4;
const int CLAIM_OWN_ITEM = 5;
const int CLAIM_STORAGE_FULL = 6;

std::string userName[MAX_USERS], userPassword[MAX_USERS], userPhone[MAX_USERS];
int userFailed[MAX_USERS];
bool userLocked[MAX_USERS];
int userCount = 0;

int foundReporter[MAX_FOUND];
std::string foundName[MAX_FOUND], foundCategory[MAX_FOUND],
    foundLocation[MAX_FOUND], foundDate[MAX_FOUND], foundDescription[MAX_FOUND],
    foundSecret[MAX_FOUND], foundStatus[MAX_FOUND];
int foundCount = 0;

int lostReporter[MAX_LOST];
std::string lostName[MAX_LOST], lostCategory[MAX_LOST], lostLocation[MAX_LOST],
    lostDate[MAX_LOST], lostDescription[MAX_LOST];
int lostCount = 0;

int claimItem[MAX_CLAIMS], claimUser[MAX_CLAIMS], claimFailed[MAX_CLAIMS];
int claimCount = 0;

int searchResult[MAX_FOUND];
int searchResultCount = 0;

// FUNGSI: loadDemoData   (flowchart: "Data kosong?" -> "Isi contoh")
// TUJUAN: menyediakan data contoh saat belum ada pengguna atau laporan.
// Penjelasan: Fungsi ini mengisi dua pengguna dan dua barang temuan hanya jika data utama masih kosong.
// CONTOH: data kosong -> pengguna Azhri dan Payung Hitam ditambahkan.
void loadDemoData()
{
  if (userCount == 0 && foundCount == 0 && lostCount == 0)
  {

    userName[0] = "Azhri";
    userPassword[0] = "Azhri123";
    userPhone[0] = "081234567890";
    userFailed[0] = 0;
    userLocked[0] = false;

    userName[1] = "Arya";
    userPassword[1] = "Arya123";
    userPhone[1] = "089876543210";
    userFailed[1] = 0;
    userLocked[1] = false;

    foundReporter[0] = 0;
    foundName[0] = "Payung Hitam";
    foundCategory[0] = "Aksesori";
    foundLocation[0] = "Kantin Utama";
    foundDate[0] = "2026-10-25";
    foundDescription[0] = "Payung lipat warna hitam merk standar";
    foundSecret[0] = "Ada stiker kecil bergambar bintang di pegangannya";
    foundStatus[0] = STATUS_AVAILABLE;

    foundReporter[1] = 1;
    foundName[1] = "Dompet Kulit Coklat";
    foundCategory[1] = "Dompet";
    foundLocation[1] = "Parkiran Fakultas";
    foundDate[1] = "2026-10-26";
    foundDescription[1] = "Dompet berisi kartu identitas dan beberapa kartu penting";
    foundSecret[1] = "Ada foto pelajar di dalam selipan kartu";
    foundStatus[1] = STATUS_AVAILABLE;

    userCount = 2;
    foundCount = 2;
  }
}

// Done
// ============ string_utils ============

// FUNGSI: trim   (flowchart: "Teks masuk" -> "Buang spasi tepi")
// TUJUAN: menghapus spasi di awal dan akhir teks.
// Penjelasan: Fungsi ini mencari batas karakter bukan spasi lalu mengembalikan bagian teks di antaranya.
// CONTOH:   buku   -> buku.
std::string trim(std::string s)
{
  int left = 0;
  int right = s.length();

  while (left < right && s[left] == ' ')
  {
    left++;
  }

  while (right > left && s[right - 1] == ' ')
  {
    right--;
  }

  return s.substr(left, right - left);
}

// FUNGSI: toLower   (flowchart: "Teks" -> "Huruf kecil")
// TUJUAN: menyamakan kapitalisasi untuk perbandingan.
// Penjelasan: Fungsi ini mengubah setiap karakter pada salinan teks menjadi huruf kecil.
// CONTOH: BukuA -> bukua.
std::string toLower(std::string s)
{
  int length = s.length();
  for (int i = 0; i < length; i++)
  {
    s[i] = tolower(s[i]);
  }

  return s;
}

// FUNGSI: containsIgnoreCase   (flowchart: "Normalisasi" -> "Cari kata")
// TUJUAN: mencari kata tanpa membedakan huruf besar dan kecil.
// Penjelasan: Fungsi ini menormalkan teks dan kata kunci lalu mencari kecocokan berurutan.
// CONTOH: Tas Biru, biru -> true.
bool containsIgnoreCase(std::string text, std::string keyword)
{
  text = toLower(text);
  keyword = toLower(keyword);

  if (keyword == "")
  {
    return true;
  }

  int keywordLength = keyword.length();
  int textLength = text.length();
  for (int i = 0; i + keywordLength <= textLength; i++)
  {
    bool match = true;

    for (int j = 0; j < keywordLength; j++)
    {
      if (text[i + j] != keyword[j])
      {
        match = false;
        break;
      }
    }

    if (match)
    {
      return true;
    }
  }
  return false;
}

// FUNGSI: sanitizeField   (flowchart: "Teks input" -> "Teks aman file")
// TUJUAN: membuat teks aman untuk disimpan dalam satu baris file.
// Penjelasan: Fungsi ini mengganti delimiter dan karakter baris baru dengan /.
// CONTOH: A dan B dipisah newline -> A/B.
std::string sanitizeField(std::string s)
{
  int length = s.length();
  for (int i = 0; i < length; i++)
  {
    if (s[i] == '|' || s[i] == '\n' || s[i] == '\r')
    {
      s[i] = '/';
    }
  }

  return s;
}

// FUNGSI: isAllDigits   (flowchart: "Teks" -> "Semua angka?")
// TUJUAN: memeriksa bahwa string tidak kosong dan seluruh karakternya angka.
// Penjelasan: Fungsi ini menolak string kosong atau karakter selain digit.
// CONTOH: 08123 -> true; 08-123 -> false.
bool isAllDigits(std::string s)
{
  if (s == "")
  {
    return false;
  }

  int length = s.length();
  for (int i = 0; i < length; i++)
  {
    bool isDigit = isdigit(s[i]);

    if (!isDigit)
    {
      return false;
    }
  }

  return true;
}

// FUNGSI: splitLine   (flowchart: "Baris" -> "Field-field")
// TUJUAN: memecah satu baris file ke array parts untuk modul storage.
// Penjelasan: Fungsi ini memecah satu baris menjadi field berdasarkan delimiter hingga batas array.
// CONTOH: "1|andi|08", '|', parts, 4 -> 3 dan parts berisi "1", "andi", "08".
int splitLine(std::string line, char delimiter, std::string parts[],
              int maxParts)
{
  std::string part = "";
  int amount = 0;

  int length = line.length();
  for (int i = 0; i < length; i++)
  {
    if (line[i] == delimiter)
    {
      if (amount < maxParts)
      {
        parts[amount] = part;
        amount++;
      }

      part = "";
    }
    else
    {
      part += line[i];
    }
  }

  if (amount < maxParts)
  {
    parts[amount] = part;
    amount++;
  }

  return amount;
}

// Done
// ============ date_utils ============

// FUNGSI: isLeapYear   (flowchart: "Tahun" -> "Kabisat?")
// TUJUAN: menentukan apakah suatu tahun adalah tahun kabisat.
// Penjelasan: Fungsi ini menerapkan aturan habis dibagi 400 atau habis dibagi 4 tetapi bukan 100.
// CONTOH: 2024 -> true; 2100 -> false.
bool isLeapYear(int year)
{
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

// FUNGSI: daysInMonth   (flowchart: "Bulan+tahun" -> "Jumlah hari")
// TUJUAN: mengembalikan jumlah hari dalam bulan tertentu.
// Penjelasan: Fungsi ini membedakan bulan 30, 31, dan Februari berdasarkan tahun kabisat.
// CONTOH: 2, 2024 -> 29.
int daysInMonth(int month, int year)
{
  switch (month)
  {
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    return 31;

  case 4:
  case 6:
  case 9:
  case 11:
    return 30;

  case 2:
    if (isLeapYear(year))
    {
      return 29;
    }
    return 28;

  default:
    return 0;
  }
}

// FUNGSI: isValidDate   (flowchart: "Tanggal" -> "Format?" -> "Bulan/hari?")
// TUJUAN: memeriksa format YYYY-MM-DD dan batas hari tiap bulan.
// Penjelasan: Fungsi ini memvalidasi pemisah, digit, bulan, dan hari termasuk tahun kabisat.
// CONTOH: 2025-02-30 -> false; 2025-02-28 -> true.
bool isValidDate(std::string date)
{
  if (date.length() != 10)
    return false;

  if (date[4] != '-' || date[7] != '-')
    return false;

  for (int i = 0; i < 10; i++)
  {
    if (i == 4 || i == 7)
    {
      continue;
    }

    if (date[i] < '0' || date[i] > '9')
    {
      return false;
    }
  }

  // convertion
  // ascii '0' = 48
  int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
             (date[2] - '0') * 10 + (date[3] - '0');
  int month = (date[5] - '0') * 10 + (date[6] - '0');
  int day = (date[8] - '0') * 10 + (date[9] - '0');

  if (year < 1)
    return false;

  if (month < 1 || month > 12)
    return false;

  if (day < 1 || day > daysInMonth(month, year))
  {
    return false;
  }

  return true;
}

// FUNGSI: getTodayDate   (flowchart: "Jam komputer" -> "YYYY-MM-DD")
// TUJUAN: mengembalikan tanggal lokal hari ini.
// Penjelasan: Fungsi ini memakai waktu lokal komputer dan memformatnya sebagai YYYY-MM-DD.
// CONTOH: pada 2025-08-17 -> 2025-08-17.
std::string getTodayDate()
{
  // bagian ini sudah jadi karena memakai <ctime> yang belum diajarkan
  time_t now = time(0);
  tm *local = localtime(&now);
  char result[11];
  strftime(result, sizeof(result), "%Y-%m-%d", local);
  return std::string(result);
}

// FUNGSI: isFutureDate   (flowchart: "Tanggal valid?" -> "Bandingkan hari ini")
// TUJUAN: menentukan apakah tanggal laporan lebih besar dari tanggal hari ini.
// Penjelasan: Fungsi ini menolak tanggal tidak valid lalu membandingkan format YYYY-MM-DD dengan tanggal lokal.
// CONTOH: besok -> true.
bool isFutureDate(std::string date)
{
  if (isValidDate(date) == false)
    return false;

  std::string today = getTodayDate();

  return date > today;
}

// Done
// ============ id_utils ============

// FUNGSI: toDisplayNo   (flowchart: "Indeks" -> "+1" -> "Nomor layar")
// TUJUAN: mengubah indeks array mulai 0 menjadi nomor layar mulai 1.
// Penjelasan: Fungsi ini menambahkan satu pada indeks untuk ditampilkan kepada pengguna.
// CONTOH: 0 -> 1.
int toDisplayNo(int index) { return index + 1; }

// FUNGSI: toIndex   (flowchart: "Nomor layar" -> "-1" -> "Indeks")
// TUJUAN: mengubah nomor layar mulai 1 menjadi indeks array mulai 0.
// Penjelasan: Fungsi ini mengurangi satu dari nomor yang dipilih pengguna.
// CONTOH: 1 -> 0.
int toIndex(int displayNo) { return displayNo - 1; }

// Done
// ============ input_helpers ============

// FUNGSI: readLine   (flowchart: "Prompt" -> "getline" -> "Teks")
// TUJUAN: menampilkan prompt lalu membaca satu baris lengkap.
// Penjelasan: Fungsi ini memakai getline agar input boleh mengandung spasi.
// CONTOH: prompt Nama, input Tas Biru -> Tas Biru.
std::string readLine(std::string prompt)
{
  cout << prompt;
  std::string result;
  getline(cin, result);
  return result;
}

// FUNGSI: readInt   (flowchart: "Input teks" -> "Angka valid?" -> "Dalam batas?")
// TUJUAN: membaca bilangan dalam batas yang ditentukan.
// Penjelasan: Fungsi ini mengulang input sampai berisi digit, tidak overflow, dan berada dalam batas.
// CONTOH: input 2 dengan batas 1..3 -> 2.
int readInt(std::string prompt, int minValue, int maxValue)
{
  while (true)
  {
    std::string text = readLine(prompt);

    // Memeriksa apakah input hanya terdiri dari angka
    if (!isAllDigits(text))
    {
      cout << "Masukkan angka yang benar.\n";
      continue;
    }

    // Mengubah teks menjadi bilangan bulat
    else
    {
      int result = 0;
      bool overflow = false;

      for (int i = 0; i < (int)text.length(); i++)
      {
        int digit = text[i] - '0';

        // Cek batas integer sebelum perkalian dan penjumlahan
        if (result > (INT_MAX - digit) / 10)
        {
          overflow = true;
          break;
        }

        result = result  * 10 + digit;

      }

      // Memeriksa apakah terjadi integer overflow
      if (overflow)
      {
        cout << "Angka terlalu besar. Silahkan masukkan angka kembali.\n";
        continue;
      }

      // Memeriksa apakah angka berada dalam batas
      if (result >= minValue && result <= maxValue)
      {
        return result;
      }
      cout << "Masukkan angka dari " << minValue << " sampai " << maxValue
           << ".\n";
    }
  }
}

// FUNGSI: pauseScreen   (flowchart: "Tampil pesan" -> "Enter" -> "Lanjut")
// TUJUAN: memberi pengguna waktu membaca hasil sebelum kembali ke menu.
// Penjelasan: Fungsi ini menunggu pengguna menekan Enter melalui readLine.
// CONTOH: setelah pesan berhasil, Enter kembali ke menu.
void pauseScreen() { readLine("Tekan Enter untuk lanjut..."); }

// FUNGSI: printHeader   (flowchart: "Judul" -> "Garis" -> "Judul")
// TUJUAN: membuat judul layar konsisten pada setiap menu.
// Penjelasan: Fungsi ini mencetak garis pemisah sebelum dan sesudah judul.
// CONTOH: Menu Pengguna tampil di antara dua garis.
void printHeader(std::string title)
{
  std::cout << "\n==============================\n";
  std::cout << title << "\n";
  std::cout << "==============================\n";
}

// FUNGSI: loadAll   (flowchart: "Buka file" -> "Baris valid?" -> "Isi array")
// TUJUAN: memuat pengguna, laporan, dan percobaan klaim dari file data.
// Penjelasan: Fungsi ini mereset count lalu membaca setiap file yang tersedia ke array paralel.
// CONTOH: users.txt tidak ada -> userCount tetap 0.
void loadAll()
{
  // 1. Reset seluruh count ke 0 terlebih dahulu
  userCount = 0;
  foundCount = 0;
  lostCount = 0;
  claimCount = 0;

  // --- Memuat users.txt ---
  std::ifstream userFile("users.txt");
  if (userFile.is_open())
  {
    std::string line;
    while (getline(userFile, line))
    {
      if (userCount >= MAX_USERS)
      {
        break;
      }
      std::string parts[5];
      int partsCount = splitLine(line, '|', parts, 5);
      if (partsCount == 5)
      {
        userName[userCount] = parts[0];
        userPassword[userCount] = parts[1];
        userPhone[userCount] = parts[2];

        // Konversi string failed ke integer secara manual
        int failedVal = 0;
        for (int i = 0; i < (int)parts[3].length(); i++)
        {
          if (parts[3][i] >= '0' && parts[3][i] <= '9')
          {
            failedVal = failedVal * 10 + (parts[3][i] - '0');
          }
        }
        userFailed[userCount] = failedVal;
        userLocked[userCount] = (parts[4] == "1");

        userCount++;
      }
    }
    userFile.close();
  }

  // --- Memuat found_items.txt ---
  std::ifstream foundFile("found_items.txt");
  if (foundFile.is_open())
  {
    std::string line;
    while (getline(foundFile, line))
    {
      if (foundCount >= MAX_FOUND)
      {
        break;
      }
      std::string parts[8];
      int partsCount = splitLine(line, '|', parts, 8);
      if (partsCount == 8)
      {
        int repIndex = 0;
        for (int i = 0; i < (int)parts[0].length(); i++)
        {
          if (parts[0][i] >= '0' && parts[0][i] <= '9')
          {
            repIndex = repIndex * 10 + (parts[0][i] - '0');
          }
        }

        foundReporter[foundCount] = repIndex;
        foundName[foundCount] = parts[1];
        foundCategory[foundCount] = parts[2];
        foundLocation[foundCount] = parts[3];
        foundDate[foundCount] = parts[4];
        foundDescription[foundCount] = parts[5];
        foundSecret[foundCount] = parts[6];
        foundStatus[foundCount] = parts[7];

        foundCount++;
      }
    }
    foundFile.close();
  }

  // --- Memuat lost_reports.txt ---
  std::ifstream lostFile("lost_reports.txt");
  if (lostFile.is_open())
  {
    std::string line;
    while (getline(lostFile, line))
    {
      if (lostCount >= MAX_LOST)
      {
        break;
      }
      std::string parts[6];
      int partsCount = splitLine(line, '|', parts, 6);
      if (partsCount == 6)
      {
        int repIndex = 0;
        for (int i = 0; i < (int)parts[0].length(); i++)
        {
          if (parts[0][i] >= '0' && parts[0][i] <= '9')
          {
            repIndex = repIndex * 10 + (parts[0][i] - '0');
          }
        }

        lostReporter[lostCount] = repIndex;
        lostName[lostCount] = parts[1];
        lostCategory[lostCount] = parts[2];
        lostLocation[lostCount] = parts[3];
        lostDate[lostCount] = parts[4];
        lostDescription[lostCount] = parts[5];

        lostCount++;
      }
    }
    lostFile.close();
  }

  // --- Memuat claim_attempts.txt / claims.txt ---
  std::ifstream claimFile("claims.txt");
  if (claimFile.is_open())
  {
    std::string line;
    while (getline(claimFile, line))
    {
      if (claimCount >= MAX_CLAIMS)
      {
        break;
      }
      std::string parts[3];
      int partsCount = splitLine(line, '|', parts, 3);
      if (partsCount == 3)
      {
        int cItem = 0, cUser = 0, cFailed = 0;

        for (int i = 0; i < (int)parts[0].length(); i++)
          if (parts[0][i] >= '0' && parts[0][i] <= '9')
            cItem = cItem * 10 + (parts[0][i] - '0');
        for (int i = 0; i < (int)parts[1].length(); i++)
          if (parts[1][i] >= '0' && parts[1][i] <= '9')
            cUser = cUser * 10 + (parts[1][i] - '0');
        for (int i = 0; i < (int)parts[2].length(); i++)
          if (parts[2][i] >= '0' && parts[2][i] <= '9')
            cFailed = cFailed * 10 + (parts[2][i] - '0');

        claimItem[claimCount] = cItem;
        claimUser[claimCount] = cUser;
        claimFailed[claimCount] = cFailed;

        claimCount++;
      }
    }
    claimFile.close();
  }
}

// PEMILIK TODO: Muflih
// FUNGSI: saveAll   (flowchart: "Array" -> "Tulis tiap record" -> "Tutup file")
// TUJUAN: menyimpan seluruh data saat layar UI berhasil mengubah data.
// TODO:
//  [ ] 1. Buka tiap file output dan loop dari 0 sampai count terkait.
//  [ ] 2. Tulis satu record per baris dengan pemisah '|', mengikuti urutan yang tertulis di data-model.md.
//  [ ] 3. Tulis bool locked sebagai 0 atau 1.
//  [ ] 4. Pastikan teks sudah sanitizeField sebelum masuk array atau sebelum ditulis.
//  [ ] 5. Tutup file setelah selesai; fungsi ini tidak boleh cout/cin.
// CONTOH: user indeks 0 menjadi "andi|rahasia|08123|0|0".
// HATI-HATI: jangan menulis foundSecret ke layar, tetapi tetap wajib ditulis ke found_items.txt untuk data klaim.
// SELESAI JIKA: [ ] M-STORAGE-2 lulus.
// JANGAN ubah nama, parameter, atau tipe return fungsi ini tanpa lapor semua pemilik modul.
void saveAll()
{
  // Penyimpanan file bersifat opsional; tidak ada data yang berubah pada stub ini.
  std::ofstream userFile("users.txt");
  if (userFile.is_open())
  {
    for (int i = 0; i < userCount; i++)
    {
      userFile << sanitizeField(userName[i]) << "|"
               << sanitizeField(userPassword[i]) << "|"
               << sanitizeField(userPhone[i]) << "|"
               << userFailed[i] << "|"
               << (userLocked[i] ? 1 : 0) << "\n";
    }
    userFile.close();
  }
}

// Done
// ============ auth ============

// FUNGSI: findUserByUsername   (flowchart: "Username" -> "Loop pengguna" -> "Indeks/-1")
// TUJUAN: mencari indeks pengguna tanpa membedakan huruf besar dan kecil.
// Penjelasan: Fungsi ini menormalkan username lalu membandingkannya dengan setiap pengguna.
// CONTOH: data Andi, input andi -> 0.
int findUserByUsername(std::string username)
{
  username = toLower(username);

  for (int i = 0; i < userCount; i++)
  {
    if (toLower(userName[i]) == username)
    {
      return i;
    }
  }

  return -1;
}

// FUNGSI: registerUser   (flowchart: "Input daftar" -> "Valid?" -> "Simpan array")
// TUJUAN: memvalidasi lalu menambahkan satu pengguna baru.
// Penjelasan: Fungsi ini memeriksa data pendaftaran, kapasitas, dan duplikasi sebelum mengisi array pengguna.
// CONTOH: andi, rahasia, 08123 -> REG_OK.
int registerUser(std::string username, std::string password,
                 std::string phone)
{
  if (username == "" || password == "" || phone == "")
    return REG_EMPTY_FIELD;

  int length = username.length();
  for (int i = 0; i < length; i++)
  {
    if (username[i] == ' ')
    {
      return REG_USERNAME_HAS_SPACE;
    }
  }

  if (findUserByUsername(username) != -1)
    return REG_DUPLICATE;

  if (password.length() < MIN_PASSWORD_LENGTH)
    return REG_PASSWORD_SHORT;

  if (isAllDigits(phone) == false)
    return REG_PHONE_INVALID;

  if (userCount >= MAX_USERS)
    return REG_FULL;

  userName[userCount] = username;
  userPassword[userCount] = password;
  userPhone[userCount] = phone;
  userFailed[userCount] = 0;
  userLocked[userCount] = false;

  userCount++;

  return REG_OK;
}

// FUNGSI: loginUser   (flowchart: "Input login" -> "Valid?" -> "Gagal 3x?")
// TUJUAN: memeriksa kredensial dan mengunci akun setelah tiga kesalahan.
// Penjelasan: Fungsi ini mengatur hitungan gagal, status kunci, dan mengembalikan indeks atau kode login.
// CONTOH: password salah tiga kali -> LOGIN_LOCKED.
int loginUser(std::string username, std::string password)
{
  int index = findUserByUsername(username);
  if (index == -1)
    return LOGIN_NOT_FOUND;

  if (userLocked[index] == true)
    return LOGIN_LOCKED;

  if (userPassword[index] == password)
  {
    userFailed[index] = 0;
    return index;
  }
  else
  {
    userFailed[index] += 1;
  }

  if (userFailed[index] >= MAX_LOGIN_ATTEMPTS)
  {
    userLocked[index] = true;
    return LOGIN_LOCKED;
  }
  else
  {
    return LOGIN_WRONG_PASSWORD;
  }

  return LOGIN_NOT_FOUND;
}

// ============ [PEMILIK Haris, Muflih, dan Mudhofar] items ============

// FUNGSI: addFoundItem   (flowchart: "Input barang" -> "Valid?" -> "Simpan")
// TUJUAN: memvalidasi dan menambah satu laporan barang temuan.
// Penjelasan: Fungsi ini memeriksa data dan tanggal, membersihkan teks, lalu mengisi array temuan.
// CONTOH: tanggal masa depan -> ITEM_FUTURE_DATE.
int addFoundItem(int reporterIndex, std::string name, std::string category,
                 std::string location, string dateFound,
                 std::string description, std::string secretDetail)
{
  // 1. Cek field wajib tidak boleh kosong (description opsional)
  if (trim(name) == "" || trim(category) == "" || trim(location) == "" ||
      trim(dateFound) == "" || trim(secretDetail) == "")
  {
    return ITEM_EMPTY_FIELD;
  }

  // 2. Cek apakah format tanggal valid
  if (!isValidDate(dateFound))
  {
    return ITEM_INVALID_DATE;
  }

  // 3. Cek apakah tanggal berada di masa depan
  if (isFutureDate(dateFound))
  {
    return ITEM_FUTURE_DATE;
  }

  // 4. Cek apakah kapasitas penyimpanan sudah penuh
  if (foundCount >= MAX_FOUND)
  {
    return ITEM_FULL;
  }

  // 5. Isi semua array found pada indeks foundCount, terapkan sanitizeField pada teks
  foundReporter[foundCount] = reporterIndex;
  foundName[foundCount] = sanitizeField(name);
  foundCategory[foundCount] = sanitizeField(category);
  foundLocation[foundCount] = sanitizeField(location);
  foundDate[foundCount] = dateFound;
  foundDescription[foundCount] = sanitizeField(description);
  foundSecret[foundCount] = sanitizeField(secretDetail);
  foundStatus[foundCount] = STATUS_AVAILABLE;

  // 6. Naikkan foundCount setelah semua pasangan terisi, lalu return ITEM_OK
  foundCount++;
  return ITEM_OK;
}

// FUNGSI: addLostReport   (flowchart: "Input hilang" -> "Valid?" -> "Simpan")
// TUJUAN: memvalidasi dan menambah satu laporan kehilangan.
// Penjelasan: Fungsi ini memeriksa data dan tanggal, membersihkan teks, lalu mengisi array kehilangan.
// CONTOH: lokasi kosong -> ITEM_EMPTY_FIELD.
int addLostReport(int reporterIndex, std::string name, std::string category,
                  std::string location, string dateLost,
                  std::string description)
{
  // 1. Cek field wajib tidak boleh kosong (description boleh kosong)
  if (trim(name) == "" || trim(category) == "" || trim(location) == "" || trim(dateLost) == "")
  {
    return ITEM_EMPTY_FIELD;
  }

  // 2. Jika tanggal tidak valid -> ITEM_INVALID_DATE; jika masa depan -> ITEM_FUTURE_DATE
  if (!isValidDate(dateLost))
  {
    return ITEM_INVALID_DATE;
  }

  if (isFutureDate(dateLost))
  {
    return ITEM_FUTURE_DATE;
  }

  // 3. Jika lostCount sudah MAX_LOST -> ITEM_FULL
  if (lostCount >= MAX_LOST)
  {
    return ITEM_FULL;
  }

  // 4. Isi seluruh array lost pada indeks lostCount dengan sanitizeField untuk semua teks
  lostReporter[lostCount] = reporterIndex;
  lostName[lostCount] = sanitizeField(name);
  lostCategory[lostCount] = sanitizeField(category);
  lostLocation[lostCount] = sanitizeField(location);
  lostDate[lostCount] = dateLost;
  lostDescription[lostCount] = sanitizeField(description);

  // 5. Naikkan lostCount lalu return ITEM_OK
  lostCount++;
  return ITEM_OK;
}

// FUNGSI: searchAvailableItems   (flowchart: "Filter" -> "Loop temuan" -> "searchResult")
// TUJUAN: mengisi hasil pencarian dengan barang berstatus tersedia.
// Penjelasan: Fungsi ini menerapkan filter nama, deskripsi, kategori, dan lokasi pada barang temuan.
// CONTOH: keyword tas menemukan Tas Biru.
int searchAvailableItems(std::string keyword, std::string category,
                         std::string location)
{

  // Reset jumlah hasil pencarian
  searchResultCount = 0;

  // Periksa seluruh data barang temuan
  for (int i = 0; i < foundCount; i++)
  {

    // lewati barang yang tidak tersedia
    if (foundStatus[i] != STATUS_AVAILABLE)
    {
      continue;
    }

    // Periksa keyword pada nama atau deskripsi barang
    if (keyword != "" &&
        !containsIgnoreCase(foundName[i], keyword) &&
        !containsIgnoreCase(foundDescription[i], keyword))
    {
      continue;
    }

    // Periksa kategori barang
    if (category != "" &&
        !containsIgnoreCase(foundCategory[i], category))
    {
      continue;
    }

    // Periksa lokasi barang
    if (location != "" &&
        !containsIgnoreCase(foundLocation[i], location))
    {
      continue;
    }

    // Simpan indeks asli barang ke hasil pencarian
    searchResult[searchResultCount] = i;
    searchResultCount++;
  }

  return searchResultCount;
}

// PEMILIK TODO: Muflih
// FUNGSI: listItemsByReporter   (flowchart: "User" -> "Loop temuan" ->
// "searchResult") TUJUAN: mengisi hasil bersama dengan semua barang temuan
// milik seorang pengguna.
// TODO:
//  [ ] 1. Set searchResultCount=0 dan loop semua found item.
//  [ ] 2. Jika foundReporter[i] sama dengan reporterIndex, simpan i ke
//  searchResult dan tambah count. [ ] 3. Return searchResultCount setelah loop.
// CONTOH: pengguna indeks 0 dengan dua temuan -> return 2.
// HATI-HATI: yang disimpan adalah indeks asli barang, bukan nomor tampilan
// indeks+1. SELESAI JIKA: [ ] M-ITEM-6 lulus. JANGAN ubah nama, parameter, atau
// tipe return fungsi ini tanpa lapor semua pemilik modul.
int listItemsByReporter(int reporterIndex)
{
  std::ofstream foundFile("found_items.txt");
  if (foundFile.is_open())
  {
    for (int i = 0; i < foundCount; i++)
    {
      foundFile << foundReporter[i] << "|"
                << sanitizeField(foundName[i]) << "|"
                << sanitizeField(foundCategory[i]) << "|"
                << sanitizeField(foundLocation[i]) << "|"
                << foundDate[i] << "|"
                << sanitizeField(foundDescription[i]) << "|"
                << sanitizeField(foundSecret[i]) << "|"
                << foundStatus[i] << "\n";
    }
    foundFile.close();
  }
  searchResultCount = 0;
  return 0;
}

// FUNGSI: getFailedClaimCount   (flowchart: "Item+user" -> "Loop klaim" -> "Jumlah")
// TUJUAN: mengambil jumlah jawaban klaim salah untuk pasangan item dan pengguna.
// Penjelasan: Fungsi ini mencari catatan klaim yang cocok dan mengembalikan 0 bila belum ada.
// CONTOH: belum pernah mencoba klaim -> 0.
int getFailedClaimCount(int itemIndex, int userIndex)
{
  for (int i = 0; i < claimCount; i++)
  {
    if (claimItem[i] == itemIndex &&
        claimUser[i] == userIndex)
    {
      return claimFailed[i];
    }
  }

  return 0;
}

// FUNGSI: getFinderContact   (flowchart: "Item" -> "Reporter" -> "Kontak")
// TUJUAN: membuat teks kontak penemu barang.
// Penjelasan: Fungsi ini mengambil reporter barang lalu menggabungkan username dan nomor teleponnya.
// CONTOH: penemu andi dengan 08123 -> andi - 08123.
std::string getFinderContact(int itemIndex)
{
  int reporterIndex = foundReporter[itemIndex];

  return userName[reporterIndex] + " - " + userPhone[reporterIndex];
}

// FUNGSI: submitClaim   (flowchart: "Pilih item" -> "Boleh klaim?" -> "Detail benar?")
// TUJUAN: menyetujui klaim benar atau mencatat jawaban salah.
// Penjelasan: Fungsi ini memeriksa kelayakan klaim, membandingkan detail rahasia, lalu memperbarui status atau percobaan.
// CONTOH: jawaban salah ketiga -> CLAIM_WRONG_DETAIL.
int submitClaim(int claimantIndex, int itemIndex, std::string secretAnswer)
{

  // Periksa apakah indeks barang falid
  if (itemIndex < 0 || itemIndex >= foundCount)
  {
    return CLAIM_ITEM_NOT_FOUND;
  }

  // Periksa apakah barang masih tersedia
  if (foundStatus[itemIndex] != STATUS_AVAILABLE)
  {
    return CLAIM_NOT_AVAILABLE;
  }

  // Pengguna tidak boleh mengklaim barang temuannya sendiri
  if (foundReporter[itemIndex] == claimantIndex)
  {
    return CLAIM_OWN_ITEM;
  }

  // Periksa batas percobaan klaim
  if (getFailedClaimCount(itemIndex, claimantIndex) >= MAX_CLAIM_ATTEMPTS)
  {
    return CLAIM_TOO_MANY_ATTEMPTS;
  }

  // Bandingkan jawaban dengan detail rahasia
  if (toLower(trim(secretAnswer)) ==
      toLower(trim(foundSecret[itemIndex])))
  {

    foundStatus[itemIndex] = STATUS_RETURNED;

    return CLAIM_APPROVED;
  }

  // Jika jawaban salah, cari data percobaan klaim sebelumnya
  for (int i = 0; i < claimCount; i++)
  {
    if (claimItem[i] == itemIndex &&
        claimUser[i] == claimantIndex)
    {

      claimFailed[i]++;
      return CLAIM_WRONG_DETAIL;
    }
  }

  // Jika belum ada data klaim dan penyimpana penuh
  if (claimCount >= MAX_CLAIMS)
  {
    return CLAIM_STORAGE_FULL;
  }

  // Buat data percobaan klaim baru
  claimItem[claimCount] = itemIndex;
  claimUser[claimCount] = claimantIndex;
  claimFailed[claimCount] = 1;

  claimCount++;

  return CLAIM_WRONG_DETAIL;
}

// ============ [PEMILIK Muflih] auth_screen ============

// FUNGSI: showRegisterScreen   (flowchart: "Input daftar" -> "registerUser" -> "Pesan")
// TUJUAN: meminta data pendaftaran, menampilkan pesan hasil, dan menyimpan jika sukses.
// TODO:
//  [ ] 1. Tampilkan header lalu ambil username, password, dan nomor dengan readLine.
//  [ ] 2. Panggil registerUser dan simpan kode hasilnya.
//  [ ] 3. Dengan if/else, tampilkan pesan Indonesia untuk setiap REG_*; jangan memakai switch.
//  [ ] 4. Jika kode REG_OK, panggil saveAll tepat sekali dan tampilkan pesan berhasil.
//  [ ] 5. Panggil pauseScreen sebelum kembali ke main.
// CONTOH: password lima karakter -> pesan password minimal 6 karakter dan tidak memanggil saveAll.
// HATI-HATI: layar menyimpan input dengan readLine; modul auth tidak boleh memakai cout/cin/file.
// SELESAI JIKA: [ ] M-AUTH-1 dan M-AUTH-2 lulus.
// JANGAN ubah nama, parameter, atau tipe return fungsi ini tanpa lapor semua pemilik modul.
void showRegisterScreen()
{
  printHeader("Register");

  std::string username = trim(readLine("Username: "));
  std::string password = readLine("Password (min 6 karakter): ");
  std::string phone = trim(readLine("Nomor telepon: "));

  int result = registerUser(username, password, phone);

  if (result == REG_OK)
  {
    std::cout << "Registrasi berhasil! Silakan login.\n";
    saveAll();
  }
  else if (result == REG_EMPTY_FIELD)
  {
    std::cout << "Gagal: Semua kolom harus diisi.\n";
  }
  else if (result == REG_USERNAME_HAS_SPACE)
  {
    std::cout << "Gagal: Username tidak boleh mengandung spasi.\n";
  }
  else if (result == REG_DUPLICATE)
  {
    std::cout << "Gagal: Username sudah digunakan.\n";
  }
  else if (result == REG_PASSWORD_SHORT)
  {
    std::cout << "Gagal: Password minimal " << MIN_PASSWORD_LENGTH
              << " karakter.\n";
  }
  else if (result == REG_PHONE_INVALID)
  {
    std::cout << "Gagal: Nomor telepon hanya boleh berisi angka.\n";
  }
  else if (result == REG_FULL)
  {
    std::cout << "Gagal: Kapasitas pengguna sudah penuh.\n";
  }
  else
  {
    std::cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
  // Stub layar: isi setelah registerUser selesai dikerjakan.
}

// PEMILIK TODO: Muflih
// FUNGSI: showLoginScreen   (flowchart: "Input login" -> "loginUser" -> "Masuk/-1")
// TUJUAN: meminta kredensial dan mengembalikan indeks pengguna bila login berhasil.
// TODO:
//  [ ] 1. Tampilkan header dan ambil username serta password melalui readLine.
//  [ ] 2. Panggil loginUser(username, password).
//  [ ] 3. Jika hasil >= 0, tampilkan selamat datang, panggil saveAll karena gagal login mungkin berubah, lalu return hasil.
//  [ ] 4. Jika hasil LOGIN_NOT_FOUND, LOGIN_WRONG_PASSWORD, atau LOGIN_LOCKED, tampilkan pesan Indonesia dengan if/else, panggil saveAll bila failed/locked berubah, pauseScreen, lalu return -1.
// CONTOH: username tidak ada -> pesan akun tidak ditemukan dan return -1.
// HATI-HATI: indeks 0 sukses; selalu cek hasil >= 0, bukan if(hasil).
// SELESAI JIKA: [ ] M-AUTH-3 dan M-AUTH-4 lulus.
// JANGAN ubah nama, parameter, atau tipe return fungsi ini tanpa lapor semua pemilik modul.
int showLoginScreen()
{
    printHeader("Login");

  std::string username = trim(readLine("Username: "));
  std::string password = readLine("Password: ");

  int result = loginUser(username, password);

  if (result >= 0)
  {
    std::cout << "Selamat datang, " << userName[result] << "!\n";
    saveAll(); // failed count direset ke 0
    return result;
  }

  if (result == LOGIN_NOT_FOUND)
  {
    std::cout << "Akun tidak ditemukan.\n";
  }
  else if (result == LOGIN_WRONG_PASSWORD)
  {
    std::cout << "Password salah.\n";
    saveAll(); // jumlah gagal berubah
  }
  else if (result == LOGIN_LOCKED)
  {
    std::cout << "Akun terkunci karena terlalu banyak percobaan gagal.\n";
    saveAll(); // status locked berubah
  }
  else
  {
    std::cout << "Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
  return -1;
  // Stub aman: -1 berarti kembali ke menu tamu.

}

// FUNGSI: showReportFoundScreen   (flowchart: "Input temuan" -> "addFoundItem" -> "Pesan")
// TUJUAN: meminta laporan barang temuan dan menyimpan jika berhasil.
// Penjelasan: Fungsi ini membaca data, mengirimnya ke addFoundItem, lalu menampilkan hasilnya.
// CONTOH: description kosong tetap dapat berhasil jika field wajib lain valid.
void showReportFoundScreen(int userIndex)
{
  printHeader("Lapor Barang Temuan");

  // 1. Ambil input dengan readLine
  std::string name = readLine("Nama barang: ");
  std::string category = readLine("Kategori: ");
  std::string location = readLine("Lokasi ditemukan: ");
  std::string date = readLine("Tanggal (YYYY-MM-DD): ");
  std::string description = readLine("Deskripsi (boleh kosong): ");
  std::string secretDetail = readLine("Detail rahasia (untuk verifikasi klaim): ");

  // 2. Panggil addFoundItem dan simpan kode hasilnya
  int result = addFoundItem(userIndex, name, category, location, date, description, secretDetail);

  // 3. Tampilkan pesan berdasarkan kode ITEM_* memakai if/else (tanpa mencetak secret detail)
  if (result == ITEM_OK)
  {
    std::cout << "Laporan barang temuan berhasil disimpan!\n";
    // 4. Jika ITEM_OK, panggil saveAll tepat sekali
    saveAll();
  }
  else if (result == ITEM_EMPTY_FIELD)
  {
    std::cout << "Gagal: Semua kolom wajib harus diisi (kecuali deskripsi).\n";
  }
  else if (result == ITEM_INVALID_DATE)
  {
    std::cout << "Gagal: Format tanggal tidak valid (gunakan YYYY-MM-DD).\n";
  }
  else if (result == ITEM_FUTURE_DATE)
  {
    std::cout << "Gagal: Tanggal tidak boleh berada di masa depan.\n";
  }
  else if (result == ITEM_FULL)
  {
    std::cout << "Gagal: Penyimpanan laporan temuan sudah penuh.\n";
  }
  else
  {
    std::cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  // 5. Panggil pauseScreen sebelum kembali ke menu
  pauseScreen();
}

// FUNGSI: showReportLostScreen   (flowchart: "Input hilang" -> "addLostReport" -> "Pesan")
// TUJUAN: meminta laporan kehilangan dan menyimpan jika berhasil.
// Penjelasan: Fungsi ini dapat memakai nama rekomendasi dari pencarian lalu menampilkan hasil penambahan laporan.
// CONTOH: pencarian dompet kosong -> nama awal dompet.
void showReportLostScreen(int userIndex, std::string prefillName)
{
  printHeader("Lapor Barang Hilang");

  std::string name = "";
  // 1. Jika prefillName tidak kosong, tampilkan dan tawarkan sebagai nama awal; tetap gunakan readLine untuk nama akhir
  if (prefillName != "")
  {
    std::cout << "Nama barang (rekomendasi dari pencarian: " << prefillName << ")\n";
    name = readLine("Nama barang akhir [Tekan Enter untuk pakai rekomendasi]: ");
    if (trim(name) == "")
    {
      name = prefillName;
    }
  }
  else
  {
    name = readLine("Nama barang: ");
  }

  // 2. Ambil category, location, date, dan description dengan readLine
  std::string category = readLine("Kategori: ");
  std::string location = readLine("Lokasi hilang: ");
  std::string date = readLine("Tanggal hilang (YYYY-MM-DD): ");
  std::string description = readLine("Deskripsi (boleh kosong): ");

  // 3. Panggil addLostReport lalu tampilkan pesan ITEM_* dengan if/else
  int result = addLostReport(userIndex, name, category, location, date, description);

  if (result == ITEM_OK)
  {
    std::cout << "Laporan barang hilang berhasil disimpan!\n";
    // 4. Jika ITEM_OK, panggil saveAll
    saveAll();
  }
  else if (result == ITEM_EMPTY_FIELD)
  {
    std::cout << "Gagal: Semua kolom wajib harus diisi (kecuali deskripsi).\n";
  }
  else if (result == ITEM_INVALID_DATE)
  {
    std::cout << "Gagal: Format tanggal tidak valid (gunakan YYYY-MM-DD).\n";
  }
  else if (result == ITEM_FUTURE_DATE)
  {
    std::cout << "Gagal: Tanggal tidak boleh berada di masa depan.\n";
  }
  else if (result == ITEM_FULL)
  {
    std::cout << "Gagal: Penyimpanan laporan hilang sudah penuh.\n";
  }
  else
  {
    std::cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  // Kemudian pauseScreen
  pauseScreen();
}

// PEMILIK TODO: Muflih
// FUNGSI: showMyReportsScreen   (flowchart: "User" -> "listItemsByReporter" ->
// "Daftar") TUJUAN: menampilkan semua barang temuan yang dilaporkan pengguna
// aktif.
// TODO:
//  [ ] 1. Panggil listItemsByReporter(userIndex).
//  [ ] 2. Jika hasil 0, tampilkan pesan belum ada laporan.
//  [ ] 3. Jika ada, loop searchResult dan tampilkan nomor toDisplayNo(index),
//  nama, lokasi, tanggal, dan status. [ ] 4. Jangan tampilkan foundSecret; lalu
//  pauseScreen.
// CONTOH: item indeks 0 tercetak sebagai nomor 1, bukan 0.
// HATI-HATI: searchResult menyimpan indeks asli; status boleh Dikembalikan
// tetap muncul di laporan saya. SELESAI JIKA: [ ] M-ITEM-6 lulus. JANGAN ubah
// nama, parameter, atau tipe return fungsi ini tanpa lapor semua pemilik modul.
void showMyReportsScreen(int userIndex)
{
  printHeader("Laporan Saya");

  int count = listItemsByReporter(userIndex);

  if (count == 0)
  {
    std::cout << "Kamu belum memiliki laporan barang temuan.\n";
    pauseScreen();
    return;
  }

  for (int i = 0; i < searchResultCount; i++)
  {
    int index = searchResult[i];

    std::cout << "\nNomor    : " << toDisplayNo(index) << "\n";
    std::cout << "Nama     : " << foundName[index] << "\n";
    std::cout << "Lokasi   : " << foundLocation[index] << "\n";
    std::cout << "Tanggal  : " << foundDate[index] << "\n";
    std::cout << "Status   : " << foundStatus[index] << "\n";
    // foundSecret sengaja tidak ditampilkan
  }

  pauseScreen();
}

// FUNGSI: showClaimFlowScreen   (flowchart: "Cari" -> "Pilih" -> "Jawab detail" -> "Klaim")
// TUJUAN: mengarahkan pengguna dari pencarian barang sampai hasil klaim.
// Penjelasan: Fungsi ini menampilkan hasil pencarian, memvalidasi pilihan, dan memproses jawaban detail rahasia.
// CONTOH: jawaban benar -> kontak penemu ditampilkan.
void showClaimFlowScreen(int userIndex)
{
  printHeader("Cari dan Klaim Barang");

  // Ambil filter pencarian dari pengguna
  std::string keyword = readLine("Nama atau deskripsi barang: ");
  std::string category = readLine("Kategori barang: ");
  std::string location = readLine("Lokasi ditemukan: ");

  // Cari barang yang masih tersedia
  int resultCount = searchAvailableItems(keyword, category, location);

  // Jika tidak ada barang yang ditemukan
  if (resultCount == 0) {
    std::cout << "\nBarang tidak ditemukan\n";
    int choice = readInt(
      "Apakah anda ingin membuat laporan barang hilang?\n"
      "1. Ya\n"
      "2. Tidak\n"
      "Pilih: ", 1, 2
    );

    if (choice == 1) {
      showReportLostScreen(userIndex, keyword);
    }

    return;
  }

  // Tampilkan daftar barang yang tersedia
  std::cout << "\n=== Daftar Barang Ditemukan ===\n";

  int maxDisplayNo = 0;

  for (int i = 0; i < searchResultCount; i++) {
    int index = searchResult[i];
    int displayNo = toDisplayNo(index);

    std::cout << "\nNomor    : " << displayNo << "\n";
    std::cout << "Nama       : " << foundName[index] << "\n";
    std::cout << "Kategori   : " << foundCategory[index] << "\n";
    std::cout << "Lokasi     : " << foundLocation[index] << "\n";
    std::cout << "Tanggal    : " << foundDate[index] << "\n";
    std::cout << "Deskripsi  : " << foundDescription[index] << "\n";

    if (displayNo > maxDisplayNo) {
      maxDisplayNo = displayNo;
    }
  }

  // Minta nomor barang yang ingin diklaim
  int displayNo = readInt(
    "\nMasukkan nomor barang yang ingin diklaim: ", 1,
    maxDisplayNo
  );

  int itemIndex = toIndex(displayNo);

  // Pastikan nomor yang dipilih benar-benar ada dalam hasil pencarian
  bool isFound = false;

  for (int i = 0; i < searchResultCount; i++){
    if (searchResult[i] == itemIndex) {
      isFound = true;
      break;
    }
  }

  if (!isFound) {
    std::cout << "\nNomor barang tidak tersedia dalam hasil pencarian.\n";
    pauseScreen();
    return;
  }

  // Ambil jawaban detail rahasia dan proses klaim
  std::string secretAnswer = readLine(
    "Masukkan detail khusus untuk membuktikan kepemilikan: "
  );

  int claimResult = submitClaim(userIndex, itemIndex, secretAnswer);

  // tampilkan pesan berdasarkan hasil klaim
  if (claimResult == CLAIM_APPROVED) {
    std::cout << "\nKlaim berhasil disetujui!\n";
    std::cout << "Silahkan hubungi penemu barang:\n";
    std::cout << getFinderContact(itemIndex) << "\n";

    // Simpan perubahan status barang
    saveAll();

  } else if (claimResult == CLAIM_WRONG_DETAIL) {
    std::cout << "\nDetail yang kamu masukkan salah.\n";
    std::cout << "Silahkan periksa kembali informasi barang.\n";

    //simpan penambahan jumlah percobaan salah
    saveAll();

  } else if (claimResult == CLAIM_TOO_MANY_ATTEMPTS) {
    std::cout << "\nKamu sudah mencapai batas percobaan klaim.\n";

  } else if (claimResult == CLAIM_ITEM_NOT_FOUND) {
    std::cout << "\nBarang tidak ditemukan.\n";

  } else if (claimResult == CLAIM_NOT_AVAILABLE) {
    std::cout << "\nBarang sudah tidak tersedia untuk diklaim.\n";

  } else if (claimResult == CLAIM_OWN_ITEM) {
    std::cout << "\nKamu tidak dapat mengklaim barang temuanmu sendiri.\n";

  } else if (claimResult == CLAIM_STORAGE_FULL) {
    std::cout << "\nPenyimpanan data percobaan klaim sudah penuh.\n";

  } else {
    std::cout << "\nTerjadi kesalahan yang tidak diketahui.\n";

  }

  // kembali setelah pengguna membaca hasil klaim
  pauseScreen();
}

// Ilham
// ============ main() ============

// FUNGSI: main   (flowchart: "Mulai" -> "loadAll" -> "Menu tamu/pengguna" -> "Keluar")
// TUJUAN: memulai data dan menjaga menu berjalan sampai pengguna keluar.
// Penjelasan: Fungsi ini memuat data, menampilkan menu tamu dan pengguna, lalu mengarahkan ke layar terkait.
// CONTOH: pilih 3 di menu tamu -> program berhenti normal.
int main()
{
  loadAll();
  if (userCount == 0)
  {
    loadDemoData();
  }

  bool isRunning = true;
  while (isRunning)
  {
    printHeader("Lost and Found Management System");
    int guestChoice = readInt("1. Register\n2. Login\n3. Keluar\nPilih: ", 1, 3);

    if (guestChoice == 1)
    {
      showRegisterScreen();
    }
    else if (guestChoice == 2)
    {
      int userIndex = showLoginScreen();
      while (userIndex >= 0)
      {
        printHeader("Menu Pengguna");
        int userChoice = readInt("1. Lapor temuan\n2. Klaim barang\n3. Laporan saya\n4. Logout\nPilih: ", 1, 4);
        if (userChoice == 1)
        {
          showReportFoundScreen(userIndex);
        }
        else if (userChoice == 2)
        {
          showClaimFlowScreen(userIndex);
        }
        else if (userChoice == 3)
        {
          showMyReportsScreen(userIndex);
        }
        else
        {
          userIndex = -1;
        }
      }
    }
    else
    {
      isRunning = false;
    }
  }

  return 0;
}
