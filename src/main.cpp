#include <cctype>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mysql/mysql.h>
#include <string>

using namespace std;

const int MAX_USERS = 100;
const int MAX_FOUND = 200;
const int MAX_LOST = 200;
const int MAX_CLAIMS = 400;
const int MAX_LOCKERS = 9;
const int MAX_LOGIN_ATTEMPTS = 3;
const int MAX_CLAIM_ATTEMPTS = 3;
const int MIN_PASSWORD_LENGTH = 6;
const string UI_BORDER(54, '=');
const string UI_DIVIDER(54, '-');
const int CATEGORY_COUNT = 6;
const string CATEGORIES[CATEGORY_COUNT] = {
    "Aksesori", "Dokumen", "Dompet", "Elektronik", "Kunci", "Pakaian"};
const int MAJOR_COUNT = 6;
const string MAJORS[MAJOR_COUNT] = {
    "Informatika", "Sistem Informasi", "Teknik Komputer",
    "Teknik Elektro", "Manajemen", "Akuntansi"};

const string STATUS_AVAILABLE = "Tersedia";
const string STATUS_RETURNED = "Dikembalikan";
const string STATUS_DEPOSIT_PENDING = "Menunggu Dititipkan";
const string STATUS_DEPOSIT_CANCELLED = "Reservasi Batal";
const string STATUS_NEEDS_VERIFICATION = "Perlu Verifikasi";
const string STATUS_CLAIM_PENDING = "Dalam Klaim";
const string STATUS_PICKUP_LOCKED = "Pengambilan Terkunci";
const string STATUS_PICKED_UP = "Sudah Diambil";
const string STATUS_CAMPUS_HANDOFF = "Diserahkan ke Kampus";
const string ROLE_STUDENT = "Mahasiswa";
const string ROLE_ADMIN = "Admin";

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
const int ITEM_LOCKER_FULL = 5;

const int CLAIM_APPROVED = 0;
const int CLAIM_WRONG_DETAIL = 1;
const int CLAIM_TOO_MANY_ATTEMPTS = 2;
const int CLAIM_ITEM_NOT_FOUND = 3;
const int CLAIM_NOT_AVAILABLE = 4;
const int CLAIM_OWN_ITEM = 5;
const int CLAIM_STORAGE_FULL = 6;

const int PICKUP_OK = 0;
const int PICKUP_WRONG_CODE = 1;
const int PICKUP_EXPIRED = 2;
const int PICKUP_LOCKED = 3;

string userName[MAX_USERS], userPassword[MAX_USERS], userPhone[MAX_USERS],
    userFullName[MAX_USERS], userMajor[MAX_USERS], userEmail[MAX_USERS],
    userRole[MAX_USERS];
int userFailed[MAX_USERS];
bool userLocked[MAX_USERS];
int userCount = 0;

int foundReporter[MAX_FOUND];
string foundName[MAX_FOUND], foundCategory[MAX_FOUND],
    foundLocation[MAX_FOUND], foundDate[MAX_FOUND], foundDescription[MAX_FOUND],
    foundSecret[MAX_FOUND], foundStatus[MAX_FOUND], foundPhoto[MAX_FOUND],
    foundDepositCode[MAX_FOUND], foundPickupCode[MAX_FOUND];
int foundLocker[MAX_FOUND], foundClaimant[MAX_FOUND],
    foundDepositFailed[MAX_FOUND], foundPickupFailed[MAX_FOUND];
long foundReservationExpiry[MAX_FOUND], foundPickupExpiry[MAX_FOUND];
bool lockerBroken[MAX_LOCKERS];
int foundCount = 0;

int lostReporter[MAX_LOST];
string lostName[MAX_LOST], lostCategory[MAX_LOST], lostLocation[MAX_LOST],
    lostDate[MAX_LOST], lostDescription[MAX_LOST];
int lostCount = 0;

int claimItem[MAX_CLAIMS], claimUser[MAX_CLAIMS], claimFailed[MAX_CLAIMS];
int claimCount = 0;

int searchResult[MAX_FOUND];
int searchResultCount = 0;

MYSQL *databaseConnection = 0;
bool databaseUserStored[MAX_USERS];
int databaseItemId[MAX_FOUND], databaseDepositId[MAX_FOUND],
    databaseDepositLockerId[MAX_FOUND], databaseActiveClaimId[MAX_FOUND],
    databaseDepositCodeId[MAX_FOUND], databasePickupCodeId[MAX_FOUND];
int databaseFailedClaimId[MAX_CLAIMS], databaseLostReportId[MAX_LOST],
    databaseLockerId[MAX_LOCKERS];

// ============ demo data ============
string getTodayDate();

// Seeds two student accounts and two ready-to-claim items when storage is empty.
void loadDemoData() {
  if (userCount != 0 || foundCount != 0 || lostCount != 0) {
    return;
  }

  userName[0] = "Azhri";
  userPassword[0] = "Azhri123";
  userPhone[0] = "081234567890";
  userFullName[0] = "Azhri Hytam";
  userMajor[0] = "Informatika";
  userEmail[0] = "azhri@example.com";
  userFailed[0] = 0;
  userLocked[0] = false;
  userRole[0] = ROLE_STUDENT;

  userName[1] = "Arya";
  userPassword[1] = "Arya123";
  userPhone[1] = "089876543210";
  userFullName[1] = "Arya Pratama";
  userMajor[1] = "Sistem Informasi";
  userEmail[1] = "arya@example.com";
  userFailed[1] = 0;
  userLocked[1] = false;
  userRole[1] = ROLE_STUDENT;

  const string demoDate = getTodayDate();

  foundReporter[0] = 0;
  foundName[0] = "Payung Hitam";
  foundCategory[0] = "Aksesori";
  foundLocation[0] = "Kantin Utama";
  foundDate[0] = demoDate;
  foundDescription[0] = "Payung lipat warna hitam merk standar";
  foundSecret[0] = "Ada stiker kecil bergambar bintang di pegangannya";
  foundStatus[0] = STATUS_AVAILABLE;
  foundPhoto[0] = "demo/payung-hitam.jpg";
  foundLocker[0] = 0;
  foundDepositCode[0] = "";
  foundReservationExpiry[0] = 0;
  foundDepositFailed[0] = 0;
  foundClaimant[0] = -1;
  foundPickupCode[0] = "";
  foundPickupExpiry[0] = 0;
  foundPickupFailed[0] = 0;

  foundReporter[1] = 1;
  foundName[1] = "Dompet Kulit Coklat";
  foundCategory[1] = "Dompet";
  foundLocation[1] = "Parkiran Fakultas";
  foundDate[1] = demoDate;
  foundDescription[1] =
      "Dompet berisi kartu identitas dan beberapa kartu penting";
  foundSecret[1] = "Ada foto pelajar di dalam selipan kartu";
  foundStatus[1] = STATUS_AVAILABLE;
  foundPhoto[1] = "demo/dompet-coklat.jpg";
  foundLocker[1] = 1;
  foundDepositCode[1] = "";
  foundReservationExpiry[1] = 0;
  foundDepositFailed[1] = 0;
  foundClaimant[1] = -1;
  foundPickupCode[1] = "";
  foundPickupExpiry[1] = 0;
  foundPickupFailed[1] = 0;

  userCount = 2;
  foundCount = 2;
}

// Creates the demo admin account when no admin exists.
void ensureDemoAdmin() {
  for (int i = 0; i < userCount; i++) {
    if (userRole[i] == ROLE_ADMIN) {
      return;
    }
  }

  if (userCount >= MAX_USERS) {
    return;
  }

  userName[userCount] = "admin";
  userPassword[userCount] = "admin123";
  userPhone[userCount] = "0000000000";
  userFullName[userCount] = "Administrator";
  userMajor[userCount] = "";
  userEmail[userCount] = "";
  userFailed[userCount] = 0;
  userLocked[userCount] = false;
  userRole[userCount] = ROLE_ADMIN;
  userCount++;
}

// Removes leading and trailing spaces from text.
string trim(string s) {
  int left = 0;
  int right = s.length();

  while (left < right && s[left] == ' ') {
    left++;
  }

  while (right > left && s[right - 1] == ' ') {
    right--;
  }

  return s.substr(left, right - left);
}

// Converts text to lowercase for case-insensitive comparisons.
string toLower(string s) {
  int length = s.length();
  for (int i = 0; i < length; i++) {
    s[i] = tolower(s[i]);
  }

  return s;
}

// Checks whether text contains a keyword without case sensitivity.
bool containsIgnoreCase(string text, string keyword) {
  text = toLower(text);
  keyword = toLower(keyword);

  if (keyword == "") {
    return true;
  }

  int keywordLength = keyword.length();
  int textLength = text.length();
  for (int i = 0; i + keywordLength <= textLength; i++) {
    bool match = true;

    for (int j = 0; j < keywordLength; j++) {
      if (text[i + j] != keyword[j]) {
        match = false;
        break;
      }
    }

    if (match) {
      return true;
    }
  }
  return false;
}

// Replaces file-delimiter and newline characters in stored text.
string sanitizeField(string s) {
  int length = s.length();
  for (int i = 0; i < length; i++) {
    if (s[i] == '|' || s[i] == '\n' || s[i] == '\r') {
      s[i] = '/';
    }
  }

  return s;
}

// Checks whether text is a nonempty sequence of digits.
bool isAllDigits(string s) {
  if (s == "") {
    return false;
  }

  int length = s.length();
  for (int i = 0; i < length; i++) {
    bool isDigit = isdigit(s[i]);

    if (!isDigit) {
      return false;
    }
  }

  return true;
}

// Checks whether a year is a leap year.
bool isLeapYear(int year) {
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

// Returns the number of days in a month.
int daysInMonth(int month, int year) {
  switch (month) {
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
    if (isLeapYear(year)) {
      return 29;
    }
    return 28;

  default:
    return 0;
  }
}

// Validates a YYYY-MM-DD date.
bool isValidDate(string date) {
  if (date.length() != 10)
    return false;

  if (date[4] != '-' || date[7] != '-')
    return false;

  for (int i = 0; i < 10; i++) {
    if (i == 4 || i == 7) {
      continue;
    }

    if (date[i] < '0' || date[i] > '9') {
      return false;
    }
  }

  int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
             (date[2] - '0') * 10 + (date[3] - '0');
  int month = (date[5] - '0') * 10 + (date[6] - '0');
  int day = (date[8] - '0') * 10 + (date[9] - '0');

  if (year < 1)
    return false;

  if (month < 1 || month > 12)
    return false;

  if (day < 1 || day > daysInMonth(month, year)) {
    return false;
  }

  return true;
}

// Returns the current local date as YYYY-MM-DD.
string getTodayDate() {
  time_t now = time(0);
  tm *local = localtime(&now);
  char result[11];
  strftime(result, sizeof(result), "%Y-%m-%d", local);
  return string(result);
}

// Checks whether a valid date is after today.
bool isFutureDate(string date) {
  if (isValidDate(date) == false)
    return false;

  string today = getTodayDate();

  return date > today;
}

// Checks whether an item date is at least 30 days old.
bool isUnclaimedFor30Days(string date) {
  if (!isValidDate(date)) {
    return false;
  }

  tm foundTime = {};
  foundTime.tm_year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
                      (date[2] - '0') * 10 + (date[3] - '0') - 1900;
  foundTime.tm_mon = (date[5] - '0') * 10 + (date[6] - '0') - 1;
  foundTime.tm_mday = (date[8] - '0') * 10 + (date[9] - '0');
  foundTime.tm_hour = 12;

  time_t foundAt = mktime(&foundTime);
  if (foundAt == (time_t)-1) {
    return false;
  }

  return difftime(time(0), foundAt) >= 30 * 24 * 60 * 60;
}

// Converts a zero-based index to a user-facing number.
int toDisplayNo(int index) { return index + 1; }

// Converts a user-facing number to a zero-based index.
int toIndex(int displayNo) { return displayNo - 1; }

// Prompts for and reads one full line of input.
string readLine(string prompt) {
  cout << prompt;
  string result;
  getline(cin, result);
  return result;
}

// Reads a bounded integer and retries invalid input.
int readInt(string prompt, int minValue, int maxValue) {
  while (true) {
    string text = readLine(prompt);

    if (!isAllDigits(text)) {
      cout << "Masukkan angka yang benar.\n";
      continue;
    }

    else {
      int result = 0;
      bool overflow = false;

      for (int i = 0; i < (int)text.length(); i++) {
        int digit = text[i] - '0';

        if (result > (INT_MAX - digit) / 10) {
          overflow = true;
          break;
        }

        result = result * 10 + digit;
      }

      if (overflow) {
        cout << "Angka terlalu besar. Silahkan masukkan angka kembali.\n";
        continue;
      }

      if (result >= minValue && result <= maxValue) {
        return result;
      }
      cout << "Masukkan angka dari " << minValue << " sampai " << maxValue
           << ".\n";
    }
  }
}

// Waits for Enter before returning to a menu.
void pauseScreen() {
  cout << "\n" << UI_DIVIDER << "\n[ Tekan Enter untuk lanjut ]";
  readLine("");
  cout << "\n";
}

// Prints a consistent terminal screen heading.
void printHeader(string title) {
  cout << "\n" << UI_BORDER << "\n";
  cout << "                 LOST & FOUND LOCKER\n";
  cout << UI_BORDER << "\n";
  cout << "  " << title << "\n" << UI_DIVIDER << "\n";
}

// Lets the user select one canonical item category.
string selectCategory(bool allowAll) {
  cout << "\nKATEGORI\n" << UI_DIVIDER << "\n";
  if (allowAll) {
    cout << "  0. Semua kategori\n";
  }
  for (int i = 0; i < CATEGORY_COUNT; i++) {
    cout << "  " << i + 1 << ". " << CATEGORIES[i] << "\n";
  }
  cout << UI_DIVIDER << "\n";

  int choice = readInt("Pilih kategori [" +
                           to_string(allowAll ? 0 : 1) + "-" +
                           to_string(CATEGORY_COUNT) + "]: ",
                       allowAll ? 0 : 1, CATEGORY_COUNT);
  if (choice == 0) {
    return "";
  }
  return CATEGORIES[choice - 1];
}

// Lets the user select one canonical study major.
string selectMajor() {
  cout << "\nJURUSAN\n" << UI_DIVIDER << "\n";
  for (int i = 0; i < MAJOR_COUNT; i++) {
    cout << "  " << i + 1 << ". " << MAJORS[i] << "\n";
  }
  cout << UI_DIVIDER << "\n";
  int choice = readInt("Pilih jurusan [1-" + to_string(MAJOR_COUNT) +
                           "]: ",
                       1, MAJOR_COUNT);
  return MAJORS[choice - 1];
}

// ============ database persistence (MariaDB/MySQL) ============
void databaseDisconnect() {
  if (databaseConnection != 0) {
    mysql_close(databaseConnection);
    databaseConnection = 0;
  }
}

bool databaseConnect() {
  const char *host = getenv("DB_HOST");
  const char *user = getenv("DB_USER");
  const char *password = getenv("DB_PASSWORD");
  const char *schema = getenv("DB_NAME");
  const char *portText = getenv("DB_PORT");

  if (host == 0 || user == 0 || password == 0 || schema == 0) {
    cerr << "Database belum dikonfigurasi. Atur DB_HOST, DB_USER, "
            "DB_PASSWORD, dan DB_NAME.\n";
    return false;
  }

  unsigned int port = 3306;
  if (portText != 0 && portText[0] != '\0') {
    for (int i = 0; portText[i] != '\0'; i++) {
      if (portText[i] < '0' || portText[i] > '9') {
        cerr << "DB_PORT harus berupa angka.\n";
        return false;
      }
    }
    port = static_cast<unsigned int>(strtoul(portText, 0, 10));
  }

  databaseConnection = mysql_init(0);
  if (databaseConnection == 0) {
    cerr << "Gagal menyiapkan koneksi database.\n";
    return false;
  }

  const char *charset = "utf8mb4";
  mysql_options(databaseConnection, MYSQL_SET_CHARSET_NAME, charset);
  if (mysql_real_connect(databaseConnection, host, user, password, schema,
                         port, 0, 0) == 0) {
    cerr << "Gagal terhubung ke database: " << mysql_error(databaseConnection)
         << "\n";
    databaseDisconnect();
    return false;
  }

  return true;
}

bool databaseExecute(string query) {
  if (mysql_query(databaseConnection, query.c_str()) != 0) {
    cerr << "Database error: " << mysql_error(databaseConnection) << "\n";
    return false;
  }
  return true;
}

MYSQL_RES *databaseSelect(string query) {
  if (!databaseExecute(query)) {
    return 0;
  }

  MYSQL_RES *result = mysql_store_result(databaseConnection);
  if (result == 0) {
    cerr << "Database tidak mengembalikan hasil query: "
         << mysql_error(databaseConnection) << "\n";
  }
  return result;
}

string databaseQuote(string value) {
  string escaped(value.length() * 2 + 1, '\0');
  unsigned long length = mysql_real_escape_string(
      databaseConnection, &escaped[0], value.c_str(), value.length());
  return "'" + escaped.substr(0, length) + "'";
}

long databaseNumber(const char *value) {
  return value == 0 ? 0 : strtol(value, 0, 10);
}

string databaseField(MYSQL_ROW row, int column) {
  return row[column] == 0 ? "" : row[column];
}

int databaseUserIndex(string nim) {
  for (int i = 0; i < userCount; i++) {
    if (userName[i] == nim) {
      return i;
    }
  }
  return -1;
}

int databaseItemIndex(int itemId) {
  for (int i = 0; i < foundCount; i++) {
    if (databaseItemId[i] == itemId) {
      return i;
    }
  }
  return -1;
}

int databaseItemIndexByDeposit(int depositId) {
  for (int i = 0; i < foundCount; i++) {
    if (databaseDepositId[i] == depositId) {
      return i;
    }
  }
  return -1;
}

int databaseItemIndexByClaim(int claimId) {
  for (int i = 0; i < foundCount; i++) {
    if (databaseActiveClaimId[i] == claimId) {
      return i;
    }
  }
  return -1;
}

int databaseSingleId(string query) {
  MYSQL_RES *result = databaseSelect(query);
  if (result == 0) {
    return 0;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  int id = row == 0 ? 0 : static_cast<int>(databaseNumber(row[0]));
  mysql_free_result(result);
  return id;
}

int databaseCategoryId(string category) {
  int id = databaseSingleId("SELECT id_kategori FROM categories WHERE "
                            "nama_kategori = " + databaseQuote(category));
  if (id != 0) {
    return id;
  }

  if (!databaseExecute("INSERT INTO categories (nama_kategori) VALUES (" +
                       databaseQuote(category) + ")")) {
    return 0;
  }
  return static_cast<int>(mysql_insert_id(databaseConnection));
}

int databaseLocationId(string location) {
  int id = databaseSingleId(
      "SELECT id_lokasi FROM locations WHERE nama_gedung = " +
      databaseQuote(location) + " AND detail = '' LIMIT 1");
  if (id != 0) {
    return id;
  }

  if (!databaseExecute("INSERT INTO locations (nama_gedung, detail) VALUES (" +
                       databaseQuote(location) + ", '')")) {
    return 0;
  }
  return static_cast<int>(mysql_insert_id(databaseConnection));
}

int databaseLockerIdForIndex(int lockerIndex) {
  if (lockerIndex < 0 || lockerIndex >= MAX_LOCKERS) {
    return 0;
  }
  if (databaseLockerId[lockerIndex] != 0) {
    return databaseLockerId[lockerIndex];
  }

  databaseLockerId[lockerIndex] = databaseSingleId(
      "SELECT id_locker FROM lockers WHERE nomor_loker = " +
      to_string(lockerIndex + 1));
  return databaseLockerId[lockerIndex];
}

bool databaseLogLockerEvent(int lockerIndex, int depositId, string eventType,
                            string detail) {
  int lockerId = databaseLockerIdForIndex(lockerIndex);
  if (lockerId == 0) {
    return false;
  }

  string deposit = depositId == 0 ? "NULL" : to_string(depositId);
  return databaseExecute(
      "INSERT INTO locker_event_logs (id_locker, id_deposit, tipe_event, "
      "keterangan) VALUES (" +
      to_string(lockerId) + ", " + deposit + ", " +
      databaseQuote(eventType) + ", " + databaseQuote(detail) + ")");
}

bool lockerIsOccupied(int lockerIndex) {
  for (int item = 0; item < foundCount; item++) {
    if (foundLocker[item] == lockerIndex &&
        foundStatus[item] != STATUS_DEPOSIT_CANCELLED &&
        foundStatus[item] != STATUS_PICKED_UP) {
      return true;
    }
  }
  return false;
}

bool databasePasswordMatches(string nim, string password) {
  return databaseSingleId("SELECT 1 FROM users WHERE nim = " +
                          databaseQuote(nim) +
                          " AND password_hash = SHA2(" +
                          databaseQuote(password) + ", 256)") == 1;
}

bool databaseAccessCodeMatches(int codeId, string code) {
  if (codeId == 0) {
    return false;
  }
  return databaseSingleId("SELECT 1 FROM access_codes WHERE id_code = " +
                          to_string(codeId) + " AND status = 'ACTIVE' AND "
                          "code_hash = SHA2(" + databaseQuote(code) +
                          ", 256)") == 1;
}

bool databaseLoadUsers() {
  MYSQL_RES *result = databaseSelect(
      "SELECT nim, no_hp, nama, jurusan, email, role, failed_login, "
      "locked_until IS NOT NULL FROM users ORDER BY nim");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0 && userCount < MAX_USERS) {
    userName[userCount] = databaseField(row, 0);
    userPassword[userCount] = "";
    userPhone[userCount] = databaseField(row, 1);
    userFullName[userCount] = databaseField(row, 2);
    userMajor[userCount] = databaseField(row, 3);
    userEmail[userCount] = databaseField(row, 4);
    userRole[userCount] = databaseField(row, 5);
    userFailed[userCount] = static_cast<int>(databaseNumber(row[6]));
    userLocked[userCount] = databaseNumber(row[7]) != 0;
    databaseUserStored[userCount] = true;
    userCount++;
  }
  mysql_free_result(result);
  return true;
}

bool databaseLoadLockers() {
  MYSQL_RES *result = databaseSelect(
      "SELECT id_locker, nomor_loker, status FROM lockers");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0) {
    int index = static_cast<int>(databaseNumber(row[1])) - 1;
    if (index >= 0 && index < MAX_LOCKERS) {
      databaseLockerId[index] = static_cast<int>(databaseNumber(row[0]));
      lockerBroken[index] = databaseField(row, 2) == "BROKEN";
    }
  }
  mysql_free_result(result);
  return true;
}

bool databaseLoadItems() {
  MYSQL_RES *result = databaseSelect(
      "SELECT i.id_item, i.nim_penemu, c.nama_kategori, l.nama_gedung, "
      "l.detail, i.nama_barang, i.ciri_ciri, i.tanggal_temuan, "
      "i.data_verifikasi, i.status, COALESCE((SELECT p.path_foto FROM "
      "item_photos p WHERE p.id_item = i.id_item ORDER BY p.urutan LIMIT 1), "
      "'') FROM items i JOIN categories c ON c.id_kategori = i.id_kategori "
      "JOIN locations l ON l.id_lokasi = i.id_lokasi ORDER BY i.id_item");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0 && foundCount < MAX_FOUND) {
    int reporter = databaseUserIndex(databaseField(row, 1));
    if (reporter < 0) {
      continue;
    }

    databaseItemId[foundCount] = static_cast<int>(databaseNumber(row[0]));
    foundReporter[foundCount] = reporter;
    foundCategory[foundCount] = databaseField(row, 2);
    foundLocation[foundCount] = databaseField(row, 3);
    if (databaseField(row, 4) != "") {
      foundLocation[foundCount] += " - " + databaseField(row, 4);
    }
    foundName[foundCount] = databaseField(row, 5);
    foundDescription[foundCount] = databaseField(row, 6);
    foundDate[foundCount] = databaseField(row, 7);
    foundSecret[foundCount] = databaseField(row, 8);
    foundStatus[foundCount] = databaseField(row, 9);
    foundPhoto[foundCount] = databaseField(row, 10);
    foundLocker[foundCount] = -1;
    foundClaimant[foundCount] = -1;
    foundCount++;
  }
  mysql_free_result(result);
  return true;
}

bool databaseLoadLostReports() {
  MYSQL_RES *result = databaseSelect(
      "SELECT r.id_laporan, r.nim_pelapor, c.nama_kategori, l.nama_gedung, "
      "l.detail, r.nama_barang, r.ciri_ciri, r.tanggal_hilang FROM "
      "lost_reports r JOIN categories c ON c.id_kategori = r.id_kategori "
      "JOIN locations l ON l.id_lokasi = r.id_lokasi ORDER BY r.id_laporan");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0 && lostCount < MAX_LOST) {
    int reporter = databaseUserIndex(databaseField(row, 1));
    if (reporter < 0) {
      continue;
    }
    databaseLostReportId[lostCount] =
        static_cast<int>(databaseNumber(row[0]));
    lostReporter[lostCount] = reporter;
    lostCategory[lostCount] = databaseField(row, 2);
    lostLocation[lostCount] = databaseField(row, 3);
    if (databaseField(row, 4) != "") {
      lostLocation[lostCount] += " - " + databaseField(row, 4);
    }
    lostName[lostCount] = databaseField(row, 5);
    lostDescription[lostCount] = databaseField(row, 6);
    lostDate[lostCount] = databaseField(row, 7);
    lostCount++;
  }
  mysql_free_result(result);
  return true;
}

bool databaseLoadDeposits() {
  MYSQL_RES *result = databaseSelect(
      "SELECT d.id_deposit, d.id_item, d.id_locker, l.nomor_loker, d.status, "
      "UNIX_TIMESTAMP(d.reserve_expires_at) FROM deposits d JOIN (SELECT "
      "id_item, MAX(id_deposit) AS id_deposit FROM deposits GROUP BY id_item) "
      "latest ON latest.id_deposit = d.id_deposit JOIN lockers l ON "
      "l.id_locker = d.id_locker");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0) {
    int itemIndex = databaseItemIndex(static_cast<int>(databaseNumber(row[1])));
    if (itemIndex < 0) {
      continue;
    }
    databaseDepositId[itemIndex] = static_cast<int>(databaseNumber(row[0]));
    databaseDepositLockerId[itemIndex] =
        static_cast<int>(databaseNumber(row[2]));
    string status = databaseField(row, 4);
    if (status == "RESERVED" || status == "DEPOSITED" || status == "VERIFY") {
      foundLocker[itemIndex] = static_cast<int>(databaseNumber(row[3])) - 1;
    }
    if (status == "RESERVED") {
      foundReservationExpiry[itemIndex] = databaseNumber(row[5]);
    }
  }
  mysql_free_result(result);
  return true;
}

bool databaseLoadClaims() {
  MYSQL_RES *active = databaseSelect(
      "SELECT id_claim, id_item, nim_pengklaim, status FROM claims WHERE "
      "status IN ('PENDING', 'APPROVED', 'LOCKED') ORDER BY id_claim");
  if (active == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(active)) != 0) {
    int itemIndex = databaseItemIndex(static_cast<int>(databaseNumber(row[1])));
    int claimant = databaseUserIndex(databaseField(row, 2));
    if (itemIndex >= 0 && claimant >= 0) {
      databaseActiveClaimId[itemIndex] = static_cast<int>(databaseNumber(row[0]));
      foundClaimant[itemIndex] = claimant;
    }
  }
  mysql_free_result(active);

  MYSQL_RES *failed = databaseSelect(
      "SELECT id_claim, id_item, nim_pengklaim, failed_count FROM claims "
      "WHERE status = 'REJECTED' AND failed_count > 0 ORDER BY id_claim");
  if (failed == 0) {
    return false;
  }

  while ((row = mysql_fetch_row(failed)) != 0 && claimCount < MAX_CLAIMS) {
    int itemIndex = databaseItemIndex(static_cast<int>(databaseNumber(row[1])));
    int claimant = databaseUserIndex(databaseField(row, 2));
    if (itemIndex >= 0 && claimant >= 0) {
      databaseFailedClaimId[claimCount] =
          static_cast<int>(databaseNumber(row[0]));
      claimItem[claimCount] = itemIndex;
      claimUser[claimCount] = claimant;
      claimFailed[claimCount] = static_cast<int>(databaseNumber(row[3]));
      claimCount++;
    }
  }
  mysql_free_result(failed);
  return true;
}

bool databaseLoadAccessCodes() {
  MYSQL_RES *result = databaseSelect(
      "SELECT id_code, id_deposit, id_claim, tipe, UNIX_TIMESTAMP(expires_at), "
      "failed_count FROM access_codes WHERE status = 'ACTIVE'");
  if (result == 0) {
    return false;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result)) != 0) {
    string type = databaseField(row, 3);
    if (type == "DEPOSIT") {
      int itemIndex = databaseItemIndexByDeposit(
          static_cast<int>(databaseNumber(row[1])));
      if (itemIndex >= 0) {
        databaseDepositCodeId[itemIndex] =
            static_cast<int>(databaseNumber(row[0]));
        foundReservationExpiry[itemIndex] = databaseNumber(row[4]);
        foundDepositFailed[itemIndex] =
            static_cast<int>(databaseNumber(row[5]));
      }
    } else if (type == "PICKUP") {
      int itemIndex = databaseItemIndexByClaim(
          static_cast<int>(databaseNumber(row[2])));
      if (itemIndex >= 0) {
        databasePickupCodeId[itemIndex] =
            static_cast<int>(databaseNumber(row[0]));
        foundPickupExpiry[itemIndex] = databaseNumber(row[4]);
        foundPickupFailed[itemIndex] = static_cast<int>(databaseNumber(row[5]));
      }
    }
  }
  mysql_free_result(result);
  return true;
}

// Loads the application state from the migrated relational schema.
void loadAll() {
  userCount = 0;
  foundCount = 0;
  lostCount = 0;
  claimCount = 0;
  for (int i = 0; i < MAX_USERS; i++) {
    databaseUserStored[i] = false;
  }
  for (int i = 0; i < MAX_FOUND; i++) {
    databaseItemId[i] = 0;
    databaseDepositId[i] = 0;
    databaseDepositLockerId[i] = 0;
    databaseActiveClaimId[i] = 0;
    databaseDepositCodeId[i] = 0;
    databasePickupCodeId[i] = 0;
  }
  for (int i = 0; i < MAX_LOST; i++) {
    databaseLostReportId[i] = 0;
  }
  for (int i = 0; i < MAX_CLAIMS; i++) {
    databaseFailedClaimId[i] = 0;
  }
  for (int i = 0; i < MAX_LOCKERS; i++) {
    databaseLockerId[i] = 0;
    lockerBroken[i] = false;
  }

  if (!databaseLoadUsers() || !databaseLoadLockers() || !databaseLoadItems() ||
      !databaseLoadLostReports() || !databaseLoadDeposits() ||
      !databaseLoadClaims() || !databaseLoadAccessCodes()) {
    cerr << "Gagal memuat data dari database.\n";
    exit(EXIT_FAILURE);
  }
}

string databaseDepositStatus(int itemIndex) {
  if (foundStatus[itemIndex] == STATUS_DEPOSIT_PENDING) {
    return "RESERVED";
  }
  if (foundStatus[itemIndex] == STATUS_DEPOSIT_CANCELLED) {
    return "CANCELLED";
  }
  if (foundStatus[itemIndex] == STATUS_PICKED_UP) {
    return "PICKED_UP";
  }
  if (foundStatus[itemIndex] == STATUS_NEEDS_VERIFICATION ||
      foundStatus[itemIndex] == STATUS_PICKUP_LOCKED) {
    return "VERIFY";
  }
  return "DEPOSITED";
}

string databaseClaimStatus(int itemIndex) {
  if (foundStatus[itemIndex] == STATUS_PICKED_UP) {
    return "APPROVED";
  }
  if (foundStatus[itemIndex] == STATUS_PICKUP_LOCKED ||
      foundStatus[itemIndex] == STATUS_NEEDS_VERIFICATION) {
    return "LOCKED";
  }
  return "PENDING";
}

bool databaseSaveUsers() {
  for (int i = 0; i < userCount; i++) {
    string lockedUntil = userLocked[i] ? "'9999-12-31 23:59:59'" : "NULL";
    string query;
    if (!databaseUserStored[i]) {
      query = "INSERT INTO users (nim, nama, jurusan, email, no_hp, "
              "password_hash, role, failed_login, locked_until) VALUES (" +
              databaseQuote(userName[i]) + ", " +
              databaseQuote(userFullName[i]) + ", " +
              databaseQuote(userMajor[i]) + ", " +
              databaseQuote(userEmail[i]) + ", " +
              databaseQuote(userPhone[i]) + ", SHA2(" +
              databaseQuote(userPassword[i]) + ", 256), " +
              databaseQuote(userRole[i]) + ", " +
              to_string(userFailed[i]) + ", " + lockedUntil +
              ") ON DUPLICATE KEY UPDATE nama = VALUES(nama), "
              "jurusan = VALUES(jurusan), email = VALUES(email), "
              "no_hp = VALUES(no_hp), password_hash = VALUES(password_hash), "
              "role = VALUES(role), failed_login = VALUES(failed_login), "
              "locked_until = VALUES(locked_until)";
    } else {
      query = "UPDATE users SET nama = " + databaseQuote(userFullName[i]) +
              ", jurusan = " + databaseQuote(userMajor[i]) +
              ", email = " + databaseQuote(userEmail[i]) +
              ", no_hp = " + databaseQuote(userPhone[i]) +
              ", role = " + databaseQuote(userRole[i]) +
              ", failed_login = " + to_string(userFailed[i]) +
              ", locked_until = " + lockedUntil + " WHERE nim = " +
              databaseQuote(userName[i]);
    }
    if (!databaseExecute(query)) {
      return false;
    }
    databaseUserStored[i] = true;
  }
  return true;
}

bool databaseSaveLockers() {
  for (int locker = 0; locker < MAX_LOCKERS; locker++) {
    int lockerId = databaseLockerIdForIndex(locker);
    if (lockerId == 0) {
      cerr << "Loker " << locker + 1 << " tidak ditemukan di database.\n";
      return false;
    }

    string status = lockerBroken[locker] ? "BROKEN"
                                         : (lockerIsOccupied(locker) ? "OCCUPIED"
                                                                     : "READY");
    if (!databaseExecute("UPDATE lockers SET status = " + databaseQuote(status) +
                         " WHERE id_locker = " + to_string(lockerId))) {
      return false;
    }
  }
  return true;
}

bool databaseSaveItems() {
  for (int i = 0; i < foundCount; i++) {
    if (foundReporter[i] < 0 || foundReporter[i] >= userCount) {
      cerr << "Pelapor barang tidak valid.\n";
      return false;
    }

    int categoryId = databaseCategoryId(foundCategory[i]);
    int locationId = databaseLocationId(foundLocation[i]);
    if (categoryId == 0 || locationId == 0) {
      return false;
    }

    string fields = "nim_penemu = " + databaseQuote(userName[foundReporter[i]]) +
                    ", id_kategori = " + to_string(categoryId) +
                    ", id_lokasi = " + to_string(locationId) +
                    ", nama_barang = " + databaseQuote(foundName[i]) +
                    ", ciri_ciri = " + databaseQuote(foundDescription[i]) +
                    ", tanggal_temuan = " + databaseQuote(foundDate[i]) +
                    ", data_verifikasi = " + databaseQuote(foundSecret[i]) +
                    ", status = " + databaseQuote(foundStatus[i]);

    if (databaseItemId[i] == 0) {
      if (!databaseExecute("INSERT INTO items SET " + fields)) {
        return false;
      }
      databaseItemId[i] = static_cast<int>(mysql_insert_id(databaseConnection));
    } else if (!databaseExecute("UPDATE items SET " + fields +
                                " WHERE id_item = " +
                                to_string(databaseItemId[i]))) {
      return false;
    }

    if (foundPhoto[i] != "" &&
        !databaseExecute("INSERT INTO item_photos (id_item, path_foto, urutan) "
                         "VALUES (" + to_string(databaseItemId[i]) + ", " +
                         databaseQuote(foundPhoto[i]) + ", 1) ON DUPLICATE KEY "
                         "UPDATE path_foto = VALUES(path_foto)")) {
      return false;
    }
  }
  return true;
}

bool databaseSaveDeposits() {
  for (int i = 0; i < foundCount; i++) {
    string status = databaseDepositStatus(i);
    int lockerId = foundLocker[i] >= 0
                       ? databaseLockerIdForIndex(foundLocker[i])
                       : databaseDepositLockerId[i];
    if (databaseDepositId[i] == 0 && lockerId == 0) {
      continue;
    }
    if (lockerId == 0) {
      cerr << "Loker barang tidak valid.\n";
      return false;
    }

    string expires = foundReservationExpiry[i] > 0
                         ? "FROM_UNIXTIME(" + to_string(foundReservationExpiry[i]) + ")"
                         : "CURRENT_TIMESTAMP";
    if (databaseDepositId[i] == 0) {
      string depositedAt = status == "DEPOSITED" ? "CURRENT_TIMESTAMP" : "NULL";
      string pickedUpAt = status == "PICKED_UP" ? "CURRENT_TIMESTAMP" : "NULL";
      if (!databaseExecute(
              "INSERT INTO deposits (id_item, id_locker, reserve_expires_at, "
              "deposited_at, picked_up_at, status) VALUES (" +
              to_string(databaseItemId[i]) + ", " + to_string(lockerId) +
              ", " + expires + ", " + depositedAt + ", " + pickedUpAt +
              ", " + databaseQuote(status) + ")")) {
        return false;
      }
      databaseDepositId[i] = static_cast<int>(mysql_insert_id(databaseConnection));
      databaseDepositLockerId[i] = lockerId;
    } else if (!databaseExecute(
                   "UPDATE deposits SET id_locker = " + to_string(lockerId) +
                   ", reserve_expires_at = " + expires + ", status = " +
                   databaseQuote(status) + ", deposited_at = CASE WHEN " +
                   databaseQuote(status) + " = 'DEPOSITED' AND deposited_at IS NULL "
                   "THEN CURRENT_TIMESTAMP ELSE deposited_at END, picked_up_at = "
                   "CASE WHEN " + databaseQuote(status) + " = 'PICKED_UP' AND "
                   "picked_up_at IS NULL THEN CURRENT_TIMESTAMP ELSE picked_up_at END "
                   "WHERE id_deposit = " + to_string(databaseDepositId[i]))) {
      return false;
    }
  }
  return true;
}

bool databaseSaveClaims() {
  for (int i = 0; i < claimCount; i++) {
    if (claimItem[i] < 0 || claimItem[i] >= foundCount || claimUser[i] < 0 ||
        claimUser[i] >= userCount) {
      continue;
    }
    if (databaseFailedClaimId[i] == 0) {
      if (!databaseExecute("INSERT INTO claims (id_item, nim_pengklaim, "
                           "detail_claim, failed_count, status) VALUES (" +
                           to_string(databaseItemId[claimItem[i]]) + ", " +
                           databaseQuote(userName[claimUser[i]]) +
                           ", '', " + to_string(claimFailed[i]) +
                           ", 'REJECTED')")) {
        return false;
      }
      databaseFailedClaimId[i] =
          static_cast<int>(mysql_insert_id(databaseConnection));
    } else if (!databaseExecute("UPDATE claims SET failed_count = " +
                                to_string(claimFailed[i]) +
                                " WHERE id_claim = " +
                                to_string(databaseFailedClaimId[i]))) {
      return false;
    }
  }

  for (int i = 0; i < foundCount; i++) {
    if (foundClaimant[i] < 0) {
      if (databaseActiveClaimId[i] != 0) {
        string status = foundStatus[i] == STATUS_PICKED_UP ? "APPROVED" : "CANCELLED";
        if (!databaseExecute("UPDATE claims SET status = " +
                             databaseQuote(status) +
                             ", verified_at = CASE WHEN " +
                             databaseQuote(status) + " = 'APPROVED' THEN "
                             "CURRENT_TIMESTAMP ELSE verified_at END WHERE "
                             "id_claim = " +
                             to_string(databaseActiveClaimId[i]))) {
          return false;
        }
      }
      continue;
    }

    string status = databaseClaimStatus(i);
    if (databaseActiveClaimId[i] == 0) {
      if (!databaseExecute("INSERT INTO claims (id_item, nim_pengklaim, "
                           "detail_claim, status) VALUES (" +
                           to_string(databaseItemId[i]) + ", " +
                           databaseQuote(userName[foundClaimant[i]]) +
                           ", '', " + databaseQuote(status) + ")")) {
        return false;
      }
      databaseActiveClaimId[i] =
          static_cast<int>(mysql_insert_id(databaseConnection));
    } else if (!databaseExecute("UPDATE claims SET status = " +
                                databaseQuote(status) +
                                ", verified_at = CASE WHEN " +
                                databaseQuote(status) + " = 'APPROVED' THEN "
                                "CURRENT_TIMESTAMP ELSE verified_at END WHERE "
                                "id_claim = " +
                                to_string(databaseActiveClaimId[i]))) {
      return false;
    }
  }
  return true;
}

bool databaseSaveAccessCode(int *codeId, int depositId, int claimId,
                            string type, string code, long expiry,
                            string inactiveStatus,
                            bool keepActiveWithoutPlaintext, int failedCount) {
  if (code == "") {
    if (*codeId == 0) {
      return true;
    }
    if (keepActiveWithoutPlaintext) {
      return databaseExecute("UPDATE access_codes SET failed_count = " +
                             to_string(failedCount) + " WHERE id_code = " +
                             to_string(*codeId));
    }
    return databaseExecute("UPDATE access_codes SET failed_count = " +
                           to_string(failedCount) + ", status = " +
                           databaseQuote(inactiveStatus) +
                           ", used_at = CASE WHEN " +
                           databaseQuote(inactiveStatus) +
                           " = 'USED' THEN CURRENT_TIMESTAMP ELSE used_at END "
                           "WHERE id_code = " + to_string(*codeId));
  }

  string expires = expiry > 0 ? "FROM_UNIXTIME(" + to_string(expiry) + ")"
                              : "CURRENT_TIMESTAMP";
  if (*codeId == 0) {
    string deposit = type == "DEPOSIT" ? to_string(depositId) : "NULL";
    string claim = type == "PICKUP" ? to_string(claimId) : "NULL";
    if (!databaseExecute("INSERT INTO access_codes (id_deposit, id_claim, "
                         "tipe, code_hash, expires_at, failed_count) VALUES (" +
                         deposit + ", " + claim + ", " + databaseQuote(type) +
                         ", SHA2(" + databaseQuote(code) + ", 256), " +
                         expires + ", " + to_string(failedCount) + ")")) {
      return false;
    }
    *codeId = static_cast<int>(mysql_insert_id(databaseConnection));
    return true;
  }

  return databaseExecute("UPDATE access_codes SET code_hash = SHA2(" +
                         databaseQuote(code) + ", 256), expires_at = " +
                         expires + ", failed_count = " +
                         to_string(failedCount) + ", status = 'ACTIVE', "
                         "used_at = NULL WHERE id_code = " +
                         to_string(*codeId));
}

bool databaseSaveLostReports() {
  for (int i = 0; i < lostCount; i++) {
    if (lostReporter[i] < 0 || lostReporter[i] >= userCount) {
      cerr << "Pelapor kehilangan tidak valid.\n";
      return false;
    }
    int categoryId = databaseCategoryId(lostCategory[i]);
    int locationId = databaseLocationId(lostLocation[i]);
    if (categoryId == 0 || locationId == 0) {
      return false;
    }
    string fields = "nim_pelapor = " + databaseQuote(userName[lostReporter[i]]) +
                    ", id_kategori = " + to_string(categoryId) +
                    ", id_lokasi = " + to_string(locationId) +
                    ", nama_barang = " + databaseQuote(lostName[i]) +
                    ", ciri_ciri = " + databaseQuote(lostDescription[i]) +
                    ", tanggal_hilang = " + databaseQuote(lostDate[i]);
    if (databaseLostReportId[i] == 0) {
      if (!databaseExecute("INSERT INTO lost_reports SET " + fields)) {
        return false;
      }
      databaseLostReportId[i] =
          static_cast<int>(mysql_insert_id(databaseConnection));
    } else if (!databaseExecute("UPDATE lost_reports SET " + fields +
                                " WHERE id_laporan = " +
                                to_string(databaseLostReportId[i]))) {
      return false;
    }
  }
  return true;
}

// Persists the in-memory state to the migrated relational schema.
void saveAll() {
  if (!databaseExecute("START TRANSACTION")) {
    return;
  }

  bool saved = databaseSaveUsers() && databaseSaveLockers() &&
               databaseSaveItems() && databaseSaveDeposits() &&
               databaseSaveClaims();
  for (int i = 0; saved && i < foundCount; i++) {
    string depositInactive = foundStatus[i] == STATUS_AVAILABLE ? "USED" : "CANCELLED";
    string pickupInactive = foundStatus[i] == STATUS_PICKED_UP
                                ? "USED"
                                : (foundStatus[i] == STATUS_PICKUP_LOCKED ? "LOCKED"
                                                                             : "CANCELLED");
    bool keepDepositCode = databaseDepositCodeId[i] != 0 &&
                           foundDepositCode[i] == "" &&
                           foundStatus[i] == STATUS_DEPOSIT_PENDING;
    bool keepPickupCode = databasePickupCodeId[i] != 0 &&
                          foundPickupCode[i] == "" &&
                          foundStatus[i] == STATUS_CLAIM_PENDING;
    saved = databaseSaveAccessCode(&databaseDepositCodeId[i], databaseDepositId[i],
                                   0, "DEPOSIT", foundDepositCode[i],
                                   foundReservationExpiry[i], depositInactive,
                                   keepDepositCode, foundDepositFailed[i]) &&
            databaseSaveAccessCode(&databasePickupCodeId[i], 0,
                                   databaseActiveClaimId[i], "PICKUP",
                                   foundPickupCode[i], foundPickupExpiry[i],
                                   pickupInactive, keepPickupCode,
                                   foundPickupFailed[i]);
  }
  saved = saved && databaseSaveLostReports();

  if (!databaseExecute(saved ? "COMMIT" : "ROLLBACK")) {
    cerr << "Gagal menyimpan data ke database.\n";
  }
}

// Finds a user index by login ID without case sensitivity.
int findUserByUsername(string username) {
  username = toLower(username);

  for (int i = 0; i < userCount; i++) {
    if (toLower(userName[i]) == username) {
      return i;
    }
  }

  return -1;
}

// Validates and stores a base user account.
int registerUser(string username, string password,
                 string phone) {
  if (username == "" || password == "" || phone == "")
    return REG_EMPTY_FIELD;

  int length = username.length();
  for (int i = 0; i < length; i++) {
    if (username[i] == ' ') {
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

// Registers a student profile and its login account.
int registerStudent(string nim, string fullName, string major,
                    string email, string phone,
                    string password) {
  nim = trim(nim);
  fullName = trim(fullName);
  major = trim(major);
  email = trim(email);
  phone = trim(phone);

  if (nim == "" || fullName == "" || major == "" || email == "" ||
      phone == "" || password == "") {
    return REG_EMPTY_FIELD;
  }

  int result = registerUser(nim, password, phone);
  if (result != REG_OK) {
    return result;
  }

  int userIndex = userCount - 1;
  userFullName[userIndex] = sanitizeField(fullName);
  userMajor[userIndex] = sanitizeField(major);
  userEmail[userIndex] = sanitizeField(email);
  userRole[userIndex] = ROLE_STUDENT;

  return REG_OK;
}

// Authenticates a user and tracks failed login attempts.
int loginUser(string username, string password) {
  int index = findUserByUsername(username);
  if (index == -1)
    return LOGIN_NOT_FOUND;

  if (userLocked[index] == true)
    return LOGIN_LOCKED;

  if (databasePasswordMatches(userName[index], password)) {
    userFailed[index] = 0;
    return index;
  } else {
    userFailed[index] += 1;
  }

  if (userFailed[index] >= MAX_LOGIN_ATTEMPTS) {
    userLocked[index] = true;
    return LOGIN_LOCKED;
  } else {
    return LOGIN_WRONG_PASSWORD;
  }

  return LOGIN_NOT_FOUND;
}

// Validates and adds a found-item record.
int addFoundItem(int reporterIndex, string name, string category,
                 string location, string dateFound,
                 string description, string secretDetail) {
  if (trim(name) == "" || trim(category) == "" || trim(location) == "" ||
      trim(dateFound) == "" || trim(secretDetail) == "") {
    return ITEM_EMPTY_FIELD;
  }

  if (!isValidDate(dateFound)) {
    return ITEM_INVALID_DATE;
  }

  if (isFutureDate(dateFound)) {
    return ITEM_FUTURE_DATE;
  }

  if (foundCount >= MAX_FOUND) {
    return ITEM_FULL;
  }

  foundReporter[foundCount] = reporterIndex;
  foundName[foundCount] = sanitizeField(name);
  foundCategory[foundCount] = sanitizeField(category);
  foundLocation[foundCount] = sanitizeField(location);
  foundDate[foundCount] = dateFound;
  foundDescription[foundCount] = sanitizeField(description);
  foundSecret[foundCount] = sanitizeField(secretDetail);
  foundStatus[foundCount] = STATUS_AVAILABLE;
  foundPhoto[foundCount] = "";
  foundLocker[foundCount] = -1;
  foundDepositCode[foundCount] = "";
  foundReservationExpiry[foundCount] = 0;
  foundDepositFailed[foundCount] = 0;
  foundClaimant[foundCount] = -1;
  foundPickupCode[foundCount] = "";
  foundPickupExpiry[foundCount] = 0;
  foundPickupFailed[foundCount] = 0;

  foundCount++;
  return ITEM_OK;
}

// Formats a locker index as an A1-C3 label.
string lockerLabel(int lockerIndex) {
  string label = "";
  label += (char)('A' + lockerIndex / 3);
  label += (char)('1' + lockerIndex % 3);
  return label;
}

// Cancels expired deposit reservations and frees their lockers.
void releaseExpiredDepositReservations() {
  long now = (long)time(0);

  for (int i = 0; i < foundCount; i++) {
    if (foundStatus[i] == STATUS_DEPOSIT_PENDING &&
        foundReservationExpiry[i] <= now) {
      foundStatus[i] = STATUS_DEPOSIT_CANCELLED;
      foundLocker[i] = -1;
      foundDepositCode[i] = "";
      foundReservationExpiry[i] = 0;
    }
  }
}

// Finds the first usable unoccupied locker.
int findAvailableLocker() {
  releaseExpiredDepositReservations();

  for (int locker = 0; locker < MAX_LOCKERS; locker++) {
    if (lockerBroken[locker]) {
      continue;
    }
    if (!lockerIsOccupied(locker)) {
      return locker;
    }
  }

  return -1;
}

// Generates an unused six-digit deposit code.
string makeDepositCode() {
  int code = (int)(time(0) % 900000) + 100000;

  for (int attempt = 0; attempt < 900000; attempt++) {
    bool used = false;
    string candidate = to_string(code);

    for (int i = 0; i < foundCount; i++) {
      if (foundStatus[i] == STATUS_DEPOSIT_PENDING &&
          foundDepositCode[i] == candidate) {
        used = true;
        break;
      }
    }

    if (!used) {
      return candidate;
    }

    code++;
    if (code > 999999) {
      code = 100000;
    }
  }

  return "";
}

// Generates an unused six-digit pickup code.
string makePickupCode() {
  int code = (int)(time(0) % 900000) + 100000;

  for (int attempt = 0; attempt < 900000; attempt++) {
    bool used = false;
    string candidate = to_string(code);

    for (int i = 0; i < foundCount; i++) {
      if (foundStatus[i] == STATUS_CLAIM_PENDING &&
          foundPickupCode[i] == candidate) {
        used = true;
        break;
      }
    }

    if (!used) {
      return candidate;
    }

    code++;
    if (code > 999999) {
      code = 100000;
    }
  }

  return "";
}

// Clears pickup claimant, code, expiry, and failure data.
void clearPickupDetails(int itemIndex) {
  foundClaimant[itemIndex] = -1;
  foundPickupCode[itemIndex] = "";
  foundPickupExpiry[itemIndex] = 0;
  foundPickupFailed[itemIndex] = 0;
}

// Creates a pending found-item deposit reservation.
int reserveFoundItem(int reporterIndex, string name, string category,
                     string location, string dateFound,
                     string description, string secretDetail,
                     string photo) {
  if (trim(photo) == "") {
    return ITEM_EMPTY_FIELD;
  }

  int result = addFoundItem(reporterIndex, name, category, location, dateFound,
                            description, secretDetail);
  if (result != ITEM_OK) {
    return result;
  }

  int locker = findAvailableLocker();
  if (locker == -1) {
    foundCount--;
    return ITEM_LOCKER_FULL;
  }

  int itemIndex = foundCount - 1;
  foundPhoto[itemIndex] = sanitizeField(photo);
  foundLocker[itemIndex] = locker;
  foundDepositCode[itemIndex] = makeDepositCode();
  foundReservationExpiry[itemIndex] = (long)time(0) + 10 * 60;
  foundStatus[itemIndex] = STATUS_DEPOSIT_PENDING;

  return ITEM_OK;
}

// Cancels a deposit reservation and frees its locker.
void cancelDepositReservation(int itemIndex) {
  foundStatus[itemIndex] = STATUS_DEPOSIT_CANCELLED;
  foundLocker[itemIndex] = -1;
  foundDepositCode[itemIndex] = "";
  foundReservationExpiry[itemIndex] = 0;
}

// Validates and adds a lost-item report.
int addLostReport(int reporterIndex, string name, string category,
                  string location, string dateLost,
                  string description) {
  if (trim(name) == "" || trim(category) == "" || trim(location) == "" ||
      trim(dateLost) == "") {
    return ITEM_EMPTY_FIELD;
  }

  if (!isValidDate(dateLost)) {
    return ITEM_INVALID_DATE;
  }

  if (isFutureDate(dateLost)) {
    return ITEM_FUTURE_DATE;
  }

  if (lostCount >= MAX_LOST) {
    return ITEM_FULL;
  }

  lostReporter[lostCount] = reporterIndex;
  lostName[lostCount] = sanitizeField(name);
  lostCategory[lostCount] = sanitizeField(category);
  lostLocation[lostCount] = sanitizeField(location);
  lostDate[lostCount] = dateLost;
  lostDescription[lostCount] = sanitizeField(description);

  lostCount++;
  return ITEM_OK;
}

// Finds public items matching the supplied filters.
int searchAvailableItems(string keyword, string category,
                         string location) {

  searchResultCount = 0;

  for (int i = 0; i < foundCount; i++) {

    if (foundStatus[i] != STATUS_AVAILABLE) {
      continue;
    }

    if (keyword != "" && !containsIgnoreCase(foundName[i], keyword) &&
        !containsIgnoreCase(foundDescription[i], keyword)) {
      continue;
    }

    if (category != "" && !containsIgnoreCase(foundCategory[i], category)) {
      continue;
    }

    if (location != "" && !containsIgnoreCase(foundLocation[i], location)) {
      continue;
    }

    searchResult[searchResultCount] = i;
    searchResultCount++;
  }

  return searchResultCount;
}

// Lists found items reported by one user.
int listItemsByReporter(int reporterIndex) {
  searchResultCount = 0;

  for (int i = 0; i < foundCount; i++) {
    if (foundReporter[i] == reporterIndex) {
      searchResult[searchResultCount] = i;
      searchResultCount++;
    }
  }

  return searchResultCount;
}

// Returns prior failed claim attempts for an item and user.
int getFailedClaimCount(int itemIndex, int userIndex) {
  for (int i = 0; i < claimCount; i++) {
    if (claimItem[i] == itemIndex && claimUser[i] == userIndex) {
      return claimFailed[i];
    }
  }

  return 0;
}

// Builds the finder contact text for an item.
string getFinderContact(int itemIndex) {
  int reporterIndex = foundReporter[itemIndex];

  return userName[reporterIndex] + " - " + userPhone[reporterIndex];
}

// Validates a claim and starts pickup when the secret matches.
int submitClaim(int claimantIndex, int itemIndex, string secretAnswer) {

  if (itemIndex < 0 || itemIndex >= foundCount) {
    return CLAIM_ITEM_NOT_FOUND;
  }

  if (foundStatus[itemIndex] != STATUS_AVAILABLE) {
    return CLAIM_NOT_AVAILABLE;
  }

  if (foundReporter[itemIndex] == claimantIndex) {
    return CLAIM_OWN_ITEM;
  }

  if (getFailedClaimCount(itemIndex, claimantIndex) >= MAX_CLAIM_ATTEMPTS) {
    return CLAIM_TOO_MANY_ATTEMPTS;
  }

  if (toLower(trim(secretAnswer)) == toLower(trim(foundSecret[itemIndex]))) {
    foundStatus[itemIndex] = STATUS_CLAIM_PENDING;
    foundClaimant[itemIndex] = claimantIndex;
    foundPickupCode[itemIndex] = makePickupCode();
    foundPickupExpiry[itemIndex] = (long)time(0) + 24 * 60 * 60;
    foundPickupFailed[itemIndex] = 0;

    return CLAIM_APPROVED;
  }

  for (int i = 0; i < claimCount; i++) {
    if (claimItem[i] == itemIndex && claimUser[i] == claimantIndex) {

      claimFailed[i]++;
      if (claimFailed[i] >= MAX_CLAIM_ATTEMPTS) {
        return CLAIM_TOO_MANY_ATTEMPTS;
      }
      return CLAIM_WRONG_DETAIL;
    }
  }

  if (claimCount >= MAX_CLAIMS) {
    return CLAIM_STORAGE_FULL;
  }

  claimItem[claimCount] = itemIndex;
  claimUser[claimCount] = claimantIndex;
  claimFailed[claimCount] = 1;

  claimCount++;

  if (claimFailed[claimCount - 1] >= MAX_CLAIM_ATTEMPTS) {
    return CLAIM_TOO_MANY_ATTEMPTS;
  }
  return CLAIM_WRONG_DETAIL;
}

// Validates a claimant pickup code and tracks failures.
int submitPickupCode(int claimantIndex, int itemIndex, string code) {
  if (foundStatus[itemIndex] != STATUS_CLAIM_PENDING ||
      foundClaimant[itemIndex] != claimantIndex) {
    return PICKUP_LOCKED;
  }

  if ((long)time(0) > foundPickupExpiry[itemIndex]) {
    foundStatus[itemIndex] = STATUS_AVAILABLE;
    clearPickupDetails(itemIndex);
    return PICKUP_EXPIRED;
  }

  if ((foundPickupCode[itemIndex] == "" ||
       trim(code) != foundPickupCode[itemIndex]) &&
      !databaseAccessCodeMatches(databasePickupCodeId[itemIndex], trim(code))) {
    foundPickupFailed[itemIndex]++;
    if (foundPickupFailed[itemIndex] >= MAX_CLAIM_ATTEMPTS) {
      foundStatus[itemIndex] = STATUS_PICKUP_LOCKED;
      foundPickupCode[itemIndex] = "";
      foundPickupExpiry[itemIndex] = 0;
      return PICKUP_LOCKED;
    }
    return PICKUP_WRONG_CODE;
  }

  return PICKUP_OK;
}

// Runs the terminal pickup and locker-sensor simulation.
void showPickupFlow(int claimantIndex, int itemIndex) {
  while (true) {
    string code = trim(readLine("Masukkan kode ambil di loker: "));
    int failedBefore = foundPickupFailed[itemIndex];
    int result = submitPickupCode(claimantIndex, itemIndex, code);

    if (result == PICKUP_EXPIRED) {
      saveAll();
      cout << "Kode ambil kedaluwarsa; barang kembali tersedia.\n";
      return;
    }
    if (result == PICKUP_LOCKED) {
      if (foundStatus[itemIndex] == STATUS_PICKUP_LOCKED &&
          foundPickupFailed[itemIndex] > failedBefore) {
        databaseLogLockerEvent(foundLocker[itemIndex],
                               databaseDepositId[itemIndex], "PIN_FAILURE",
                               "Kode ambil salah; pengambilan dikunci.");
      }
      saveAll();
      cout
          << "Kode ambil terkunci; admin telah diberi penanda verifikasi.\n";
      return;
    }
    if (result == PICKUP_WRONG_CODE) {
      databaseLogLockerEvent(foundLocker[itemIndex], databaseDepositId[itemIndex],
                             "PIN_FAILURE", "Kode ambil salah.");
      cout << "Kode salah. Percobaan tersisa: "
                << MAX_CLAIM_ATTEMPTS - foundPickupFailed[itemIndex] << "\n";
      saveAll();
      continue;
    }

    cout << "[OK] Pintu loker terbuka.\n";
    while (true) {
      int sensor = readInt("\nKONFIRMASI PENGAMBILAN\n" + UI_DIVIDER + "\n"
                           "  1. Barang sudah diambil dan pintu tertutup\n"
                           "  2. Saya belum selesai\n"
                           "  3. Laporkan masalah loker\n" + UI_DIVIDER + "\n"
                           "Pilih tindakan [1-3]: ",
                           1, 3);

      if (sensor == 1) {
        int lockerIndex = foundLocker[itemIndex];
        foundStatus[itemIndex] = STATUS_PICKED_UP;
        foundLocker[itemIndex] = -1;
        clearPickupDetails(itemIndex);
        saveAll();
        databaseLogLockerEvent(lockerIndex, databaseDepositId[itemIndex], "PICKUP",
                               "Barang diambil.");
        cout << "[OK] Pengambilan selesai. Loker kembali tersedia.\n";
        return;
      }
      if (sensor == 3) {
        int lockerIndex = foundLocker[itemIndex];
        foundStatus[itemIndex] = STATUS_NEEDS_VERIFICATION;
        foundPickupCode[itemIndex] = "";
        foundPickupExpiry[itemIndex] = 0;
        saveAll();
        databaseLogLockerEvent(lockerIndex, databaseDepositId[itemIndex], "ANOMALY",
                               "Masalah loker dilaporkan saat pengambilan.");
        cout
            << "Anomali tercatat; loker dikunci untuk verifikasi admin.\n";
        return;
      }
    }
  }
}

// Runs the terminal student registration screen.
void showRegisterScreen() {
  printHeader("Register");

  string nim = trim(readLine("NIM: "));
  string fullName = trim(readLine("Nama lengkap: "));
  string major = selectMajor();
  string email = trim(readLine("Email: "));
  string phone = trim(readLine("Nomor telepon: "));
  string password = readLine("Password (min 6 karakter): ");

  int result = registerStudent(nim, fullName, major, email, phone, password);

  if (result == REG_OK) {
    cout << "Registrasi berhasil! Silakan login.\n";
    saveAll();
  } else if (result == REG_EMPTY_FIELD) {
    cout << "Gagal: Semua kolom harus diisi.\n";
  } else if (result == REG_USERNAME_HAS_SPACE) {
    cout << "Gagal: NIM tidak boleh mengandung spasi.\n";
  } else if (result == REG_DUPLICATE) {
    cout << "Gagal: NIM sudah digunakan.\n";
  } else if (result == REG_PASSWORD_SHORT) {
    cout << "Gagal: Password minimal " << MIN_PASSWORD_LENGTH
              << " karakter.\n";
  } else if (result == REG_PHONE_INVALID) {
    cout << "Gagal: Nomor telepon hanya boleh berisi angka.\n";
  } else if (result == REG_FULL) {
    cout << "Gagal: Kapasitas pengguna sudah penuh.\n";
  } else {
    cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
}

// Runs the terminal login screen.
int showLoginScreen() {
  printHeader("Login");

  string nim = trim(readLine("NIM (atau ID lama): "));
  string password = readLine("Password: ");

  int result = loginUser(nim, password);

  if (result >= 0) {
    cout << "Selamat datang, " << userName[result] << "!\n";
    saveAll();
    return result;
  }

  if (result == LOGIN_NOT_FOUND) {
    cout << "Akun tidak ditemukan.\n";
  } else if (result == LOGIN_WRONG_PASSWORD) {
    cout << "Password salah.\n";
    saveAll();
  } else if (result == LOGIN_LOCKED) {
    cout << "Akun terkunci karena terlalu banyak percobaan gagal.\n";
    saveAll();
  } else {
    cout << "Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
  return -1;
}

// Runs the terminal locker deposit screen.
void showReportFoundScreen(int userIndex) {
  printHeader("Titip Barang");

  string name = readLine("Nama barang: ");
  string category = selectCategory(false);
  string location = readLine("Lokasi ditemukan: ");
  string date = readLine("Tanggal (YYYY-MM-DD): ");
  string photo = trim(readLine("Path foto wajib: "));
  string description = readLine("Deskripsi (boleh kosong): ");
  string secretDetail =
      readLine("Detail rahasia (untuk verifikasi klaim): ");

  int result = reserveFoundItem(userIndex, name, category, location, date,
                                description, secretDetail, photo);

  if (result == ITEM_EMPTY_FIELD) {
    cout << "Gagal: Semua kolom wajib dan path foto harus diisi.\n";
    pauseScreen();
    return;
  }
  if (result == ITEM_INVALID_DATE) {
    cout << "Gagal: Format tanggal tidak valid (gunakan YYYY-MM-DD).\n";
    pauseScreen();
    return;
  }
  if (result == ITEM_FUTURE_DATE) {
    cout << "Gagal: Tanggal tidak boleh berada di masa depan.\n";
    pauseScreen();
    return;
  }
  if (result == ITEM_FULL) {
    cout << "Gagal: Penyimpanan laporan temuan sudah penuh.\n";
    pauseScreen();
    return;
  }
  if (result == ITEM_LOCKER_FULL) {
    cout << "Gagal: Semua loker sedang terisi atau direservasi.\n";
    pauseScreen();
    return;
  }

  int itemIndex = foundCount - 1;
  saveAll();
  cout << "\nRESERVASI LOKER\n" << UI_DIVIDER << "\n";
  cout << "[OK] Loker " << lockerLabel(foundLocker[itemIndex])
       << " dicadangkan selama 10 menit.\n";
  cout << "Kode titip: " << foundDepositCode[itemIndex] << "\n";

  while (foundDepositFailed[itemIndex] < MAX_CLAIM_ATTEMPTS) {
    string code = trim(readLine("Masukkan kode pada keypad loker: "));
    if ((long)time(0) > foundReservationExpiry[itemIndex]) {
      cancelDepositReservation(itemIndex);
      saveAll();
      cout << "Reservasi habis; kode titip dihanguskan.\n";
      pauseScreen();
      return;
    }

    if ((foundDepositCode[itemIndex] != "" &&
         code == foundDepositCode[itemIndex]) ||
        databaseAccessCodeMatches(databaseDepositCodeId[itemIndex], code)) {
      cout << "[OK] Pintu loker terbuka.\n";
      break;
    }

    databaseLogLockerEvent(foundLocker[itemIndex], databaseDepositId[itemIndex],
                           "PIN_FAILURE", "Kode titip salah.");
    foundDepositFailed[itemIndex]++;
    saveAll();
    if (foundDepositFailed[itemIndex] < MAX_CLAIM_ATTEMPTS) {
      cout << "Kode salah. Percobaan tersisa: "
                << MAX_CLAIM_ATTEMPTS - foundDepositFailed[itemIndex] << "\n";
    }
  }

  if (foundDepositFailed[itemIndex] >= MAX_CLAIM_ATTEMPTS) {
    cancelDepositReservation(itemIndex);
    saveAll();
    cout << "Kode salah tiga kali; reservasi dibatalkan.\n";
    pauseScreen();
    return;
  }

  while (true) {
    int sensor = readInt("\nKONFIRMASI PENITIPAN\n" + UI_DIVIDER + "\n"
                         "  1. Barang sudah ditaruh dan pintu tertutup\n"
                         "  2. Saya masih menata barang\n"
                         "  3. Laporkan masalah loker\n" + UI_DIVIDER + "\n"
                         "Pilih tindakan [1-3]: ",
                         1, 3);

    if (sensor == 1) {
      foundStatus[itemIndex] = STATUS_AVAILABLE;
      foundDepositCode[itemIndex] = "";
      foundReservationExpiry[itemIndex] = 0;
      foundDepositFailed[itemIndex] = 0;
      saveAll();
      databaseLogLockerEvent(foundLocker[itemIndex], databaseDepositId[itemIndex],
                             "DROPOFF", "Barang dititipkan.");
      cout << "[OK] Penitipan selesai. Barang sekarang dapat dicari.\n";
      break;
    }

    if (sensor == 3) {
      foundStatus[itemIndex] = STATUS_NEEDS_VERIFICATION;
      foundDepositCode[itemIndex] = "";
      foundReservationExpiry[itemIndex] = 0;
      foundDepositFailed[itemIndex] = 0;
      saveAll();
      databaseLogLockerEvent(foundLocker[itemIndex], databaseDepositId[itemIndex],
                             "ANOMALY", "Masalah loker dilaporkan saat penitipan.");
      cout << "Anomali tercatat; loker dikunci untuk verifikasi admin.\n";
      break;
    }
  }

  pauseScreen();
}

// Runs the terminal lost-item report screen.
void showReportLostScreen(int userIndex, string prefillName) {
  printHeader("Lapor Barang Hilang");

  string name = "";
  if (prefillName != "") {
    cout << "Nama barang (rekomendasi dari pencarian: " << prefillName
              << ")\n";
    name =
        readLine("Nama barang akhir [Tekan Enter untuk pakai rekomendasi]: ");
    if (trim(name) == "") {
      name = prefillName;
    }
  } else {
    name = readLine("Nama barang: ");
  }

  string category = selectCategory(false);
  string location = readLine("Lokasi hilang: ");
  string date = readLine("Tanggal hilang (YYYY-MM-DD): ");
  string description = readLine("Deskripsi (boleh kosong): ");

  int result =
      addLostReport(userIndex, name, category, location, date, description);

  if (result == ITEM_OK) {
    cout << "Laporan barang hilang berhasil disimpan!\n";
    saveAll();
  } else if (result == ITEM_EMPTY_FIELD) {
    cout << "Gagal: Semua kolom wajib harus diisi (kecuali deskripsi).\n";
  } else if (result == ITEM_INVALID_DATE) {
    cout << "Gagal: Format tanggal tidak valid (gunakan YYYY-MM-DD).\n";
  } else if (result == ITEM_FUTURE_DATE) {
    cout << "Gagal: Tanggal tidak boleh berada di masa depan.\n";
  } else if (result == ITEM_FULL) {
    cout << "Gagal: Penyimpanan laporan hilang sudah penuh.\n";
  } else {
    cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
}

void showMyClaimHistory(int userIndex) {
  MYSQL_RES *result = databaseSelect(
      "SELECT i.nama_barang, c.status, DATE_FORMAT(c.claimed_at, "
      "'%Y-%m-%d %H:%i') FROM claims c JOIN items i ON i.id_item = c.id_item "
      "WHERE c.nim_pengklaim = " + databaseQuote(userName[userIndex]) +
      " ORDER BY c.claimed_at DESC, c.id_claim DESC");
  if (result == 0) {
    cout << "Riwayat klaim tidak dapat dimuat.\n";
    return;
  }

  cout << "\nRIWAYAT KLAIM SAYA\n" << UI_DIVIDER << "\n";
  MYSQL_ROW row;
  bool hasClaims = false;
  while ((row = mysql_fetch_row(result)) != 0) {
    hasClaims = true;
    cout << "\n" << databaseField(row, 0) << "\n";
    cout << "Status   : " << databaseField(row, 1) << "\n";
    cout << "Diajukan : " << databaseField(row, 2) << "\n";
  }
  if (!hasClaims) {
    cout << "Kamu belum memiliki riwayat klaim.\n";
  }
  mysql_free_result(result);
}

// Shows the active user’s found-item reports and claims.
void showMyReportsScreen(int userIndex) {
  printHeader("Laporan Saya");

  int count = listItemsByReporter(userIndex);
  cout << "\nLAPORAN TEMUAN SAYA\n" << UI_DIVIDER << "\n";
  if (count == 0) {
    cout << "Kamu belum memiliki laporan barang temuan.\n";
  }

  for (int i = 0; i < searchResultCount; i++) {
    int index = searchResult[i];

    cout << "\n[" << toDisplayNo(index) << "] " << foundName[index]
              << "\n" << UI_DIVIDER << "\n";
    cout << "Lokasi   : " << foundLocation[index] << "\n";
    cout << "Tanggal  : " << foundDate[index] << "\n";
    cout << "Status   : " << foundStatus[index] << "\n";
  }

  showMyClaimHistory(userIndex);
  pauseScreen();
}

// Runs the terminal search, claim, and pickup flow.
void showClaimFlowScreen(int userIndex) {
  printHeader("Cari dan Klaim Barang");

  string keyword = readLine("Nama atau deskripsi barang: ");
  string category = selectCategory(true);
  string location = readLine("Lokasi ditemukan: ");

  int resultCount = searchAvailableItems(keyword, category, location);

  if (resultCount == 0) {
    cout << "\nBarang tidak ditemukan\n";
    int choice = readInt("Apakah anda ingin membuat laporan barang hilang?\n"
                         "1. Ya\n"
                         "2. Tidak\n"
                         "Pilih: ",
                         1, 2);

    if (choice == 1) {
      showReportLostScreen(userIndex, keyword);
    }

    return;
  }

  cout << "\nDAFTAR BARANG DITEMUKAN\n" << UI_DIVIDER << "\n";

  int maxDisplayNo = 0;

  for (int i = 0; i < searchResultCount; i++) {
    int index = searchResult[i];
    int displayNo = toDisplayNo(index);

    cout << "\n[" << displayNo << "] " << foundName[index] << "\n"
              << UI_DIVIDER << "\n";
    cout << "Kategori   : " << foundCategory[index] << "\n";
    cout << "Lokasi     : " << foundLocation[index] << "\n";
    cout << "Tanggal    : " << foundDate[index] << "\n";
    cout << "Deskripsi  : " << foundDescription[index] << "\n";

    if (displayNo > maxDisplayNo) {
      maxDisplayNo = displayNo;
    }
  }

  int displayNo =
      readInt("\nMasukkan nomor barang yang ingin diklaim: ", 1, maxDisplayNo);

  int itemIndex = toIndex(displayNo);

  bool isFound = false;

  for (int i = 0; i < searchResultCount; i++) {
    if (searchResult[i] == itemIndex) {
      isFound = true;
      break;
    }
  }

  if (!isFound) {
    cout << "\nNomor barang tidak tersedia dalam hasil pencarian.\n";
    pauseScreen();
    return;
  }

  string secretAnswer =
      readLine("Masukkan detail khusus untuk membuktikan kepemilikan: ");

  int claimResult = submitClaim(userIndex, itemIndex, secretAnswer);

  if (claimResult == CLAIM_APPROVED) {
    cout << "\nKlaim berhasil disetujui.\n";
    if (foundLocker[itemIndex] >= 0) {
      cout << "Loker: " << lockerLabel(foundLocker[itemIndex]) << "\n";
    }
    cout << "Kode ambil berlaku 24 jam: " << foundPickupCode[itemIndex]
              << "\n";
    saveAll();
    showPickupFlow(userIndex, itemIndex);

  } else if (claimResult == CLAIM_WRONG_DETAIL) {
    cout << "\nDetail yang kamu masukkan salah.\n";
    cout << "Silahkan periksa kembali informasi barang.\n";

    saveAll();

  } else if (claimResult == CLAIM_TOO_MANY_ATTEMPTS) {
    cout
        << "\nKamu sudah mencapai batas percobaan klaim; hubungi admin.\n";
    saveAll();

  } else if (claimResult == CLAIM_ITEM_NOT_FOUND) {
    cout << "\nBarang tidak ditemukan.\n";

  } else if (claimResult == CLAIM_NOT_AVAILABLE) {
    cout << "\nBarang sudah tidak tersedia untuk diklaim.\n";

  } else if (claimResult == CLAIM_OWN_ITEM) {
    cout << "\nKamu tidak dapat mengklaim barang temuanmu sendiri.\n";

  } else if (claimResult == CLAIM_STORAGE_FULL) {
    cout << "\nPenyimpanan data percobaan klaim sudah penuh.\n";

  } else {
    cout << "\nTerjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
}

int showBlockedClaims() {
  int blockedCount = 0;
  cout << "\nKLAIM DIBLOKIR\n" << UI_DIVIDER << "\n";
  for (int i = 0; i < claimCount; i++) {
    if (claimFailed[i] < MAX_CLAIM_ATTEMPTS) {
      continue;
    }

    int itemIndex = claimItem[i];
    int claimantIndex = claimUser[i];
    cout << "  [" << i + 1 << "] " << foundName[itemIndex]
         << "\n      Pengklaim: " << userFullName[claimantIndex] << " ("
         << userName[claimantIndex] << ")\n";
    blockedCount++;
  }
  if (blockedCount == 0) {
    cout << "Tidak ada klaim diblokir.\n";
  }
  return blockedCount;
}

// Runs the terminal admin anomaly and locker workflow.
void showAdminMenu() {
  bool active = true;
  while (active) {
    printHeader("MENU ADMIN");
    cout << "  1. Lihat anomali\n";
    cout << "  2. Tangani barang\n";
    cout << "  3. Tangani klaim diblokir\n";
    cout << "  4. Status loker\n";
    cout << "  5. Logout\n" << UI_DIVIDER << "\n";
    int choice = readInt("Pilih menu [1-5]: ", 1, 5);
    if (choice == 1) {
      bool found = false;
      for (int i = 0; i < foundCount; i++) {
        if (foundStatus[i] == STATUS_NEEDS_VERIFICATION ||
            foundStatus[i] == STATUS_PICKUP_LOCKED ||
            (foundStatus[i] == STATUS_AVAILABLE &&
             isUnclaimedFor30Days(foundDate[i]))) {
          cout << "  [" << toDisplayNo(i) << "] " << foundName[i]
                    << "\n      Status: " << foundStatus[i] << "\n";
          found = true;
        }
      }
      if (!found)
        cout << "Tidak ada anomali aktif.\n";
      pauseScreen();
    } else if (choice == 2) {
      int item = toIndex(readInt("Nomor barang: ", 1, foundCount));
      if (foundStatus[item] == STATUS_PICKUP_LOCKED) {
        int verified = readInt("Pemilik terverifikasi?\n1. Ya, kode baru\n2. "
                               "Tidak, batalkan klaim\nPilih: ",
                               1, 2);
        if (verified == 1) {
          foundStatus[item] = STATUS_CLAIM_PENDING;
          foundPickupCode[item] = makePickupCode();
          foundPickupExpiry[item] = (long)time(0) + 24 * 60 * 60;
          foundPickupFailed[item] = 0;
          cout << "Kode ambil baru: " << foundPickupCode[item] << "\n";
        } else {
          foundStatus[item] = STATUS_AVAILABLE;
          clearPickupDetails(item);
        }
        saveAll();
      } else if (foundStatus[item] == STATUS_NEEDS_VERIFICATION) {
        int action = readInt(
            "1. Reset: barang tersedia\n2. Tandai loker rusak\nPilih: ", 1, 2);
        if (action == 1) {
          foundStatus[item] = STATUS_AVAILABLE;
        } else if (foundLocker[item] >= 0) {
          lockerBroken[foundLocker[item]] = true;
        }
        saveAll();
      } else if (foundStatus[item] == STATUS_AVAILABLE &&
                 isUnclaimedFor30Days(foundDate[item])) {
        foundStatus[item] = STATUS_CAMPUS_HANDOFF;
        foundLocker[item] = -1;
        clearPickupDetails(item);
        saveAll();
        cout << "Barang diserahkan ke pengelola kampus.\n";
      } else {
        cout << "Barang ini tidak memerlukan tindakan admin.\n";
      }
      pauseScreen();
    } else if (choice == 3) {
      if (showBlockedClaims() == 0) {
        pauseScreen();
        continue;
      }

      int claim = toIndex(readInt("Nomor klaim diblokir: ", 1, claimCount));
      if (claimFailed[claim] < MAX_CLAIM_ATTEMPTS) {
        cout << "Klaim tersebut tidak diblokir.\n";
        pauseScreen();
        continue;
      }

      int item = claimItem[claim];
      int verified = readInt("Pemilik terverifikasi?\n1. Ya, kode ambil baru\n2. "
                             "Tidak, tolak klaim\nPilih: ",
                             1, 2);
      if (verified == 1) {
        if (foundStatus[item] != STATUS_AVAILABLE) {
          cout << "Barang tidak lagi tersedia untuk diklaim.\n";
          pauseScreen();
          continue;
        }
        foundStatus[item] = STATUS_CLAIM_PENDING;
        foundClaimant[item] = claimUser[claim];
        foundPickupCode[item] = makePickupCode();
        foundPickupExpiry[item] = (long)time(0) + 24 * 60 * 60;
        foundPickupFailed[item] = 0;
        claimFailed[claim] = 0;
        saveAll();
        cout << "Kode ambil baru: " << foundPickupCode[item] << "\n";
      } else {
        claimFailed[claim] = 0;
        saveAll();
        cout << "Klaim ditolak dan dicatat.\n";
      }
      pauseScreen();
    } else if (choice == 4) {
      cout << "\nLOKER 3 x 3\n" << UI_DIVIDER << "\n";
      for (int i = 0; i < MAX_LOCKERS; i++) {
        string status = lockerBroken[i] ? "DEFECTIVE"
                                         : (lockerIsOccupied(i) ? "FULL" : "READY");
        cout << "  " << lockerLabel(i) << " [" << status << "]";
        if (i % 3 == 2) {
          cout << "\n";
        }
      }
      pauseScreen();
    } else {
      active = false;
    }
  }
}

// Starts the application and routes guest, student, and admin menus.
int main() {
  if (!databaseConnect()) {
    return 1;
  }

  loadAll();
  bool seededDemoData = userCount == 0;
  if (seededDemoData) {
    loadDemoData();
  }
  int usersBeforeAdmin = userCount;
  ensureDemoAdmin();
  if (seededDemoData || userCount != usersBeforeAdmin) {
    saveAll();
  }

  bool isRunning = true;
  while (isRunning) {
    printHeader("MAIN MENU");
    cout << "  1. Register mahasiswa\n";
    cout << "  2. Login\n";
    cout << "  3. Keluar\n" << UI_DIVIDER << "\n";
    int guestChoice = readInt("Pilih menu [1-3]: ", 1, 3);

    if (guestChoice == 1) {
      showRegisterScreen();
    } else if (guestChoice == 2) {
      int userIndex = showLoginScreen();
      if (userIndex >= 0 && userRole[userIndex] == ROLE_ADMIN) {
        showAdminMenu();
        userIndex = -1;
      }
      while (userIndex >= 0) {
        printHeader("MENU MAHASISWA");
        cout << "  1. Titip barang\n";
        cout << "  2. Cari dan klaim barang\n";
        cout << "  3. Riwayat saya\n";
        cout << "  4. Logout\n" << UI_DIVIDER << "\n";
        int userChoice = readInt("Pilih menu [1-4]: ", 1, 4);
        if (userChoice == 1) {
          showReportFoundScreen(userIndex);
        } else if (userChoice == 2) {
          showClaimFlowScreen(userIndex);
        } else if (userChoice == 3) {
          showMyReportsScreen(userIndex);
        } else {
          userIndex = -1;
        }
      }
    } else {
      isRunning = false;
    }
  }

  databaseDisconnect();
  return 0;
}
