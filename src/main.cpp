#include <cctype>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
// Headers live in /usr/include/mysql, so this path resolves without -I and
// editors (clangd) can find it without running mysql_config.
#include <mysql/mysql.h>
#include <crypt.h>
#include <cstring>
#include <random>

using namespace std;

const int MAX_USERS = 100;
const int MAX_FOUND = 200;
const int MAX_LOST = 200;
const int MAX_CLAIMS = 400;
const int MAX_LOCKERS = 9;
const int MAX_LOGS = 500;
const int MAX_LOGIN_ATTEMPTS = 3;
const int LOGIN_LOCK_DURATION_SECONDS = 15 * 60;
const int MAX_CLAIM_ATTEMPTS = 3;
const int MIN_PASSWORD_LENGTH = 6;
const int BCRYPT_COST = 10;
const int CODE_MIN = 100000;
const int CODE_MAX = 999999;
const int CODE_SPAN = CODE_MAX - CODE_MIN + 1;
const int RESERVATION_SECONDS = 10 * 60;
const int PICKUP_VALID_SECONDS = 24 * 60 * 60;
const int UNCLAIMED_DAYS = 30;
const string UI_BORDER(54, '=');
const string UI_DIVIDER(54, '-');
const int CATEGORY_COUNT = 6;
const string CATEGORIES[CATEGORY_COUNT] = {
    "Aksesori", "Dokumen", "Dompet", "Elektronik", "Kunci", "Pakaian"};
const int MAJOR_COUNT = 6;
const string MAJORS[MAJOR_COUNT] = {
    "Informatika", "Sistem Informasi", "Teknik Komputer",
    "Teknik Elektro", "Manajemen", "Akuntansi"};
const int LOCATION_COUNT = 8;
const string LOCATIONS[LOCATION_COUNT] = {
    "Kantin Utama",   "Parkiran Fakultas", "Perpustakaan", "Gedung A",
    "Gedung B",       "Lobi Utama",        "Masjid/Musala", "Lapangan"};

const string STATUS_AVAILABLE = "Tersedia";
const string STATUS_RETURNED = "Dikembalikan";
const string STATUS_DEPOSIT_PENDING = "Menunggu Dititipkan";
const string STATUS_DEPOSIT_CANCELLED = "Reservasi Batal";
const string STATUS_NEEDS_VERIFICATION = "Perlu Verifikasi";
const string STATUS_CLAIM_PENDING = "Dalam Klaim";
const string STATUS_PICKUP_LOCKED = "Pengambilan Terkunci";
const string STATUS_PICKED_UP = "Sudah Diambil";
const string STATUS_CAMPUS_HANDOFF = "Diserahkan ke Kampus";
const string CLAIM_STATUS_FAILED = "Gagal";
const string CLAIM_STATUS_APPROVED = "Disetujui";
const string CLAIM_STATUS_DONE = "Selesai";
const string CLAIM_STATUS_BLOCKED = "Diblokir";
const string CLAIM_STATUS_REJECTED = "Ditolak";
const string LOST_STATUS_SEARCHING = "Dicari";
const string ROLE_STUDENT = "Mahasiswa";
const string ROLE_ADMIN = "Admin";

const string LOG_RESERVASI = "RESERVASI";
const string LOG_RESERVASI_BATAL = "RESERVASI_BATAL";
const string LOG_PINTU_TITIP = "PINTU_TITIP";
const string LOG_TITIP_OK = "TITIP_OK";
const string LOG_PINTU_AMBIL = "PINTU_AMBIL";
const string LOG_AMBIL_OK = "AMBIL_OK";
const string LOG_MASALAH_LOKER = "MASALAH_LOKER";
const string LOG_RESET_ADMIN = "RESET_ADMIN";
const string LOG_LOKER_RUSAK = "LOKER_RUSAK";

const int REG_OK = 0;
const int REG_EMPTY_FIELD = 1;
const int REG_USERNAME_HAS_SPACE = 2;
const int REG_DUPLICATE = 3;
const int REG_PASSWORD_SHORT = 4;
const int REG_PHONE_INVALID = 5;
const int REG_FULL = 6;
const int REG_EMAIL_DUPLICATE = 7;
const int REG_FIELD_TOO_LONG = 8;

const int LOGIN_NOT_FOUND = -1;
const int LOGIN_WRONG_PASSWORD = -2;
const int LOGIN_LOCKED = -3;

const int ITEM_OK = 0;
const int ITEM_EMPTY_FIELD = 1;
const int ITEM_INVALID_DATE = 2;
const int ITEM_FUTURE_DATE = 3;
const int ITEM_FULL = 4;
const int ITEM_LOCKER_FULL = 5;
const int ITEM_FIELD_TOO_LONG = 6;

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
long userLockedUntil[MAX_USERS];
int userCount = 0;

int foundReporter[MAX_FOUND];
string foundName[MAX_FOUND], foundCategory[MAX_FOUND],
    foundLocation[MAX_FOUND], foundDate[MAX_FOUND], foundDescription[MAX_FOUND],
    foundSecret[MAX_FOUND], foundStatus[MAX_FOUND],
    foundDepositCode[MAX_FOUND], foundPickupCode[MAX_FOUND];
// Plaintext of the access code just issued, because only its digest is stored.
string issuedCode = "";
int foundLocker[MAX_FOUND], foundClaimant[MAX_FOUND],
    foundDepositFailed[MAX_FOUND], foundPickupFailed[MAX_FOUND];
long foundReservationExpiry[MAX_FOUND], foundPickupExpiry[MAX_FOUND];
// deposits.reserved_at/deposited_at/picked_up_at and access_codes.used_at.
// used_at belongs to the last issued code and is reset when a new code is made.
long foundReservedAt[MAX_FOUND], foundDepositedAt[MAX_FOUND],
    foundPickedUpAt[MAX_FOUND], foundDepositCodeUsedAt[MAX_FOUND],
    foundPickupCodeUsedAt[MAX_FOUND];
bool lockerBroken[MAX_LOCKERS];
int foundCount = 0;

int lostReporter[MAX_LOST];
string lostName[MAX_LOST], lostCategory[MAX_LOST], lostLocation[MAX_LOST],
    lostDate[MAX_LOST], lostDescription[MAX_LOST], lostStatus[MAX_LOST];
int lostCount = 0;

int claimItem[MAX_CLAIMS], claimUser[MAX_CLAIMS], claimFailed[MAX_CLAIMS];
string claimStatus[MAX_CLAIMS], claimDetail[MAX_CLAIMS];
long claimClaimedAt[MAX_CLAIMS], claimVerifiedAt[MAX_CLAIMS];
int claimCount = 0;

int logLocker[MAX_LOGS], logItem[MAX_LOGS];
string logType[MAX_LOGS], logNote[MAX_LOGS];
long logTime[MAX_LOGS];
int logCount = 0;

int searchResult[MAX_FOUND];
int searchResultCount = 0;

// Forward declarations for helpers used by the storage layer.
string hashText(string text);
void showAdminMenu();
void showClaimFlowScreen(int userIndex);
void showMyReportsScreen(int userIndex);
void showRegisterScreen();
void showReportFoundScreen(int userIndex);
int showLoginScreen();
int findUserByUsername(string username);
int findClaimRecord(int itemIndex, int userIndex);

// Creates the default admin account when the database has no admin yet.
void ensureAdmin() {
  for (int i = 0; i < userCount; i++) {
    if (userRole[i] == ROLE_ADMIN) {
      return;
    }
  }

  if (userCount >= MAX_USERS) {
    return;
  }

  userName[userCount] = "admin";
  userPassword[userCount] = hashText("admin123");
  userPhone[userCount] = "0000000000";
  userFullName[userCount] = "Administrator";
  userMajor[userCount] = "";
  userEmail[userCount] = "";
  userFailed[userCount] = 0;
  userLockedUntil[userCount] = 0;
  userRole[userCount] = ROLE_ADMIN;
  userCount++;
}

// Checks whether text fits the RAT column size, e.g. varchar(100).
bool fitsLength(string text, int limit) { return (int)text.length() <= limit; }

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
  return toLower(text).find(toLower(keyword)) != string::npos;
}

// Replaces delimiter and newline characters in stored text.
string sanitizeField(string s) {
  int length = s.length();
  for (int i = 0; i < length; i++) {
    if (s[i] == '|' || s[i] == '\n' || s[i] == '\r') {
      s[i] = '/';
    }
  }

  return s;
}

// Returns a salted bcrypt hash (60 characters) of text.
string hashText(string text) {
  unsigned char random[16];
  random_device bytes;
  for (int i = 0; i < 16; i++) {
    random[i] = (unsigned char)(bytes() & 0xff);
  }

  char salt[64];
  if (crypt_gensalt_rn("$2b$", BCRYPT_COST, (const char *)random, 16, salt,
                       sizeof(salt)) == NULL) {
    cout << "[Hash] Gagal membuat salt bcrypt.\n";
    exit(1);
  }

  struct crypt_data data;
  memset(&data, 0, sizeof(data));
  char *hash = crypt_rn(text.c_str(), salt, &data, (int)sizeof(data));
  if (hash == NULL) {
    cout << "[Hash] Gagal menghitung bcrypt.\n";
    exit(1);
  }

  return string(hash);
}

// Checks whether stored text is already a bcrypt hash.
bool isHashed(string stored) {
  return stored.length() == 60 && stored.compare(0, 4, "$2b$") == 0;
}

// Checks a typed value against a stored bcrypt hash; the salt is inside it.
bool hashMatches(string typed, string stored) {
  struct crypt_data data;
  memset(&data, 0, sizeof(data));
  char *hash = crypt_rn(typed.c_str(), stored.c_str(), &data, (int)sizeof(data));

  return hash != NULL && stored == hash;
}

// Matches a typed password against a hash or a legacy plaintext value.
bool passwordMatches(string typed, string stored) {
  if (isHashed(stored)) {
    return hashMatches(typed, stored);
  }

  return stored == typed;
}

// Matches a typed access code against a hash or a legacy plaintext value.
bool codeMatches(string typed, string stored) {
  typed = trim(typed);
  if (stored == "") {
    return false;
  }
  if (isHashed(stored)) {
    return hashMatches(typed, stored);
  }

  return stored == typed;
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

// Formats a stored timestamp as YYYY-MM-DD HH:MM.
string formatLogTime(long stamp) {
  time_t raw = (time_t)stamp;
  tm *local = localtime(&raw);
  char result[17];
  strftime(result, sizeof(result), "%Y-%m-%d %H:%M", local);
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

  return difftime(time(0), foundAt) >= UNCLAIMED_DAYS * 24 * 60 * 60;
}

// Converts a zero-based index to a user-facing number.
int toDisplayNo(int index) { return index + 1; }

// Converts a user-facing number to a zero-based index.
int toIndex(int displayNo) { return displayNo - 1; }

// Prompts for and reads one full line of input.
string readLine(string prompt) {
  cout << prompt;
  string result;
  if (!getline(cin, result)) {
    // Input ended (Ctrl-D or a closed pipe), so stop instead of looping.
    cout << "\n";
    exit(0);
  }
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

// Asks the locker sensor question after a door opens; returns 1, 2, or 3.
int readSensorConfirm(string title, string done, string pending) {
  return readInt("\n" + title + "\n" + UI_DIVIDER + "\n  1. " + done +
                     "\n  2. " + pending +
                     "\n  3. Laporkan masalah loker\n" + UI_DIVIDER +
                     "\nPilih tindakan [1-3]: ",
                 1, 3);
}

// Prints a consistent terminal screen heading.
void printHeader(string title) {
  cout << "\n" << UI_BORDER << "\n";
  cout << "                 LOST & FOUND LOCKER\n";
  cout << UI_BORDER << "\n";
  cout << "  " << title << "\n" << UI_DIVIDER << "\n";
}

// Prints a numbered menu and returns the chosen number.
int showMenu(string title, const string options[], int count) {
  printHeader(title);
  for (int i = 0; i < count; i++) {
    cout << "  " << i + 1 << ". " << options[i] << "\n";
  }
  cout << UI_DIVIDER << "\n";

  return readInt("Pilih menu [1-" + to_string(count) + "]: ", 1, count);
}

// Lets the user pick one entry from a fixed list, or "" when 0 is allowed.
string selectFromList(string title, const string options[], int count,
                      bool allowAll) {
  cout << "\n" << title << "\n" << UI_DIVIDER << "\n";
  if (allowAll) {
    cout << "  0. Semua " << toLower(title) << "\n";
  }
  for (int i = 0; i < count; i++) {
    cout << "  " << i + 1 << ". " << options[i] << "\n";
  }
  cout << UI_DIVIDER << "\n";

  int choice = readInt("Pilih " + toLower(title) + " [" +
                           to_string(allowAll ? 0 : 1) + "-" +
                           to_string(count) + "]: ",
                       allowAll ? 0 : 1, count);
  if (choice == 0) {
    return "";
  }
  return options[choice - 1];
}

// Lets the user select one canonical item category.
string selectCategory(bool allowAll) {
  return selectFromList("KATEGORI", CATEGORIES, CATEGORY_COUNT, allowAll);
}

// Lets the user select one canonical campus location.
string selectLocation(bool allowAll) {
  return selectFromList("LOKASI", LOCATIONS, LOCATION_COUNT, allowAll);
}

// Lets the user select one canonical study major.
string selectMajor() {
  return selectFromList("JURUSAN", MAJORS, MAJOR_COUNT, false);
}

// ===================== Storage: MySQL =====================
// MySQL adalah satu-satunya penyimpanan data. Seluruh array dimuat dari
// database saat program mulai dan ditulis ulang setiap saveAll().
MYSQL *dbConn = NULL;

// Closes the MySQL connection when it is open.
void dbClose() {
  if (dbConn != NULL) {
    mysql_close(dbConn);
    dbConn = NULL;
  }
}

// Reads LF_DB_* from the environment, falling back to a default.
string dbSetting(const char *name, const char *fallback) {
  const char *value = getenv(name);
  return (value == NULL || value[0] == '\0') ? string(fallback) : string(value);
}

// Opens the connection from the LF_DB_* settings; false keeps main from running.
bool dbConnect() {
  dbConn = mysql_init(NULL);
  if (dbConn == NULL) {
    cout << "[DB] mysql_init gagal: kehabisan memori\n";
    return false;
  }

  if (mysql_real_connect(dbConn, dbSetting("LF_DB_HOST", "127.0.0.1").c_str(),
                         dbSetting("LF_DB_USER", "lf").c_str(),
                         dbSetting("LF_DB_PASS", "").c_str(),
                         dbSetting("LF_DB_NAME", "lostfound").c_str(),
                         (unsigned int)atoi(dbSetting("LF_DB_PORT", "3306").c_str()),
                         NULL, 0) == NULL) {
    cout << "[DB] Tidak terhubung ke MySQL: " << mysql_error(dbConn) << "\n";
    dbClose();
    return false;
  }

  mysql_set_character_set(dbConn, "utf8mb4");
  return true;
}

// Runs one SQL statement and prints the server error when it fails.
bool dbRun(string sql) {
  if (mysql_query(dbConn, sql.c_str()) != 0) {
    cout << "[DB] Query gagal: " << mysql_error(dbConn) << "\n";
    return false;
  }

  return true;
}

// Escapes text and wraps it in quotes for use inside SQL.
string dbQuote(string text) {
  string out(text.length() * 2 + 1, '\0');
  unsigned long size = mysql_real_escape_string(
      dbConn, &out[0], text.c_str(), (unsigned long)text.length());
  out.resize(size);

  return "'" + out + "'";
}

// Describes a deposit row for the deposits.status column.
string dbDepositStatus(int itemIndex) {
  if (foundStatus[itemIndex] == STATUS_DEPOSIT_PENDING) {
    return "Menunggu";
  }
  if (foundStatus[itemIndex] == STATUS_PICKED_UP) {
    return "Diambil";
  }
  if (foundStatus[itemIndex] == STATUS_NEEDS_VERIFICATION) {
    return "Periksa";
  }
  if (foundStatus[itemIndex] == STATUS_DEPOSIT_CANCELLED) {
    return "Batal";
  }

  return "Tersimpan";
}

// Runs a SELECT and returns its result set, or NULL after printing the error.
MYSQL_RES *dbSelect(string sql) {
  if (mysql_query(dbConn, sql.c_str()) != 0) {
    cout << "[DB] Query gagal: " << mysql_error(dbConn) << "\n";
    return NULL;
  }

  return mysql_store_result(dbConn);
}

// Returns one result column as text, or "" when the value is NULL.
string dbValue(MYSQL_ROW row, int column) {
  if (row[column] == NULL) {
    return "";
  }

  return string(row[column]);
}

// Converts a stored timestamp to SQL, using NULL when the column allows it.
string dbTime(long stamp) {
  if (stamp <= 0) {
    return "NULL";
  }

  return "FROM_UNIXTIME(" + to_string(stamp) + ")";
}

// Like dbTime, but for the NOT NULL timestamp columns of schema.sql.
// FROM_UNIXTIME(0) itself returns NULL, so the epoch is written literally.
string dbTimeRequired(long stamp) {
  if (stamp <= 0) {
    return "'1970-01-01 00:00:00'";
  }

  return dbTime(stamp);
}

// Like dbQuote, but an empty text becomes NULL.
string dbQuoteOrNull(string text) {
  if (text == "") {
    return "NULL";
  }

  return dbQuote(text);
}

// Makes sure a free-text category exists so items can point at it.
bool ensureCategory(string name) {
  return dbRun("INSERT IGNORE INTO categories (nama_kategori) VALUES (" +
               dbQuote(name) + ")");
}

// Makes sure a free-text location exists so items and reports can point at it.
bool ensureLocation(string name) {
  return dbRun("INSERT INTO locations (nama_gedung) SELECT " + dbQuote(name) +
               " FROM DUAL WHERE NOT EXISTS (SELECT 1 FROM locations "
               "WHERE nama_gedung = " +
               dbQuote(name) + ")");
}

// Copies every array into MySQL inside one transaction (all or nothing).
bool dbSyncAll() {
  if (dbConn == NULL) {
    return false;
  }

  bool itemOk[MAX_FOUND] = {};
  bool depositOk[MAX_FOUND] = {};
  bool claimOk[MAX_CLAIMS] = {};
  int skipped = 0;

  bool ok = dbRun("START TRANSACTION");

  // Children first, so foreign keys never block the delete.
  const string tables[10] = {"locker_event_logs", "access_codes", "claims",
                             "deposits",          "lost_reports", "items",
                             "lockers",           "locations",    "categories",
                             "users"};
  for (int i = 0; i < 10 && ok; i++) {
    ok = dbRun("DELETE FROM " + tables[i]);
  }

  for (int i = 0; i < userCount && ok; i++) {
    string locked = dbTime(userLockedUntil[i]);
    ok = dbRun("INSERT INTO users (nim, nama, jurusan, email, no_hp, "
               "password_hash, role, failed_login, locked_until) VALUES (" +
               dbQuote(userName[i]) + "," + dbQuote(userFullName[i]) + "," +
               dbQuote(userMajor[i]) + "," +
               dbQuoteOrNull(userEmail[i]) + "," +
               dbQuote(userPhone[i]) + "," + dbQuote(userPassword[i]) + "," +
               dbQuote(userRole[i]) + "," + to_string(userFailed[i]) + "," +
               locked + ")");
  }

  for (int i = 0; i < CATEGORY_COUNT && ok; i++) {
    ok = dbRun("INSERT INTO categories (id_kategori, nama_kategori) VALUES (" +
               to_string(i + 1) + "," + dbQuote(CATEGORIES[i]) + ")");
  }

  for (int i = 0; i < LOCATION_COUNT && ok; i++) {
    ok = dbRun("INSERT INTO locations (id_lokasi, nama_gedung) VALUES (" +
               to_string(i + 1) + "," + dbQuote(LOCATIONS[i]) + ")");
  }

  for (int i = 0; i < MAX_LOCKERS && ok; i++) {
    ok = dbRun("INSERT INTO lockers (id_locker, nomor_loker, lokasi_unit, "
               "status) VALUES (" +
               to_string(i + 1) + "," + to_string(i + 1) + ",'Loker 3x3'," +
               (lockerBroken[i] ? "'Rusak'" : "'Aktif'") + ")");
  }

  for (int i = 0; i < foundCount && ok; i++) {
    int reporter = foundReporter[i];
    if (reporter < 0 || reporter >= userCount || !isValidDate(foundDate[i])) {
      skipped++;
      continue;
    }

    // Unknown (old, free-text) categories and locations get their own row.
    ok = ensureCategory(foundCategory[i]);
    if (ok) {
      ok = ensureLocation(foundLocation[i]);
    }
    if (ok) {
      ok = dbRun(
          "INSERT INTO items (id_item, nim_penemu, id_kategori, id_lokasi, "
          "nama_barang, ciri_ciri, tanggal_temuan, data_verifikasi, status) "
          "VALUES (" +
          to_string(i + 1) + "," + dbQuote(userName[reporter]) +
          ",(SELECT id_kategori FROM categories WHERE nama_kategori = " +
          dbQuote(foundCategory[i]) +
          " LIMIT 1),(SELECT id_lokasi FROM locations WHERE nama_gedung = " +
          dbQuote(foundLocation[i]) + " LIMIT 1)," + dbQuote(foundName[i]) +
          "," + dbQuote(foundDescription[i]) + "," + dbQuote(foundDate[i]) +
          "," + dbQuote(foundSecret[i]) + "," + dbQuote(foundStatus[i]) + ")");
    }
    itemOk[i] = ok;
  }

  for (int i = 0; i < lostCount && ok; i++) {
    int reporter = lostReporter[i];
    if (reporter < 0 || reporter >= userCount || !isValidDate(lostDate[i])) {
      skipped++;
      continue;
    }

    ok = ensureCategory(lostCategory[i]);
    if (ok) {
      ok = ensureLocation(lostLocation[i]);
    }
    if (ok) {
      ok = dbRun(
          "INSERT INTO lost_reports (id_laporan, nim_pelapor, id_kategori, "
          "id_lokasi, nama_barang, ciri_ciri, tanggal_hilang, status) VALUES (" +
          to_string(i + 1) + "," + dbQuote(userName[reporter]) +
          ",(SELECT id_kategori FROM categories WHERE nama_kategori = " +
          dbQuote(lostCategory[i]) +
          " LIMIT 1),(SELECT id_lokasi FROM locations WHERE nama_gedung = " +
          dbQuote(lostLocation[i]) + " LIMIT 1)," + dbQuote(lostName[i]) + "," +
          dbQuote(lostDescription[i]) + "," + dbQuote(lostDate[i]) + "," +
          dbQuote(lostStatus[i]) + ")");
    }
  }

  for (int i = 0; i < claimCount && ok; i++) {
    int item = claimItem[i];
    int user = claimUser[i];
    if (item < 0 || item >= foundCount || !itemOk[item] || user < 0 ||
        user >= userCount) {
      skipped++;
      continue;
    }

    ok = dbRun("INSERT INTO claims (id_claim, id_item, nim_pengklaim, "
               "detail_claim, failed_count, status, claimed_at, verified_at) "
               "VALUES (" +
               to_string(i + 1) + "," + to_string(item + 1) + "," +
               dbQuote(userName[user]) + "," + dbQuote(claimDetail[i]) + "," +
               to_string(claimFailed[i]) + "," +
               dbQuote(claimStatus[i]) + "," +
               dbTime(claimClaimedAt[i]) + "," +
               dbTime(claimVerifiedAt[i]) + ")");
    claimOk[i] = ok;
  }

  // A deposit row exists for every item that currently sits in a locker.
  for (int i = 0; i < foundCount && ok; i++) {
    if (!itemOk[i] || foundLocker[i] < 0 || foundLocker[i] >= MAX_LOCKERS) {
      continue;
    }

    ok = dbRun("INSERT INTO deposits (id_deposit, id_item, id_locker, "
               "reserved_at, reserve_expires_at, deposited_at, picked_up_at, "
               "status) VALUES (" +
               to_string(i + 1) + "," + to_string(i + 1) + "," +
               to_string(foundLocker[i] + 1) + "," +
               dbTime(foundReservedAt[i]) + "," +
               dbTime(foundReservationExpiry[i]) + "," +
               dbTime(foundDepositedAt[i]) + "," + dbTime(foundPickedUpAt[i]) +
               "," + dbQuote(dbDepositStatus(i)) + ")");
    depositOk[i] = ok;
  }

  // Only active codes are kept: the arrays clear a code once it is used up.
  for (int i = 0; i < foundCount && ok; i++) {
    if (depositOk[i] && foundDepositCode[i] != "") {
      ok = dbRun("INSERT INTO access_codes (id_deposit, id_claim, tipe, "
                 "code_hash, expires_at, used_at, failed_count, status) "
                 "VALUES (" +
                 to_string(i + 1) + ",NULL,'titip'," +
                 dbQuote(foundDepositCode[i]) + "," +
                 dbTimeRequired(foundReservationExpiry[i]) + "," +
                 dbTime(foundDepositCodeUsedAt[i]) + "," +
                 to_string(foundDepositFailed[i]) + ",'Aktif')");
    }

    if (ok && itemOk[i] && foundPickupCode[i] != "" && foundClaimant[i] >= 0) {
      int claim = findClaimRecord(i, foundClaimant[i]);
      if (claim != -1 && claimOk[claim]) {
        ok = dbRun("INSERT INTO access_codes (id_deposit, id_claim, tipe, "
                   "code_hash, expires_at, used_at, failed_count, status) "
                   "VALUES (NULL," +
                   to_string(claim + 1) + ",'ambil'," +
                   dbQuote(foundPickupCode[i]) + "," +
                   dbTimeRequired(foundPickupExpiry[i]) + "," +
                   dbTime(foundPickupCodeUsedAt[i]) + "," +
                   to_string(foundPickupFailed[i]) + ",'Aktif')");
      }
    }
  }

  for (int i = 0; i < logCount && ok; i++) {
    if (logLocker[i] < 0 || logLocker[i] >= MAX_LOCKERS) {
      skipped++;
      continue;
    }

    int item = logItem[i];
    string deposit = "NULL";
    if (item >= 0 && item < foundCount && depositOk[item]) {
      deposit = to_string(item + 1);
    }

    ok = dbRun("INSERT INTO locker_event_logs (id_locker, id_deposit, "
               "tipe_event, waktu, keterangan) VALUES (" +
               to_string(logLocker[i] + 1) + "," + deposit + "," +
               dbQuote(logType[i]) + "," + dbTimeRequired(logTime[i]) + "," +
               dbQuote(logNote[i]) + ")");
  }

  if (ok) {
    ok = dbRun("COMMIT");
  } else {
    dbRun("ROLLBACK");
  }
  if (skipped > 0) {
    cout << "[DB] " << skipped << " baris lama dilewati (data tidak valid).\n";
  }

  return ok;
}

// Reads every array from MySQL (the only storage now).
void loadAll() {
  userCount = 0;
  foundCount = 0;
  lostCount = 0;
  claimCount = 0;
  logCount = 0;

  MYSQL_RES *result = dbSelect(
      "SELECT nim, password_hash, no_hp, failed_login, "
      "IFNULL(UNIX_TIMESTAMP(locked_until), 0), nama, jurusan, email, role "
      "FROM users ORDER BY nim");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL && userCount < MAX_USERS) {
      userName[userCount] = dbValue(row, 0);
      userPassword[userCount] = dbValue(row, 1);
      userPhone[userCount] = dbValue(row, 2);
      userFailed[userCount] = atoi(dbValue(row, 3).c_str());
      userLockedUntil[userCount] = atol(dbValue(row, 4).c_str());
      userFullName[userCount] = dbValue(row, 5);
      userMajor[userCount] = dbValue(row, 6);
      userEmail[userCount] = dbValue(row, 7);
      userRole[userCount] = dbValue(row, 8);

      userCount++;
    }
    mysql_free_result(result);
  }

  result = dbSelect("SELECT status FROM lockers ORDER BY nomor_loker");
  if (result != NULL) {
    MYSQL_ROW row;
    for (int i = 0; i < MAX_LOCKERS && (row = mysql_fetch_row(result)); i++) {
      lockerBroken[i] = dbValue(row, 0) == "Rusak";
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT u.nim, c.nama_kategori, l.nama_gedung, i.tanggal_temuan, "
      "i.nama_barang, i.ciri_ciri, i.data_verifikasi, i.status FROM items i "
      "JOIN users u ON u.nim = i.nim_penemu "
      "JOIN categories c ON c.id_kategori = i.id_kategori "
      "JOIN locations l ON l.id_lokasi = i.id_lokasi ORDER BY i.id_item");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL && foundCount < MAX_FOUND) {
      foundReporter[foundCount] = findUserByUsername(dbValue(row, 0));
      if (foundReporter[foundCount] == -1) {
        continue;
      }
      foundCategory[foundCount] = dbValue(row, 1);
      foundLocation[foundCount] = dbValue(row, 2);
      foundDate[foundCount] = dbValue(row, 3);
      foundName[foundCount] = dbValue(row, 4);
      foundDescription[foundCount] = dbValue(row, 5);
      foundSecret[foundCount] = dbValue(row, 6);
      foundStatus[foundCount] = dbValue(row, 7);
      foundLocker[foundCount] = -1;
      foundDepositCode[foundCount] = "";
      foundReservationExpiry[foundCount] = 0;
      foundDepositFailed[foundCount] = 0;
      foundClaimant[foundCount] = -1;
      foundPickupCode[foundCount] = "";
      foundPickupExpiry[foundCount] = 0;
      foundPickupFailed[foundCount] = 0;
      foundReservedAt[foundCount] = 0;
      foundDepositedAt[foundCount] = 0;
      foundPickedUpAt[foundCount] = 0;
      foundDepositCodeUsedAt[foundCount] = 0;
      foundPickupCodeUsedAt[foundCount] = 0;

      foundCount++;
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT u.nim, c.nama_kategori, l.nama_gedung, r.tanggal_hilang, "
      "r.nama_barang, r.ciri_ciri, r.status FROM lost_reports r "
      "JOIN users u ON u.nim = r.nim_pelapor "
      "JOIN categories c ON c.id_kategori = r.id_kategori "
      "JOIN locations l ON l.id_lokasi = r.id_lokasi ORDER BY r.id_laporan");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL && lostCount < MAX_LOST) {
      lostReporter[lostCount] = findUserByUsername(dbValue(row, 0));
      if (lostReporter[lostCount] == -1) {
        continue;
      }
      lostCategory[lostCount] = dbValue(row, 1);
      lostLocation[lostCount] = dbValue(row, 2);
      lostDate[lostCount] = dbValue(row, 3);
      lostName[lostCount] = dbValue(row, 4);
      lostDescription[lostCount] = dbValue(row, 5);
      lostStatus[lostCount] = dbValue(row, 6);

      lostCount++;
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT id_item, u.nim, detail_claim, failed_count, status, "
      "IFNULL(UNIX_TIMESTAMP(claimed_at), 0), "
      "IFNULL(UNIX_TIMESTAMP(verified_at), 0) FROM claims c "
      "JOIN users u ON u.nim = c.nim_pengklaim ORDER BY id_claim");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL && claimCount < MAX_CLAIMS) {
      int item = atoi(dbValue(row, 0).c_str()) - 1;
      int claimant = findUserByUsername(dbValue(row, 1));
      if (item < 0 || item >= MAX_FOUND || claimant == -1) {
        continue;
      }
      claimItem[claimCount] = item;
      claimUser[claimCount] = claimant;
      claimDetail[claimCount] = dbValue(row, 2);
      claimFailed[claimCount] = atoi(dbValue(row, 3).c_str());
      claimStatus[claimCount] = dbValue(row, 4);
      claimClaimedAt[claimCount] = atol(dbValue(row, 5).c_str());
      claimVerifiedAt[claimCount] = atol(dbValue(row, 6).c_str());

      if (claimStatus[claimCount] == CLAIM_STATUS_APPROVED &&
          item < foundCount && foundStatus[item] == STATUS_CLAIM_PENDING) {
        foundClaimant[item] = claimant;
      }

      claimCount++;
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT id_item, id_locker, IFNULL(UNIX_TIMESTAMP(reserved_at), 0), "
      "IFNULL(UNIX_TIMESTAMP(reserve_expires_at), 0), "
      "IFNULL(UNIX_TIMESTAMP(deposited_at), 0), "
      "IFNULL(UNIX_TIMESTAMP(picked_up_at), 0) FROM deposits");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
      int item = atoi(dbValue(row, 0).c_str()) - 1;
      int locker = atoi(dbValue(row, 1).c_str()) - 1;
      if (item < 0 || item >= foundCount) {
        continue;
      }
      foundLocker[item] = (locker >= 0 && locker < MAX_LOCKERS) ? locker : -1;
      foundReservedAt[item] = atol(dbValue(row, 2).c_str());
      foundReservationExpiry[item] = atol(dbValue(row, 3).c_str());
      foundDepositedAt[item] = atol(dbValue(row, 4).c_str());
      foundPickedUpAt[item] = atol(dbValue(row, 5).c_str());
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT id_deposit, id_claim, tipe, code_hash, "
      "IFNULL(UNIX_TIMESTAMP(expires_at), 0), "
      "IFNULL(UNIX_TIMESTAMP(used_at), 0), failed_count FROM access_codes");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL) {
      int deposit = dbValue(row, 0) == "" ? -1 : atoi(dbValue(row, 0).c_str()) - 1;
      int claim = dbValue(row, 1) == "" ? -1 : atoi(dbValue(row, 1).c_str()) - 1;
      string type = dbValue(row, 2);

      // Deposit 7 holds the codes of the item at index 6, and claim 3 belongs to
      // the claim record at index 2, so ids map straight back to array slots.
      if (type == "titip" && deposit >= 0 && deposit < foundCount) {
        foundDepositCode[deposit] = dbValue(row, 3);
        foundReservationExpiry[deposit] = atol(dbValue(row, 4).c_str());
        foundDepositCodeUsedAt[deposit] = atol(dbValue(row, 5).c_str());
        foundDepositFailed[deposit] = atoi(dbValue(row, 6).c_str());
      } else if (type == "ambil" && claim >= 0 && claim < claimCount) {
        int item = claimItem[claim];
        foundPickupCode[item] = dbValue(row, 3);
        foundPickupExpiry[item] = atol(dbValue(row, 4).c_str());
        foundPickupCodeUsedAt[item] = atol(dbValue(row, 5).c_str());
        foundPickupFailed[item] = atoi(dbValue(row, 6).c_str());
        foundClaimant[item] = claimUser[claim];
      }
    }
    mysql_free_result(result);
  }

  result = dbSelect(
      "SELECT id_locker, id_deposit, tipe_event, "
      "IFNULL(UNIX_TIMESTAMP(waktu), 0), keterangan FROM locker_event_logs "
      "ORDER BY id_log");
  if (result != NULL) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != NULL && logCount < MAX_LOGS) {
      int locker = atoi(dbValue(row, 0).c_str()) - 1;
      if (locker < 0 || locker >= MAX_LOCKERS) {
        continue;
      }
      logLocker[logCount] = locker;
      logItem[logCount] =
          dbValue(row, 1) == "" ? -1 : atoi(dbValue(row, 1).c_str()) - 1;
      logType[logCount] = dbValue(row, 2);
      logTime[logCount] = atol(dbValue(row, 3).c_str());
      logNote[logCount] = dbValue(row, 4);

      logCount++;
    }
    mysql_free_result(result);
  }
}

// Writes every array to MySQL in one transaction.
void saveAll() {
  if (dbConn == NULL) {
    return;
  }

  dbSyncAll();
}

// Appends one locker event, dropping the oldest entry when the log is full.
void addLockerLog(int locker, int item, string type, string note) {
  if (logCount >= MAX_LOGS) {
    for (int i = 0; i < MAX_LOGS - 1; i++) {
      logLocker[i] = logLocker[i + 1];
      logItem[i] = logItem[i + 1];
      logType[i] = logType[i + 1];
      logTime[i] = logTime[i + 1];
      logNote[i] = logNote[i + 1];
    }
    logCount = MAX_LOGS - 1;
  }

  logLocker[logCount] = locker;
  logItem[logCount] = item;
  logType[logCount] = type;
  logTime[logCount] = (long)time(0);
  // keterangan is varchar(255) in the schema; a log must never fail to be written.
  logNote[logCount] = sanitizeField(note).substr(0, 255);

  logCount++;
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

// Finds a user index by email without case sensitivity.
int findUserByEmail(string email) {
  email = toLower(trim(email));
  if (email == "") {
    return -1;
  }

  for (int i = 0; i < userCount; i++) {
    if (toLower(userEmail[i]) == email) {
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
  userPassword[userCount] = hashText(password);
  userPhone[userCount] = phone;
  userFailed[userCount] = 0;
  userLockedUntil[userCount] = 0;

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

  if (!fitsLength(nim, 15) || !fitsLength(fullName, 100) ||
      !fitsLength(major, 50) || !fitsLength(email, 100) ||
      !fitsLength(phone, 20)) {
    return REG_FIELD_TOO_LONG;
  }

  // Checked before registerUser so a rejected email leaves no half-made account.
  if (findUserByEmail(email) != -1) {
    return REG_EMAIL_DUPLICATE;
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

  long now = (long)time(0);
  if (now < userLockedUntil[index]) {
    return LOGIN_LOCKED;
  }
  if (userLockedUntil[index] != 0) {
    // The lock expired on its own, so clear it before checking the password.
    userLockedUntil[index] = 0;
    userFailed[index] = 0;
  }

  if (passwordMatches(password, userPassword[index])) {
    // Legacy plaintext rows are upgraded the first time they log in.
    userPassword[index] = hashText(password);
    userFailed[index] = 0;
    return index;
  } else {
    userFailed[index] += 1;
  }

  if (userFailed[index] >= MAX_LOGIN_ATTEMPTS) {
    userLockedUntil[index] = now + LOGIN_LOCK_DURATION_SECONDS;
    return LOGIN_LOCKED;
  }

  return LOGIN_WRONG_PASSWORD;
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

  if (!fitsLength(name, 100)) {
    return ITEM_FIELD_TOO_LONG;
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
  foundLocker[foundCount] = -1;
  foundDepositCode[foundCount] = "";
  foundReservationExpiry[foundCount] = 0;
  foundDepositFailed[foundCount] = 0;
  foundClaimant[foundCount] = -1;
  foundPickupCode[foundCount] = "";
  foundPickupExpiry[foundCount] = 0;
  foundPickupFailed[foundCount] = 0;
  foundReservedAt[foundCount] = 0;
  foundDepositedAt[foundCount] = 0;
  foundPickedUpAt[foundCount] = 0;
  foundDepositCodeUsedAt[foundCount] = 0;
  foundPickupCodeUsedAt[foundCount] = 0;

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
      addLockerLog(foundLocker[i], i, LOG_RESERVASI_BATAL, foundName[i]);
      foundLocker[i] = -1;
      foundDepositCode[i] = "";
      foundReservationExpiry[i] = 0;
    }
  }
}

// A locker counts as occupied while its item still sits inside, so a deposit row
// that already recorded picked_up_at does not hold the locker.
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

// Generates a six-digit code that no item with the given status uses yet.
string makeCode(string activeStatus, string codes[]) {
  int code = (int)(time(0) % CODE_SPAN) + CODE_MIN;

  for (int attempt = 0; attempt < CODE_SPAN; attempt++) {
    bool used = false;
    string candidate = to_string(code);

    for (int i = 0; i < foundCount; i++) {
      if (foundStatus[i] == activeStatus &&
          codeMatches(candidate, codes[i])) {
        used = true;
        break;
      }
    }

    if (!used) {
      return candidate;
    }

    code++;
    if (code > CODE_MAX) {
      code = CODE_MIN;
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
                     string description, string secretDetail) {
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
  foundLocker[itemIndex] = locker;
  issuedCode = makeCode(STATUS_DEPOSIT_PENDING, foundDepositCode);
  foundDepositCode[itemIndex] = hashText(issuedCode);
  foundReservationExpiry[itemIndex] = (long)time(0) + RESERVATION_SECONDS;
  foundDepositCodeUsedAt[itemIndex] = 0;
  foundReservedAt[itemIndex] = (long)time(0);
  foundStatus[itemIndex] = STATUS_DEPOSIT_PENDING;
  addLockerLog(locker, itemIndex, LOG_RESERVASI, foundName[itemIndex]);

  return ITEM_OK;
}

// Cancels a deposit reservation and frees its locker.
void cancelDepositReservation(int itemIndex) {
  addLockerLog(foundLocker[itemIndex], itemIndex, LOG_RESERVASI_BATAL,
               foundName[itemIndex]);
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

  if (!fitsLength(name, 100)) {
    return ITEM_FIELD_TOO_LONG;
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
  lostStatus[lostCount] = LOST_STATUS_SEARCHING;

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

// Finds the claim record of one item and user, or -1 when there is none.
int findClaimRecord(int itemIndex, int userIndex) {
  for (int i = 0; i < claimCount; i++) {
    if (claimItem[i] == itemIndex && claimUser[i] == userIndex) {
      return i;
    }
  }

  return -1;
}

// Finds the claim record of an item and user, creating it when missing.
int findOrCreateClaim(int itemIndex, int userIndex) {
  int record = findClaimRecord(itemIndex, userIndex);
  if (record != -1) {
    return record;
  }

  if (claimCount >= MAX_CLAIMS) {
    return -1;
  }

  record = claimCount;
  claimItem[record] = itemIndex;
  claimUser[record] = userIndex;
  claimFailed[record] = 0;
  claimClaimedAt[record] = 0;
  claimVerifiedAt[record] = 0;
  claimCount++;

  return record;
}

// Returns prior failed claim attempts for an item and user.
int getFailedClaimCount(int itemIndex, int userIndex) {
  int record = findClaimRecord(itemIndex, userIndex);
  if (record == -1) {
    return 0;
  }

  return claimFailed[record];
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
    issuedCode = makeCode(STATUS_CLAIM_PENDING, foundPickupCode);
    foundPickupCode[itemIndex] = hashText(issuedCode);
    foundPickupExpiry[itemIndex] = (long)time(0) + PICKUP_VALID_SECONDS;
    foundPickupFailed[itemIndex] = 0;
    foundPickupCodeUsedAt[itemIndex] = 0;

    int record = findOrCreateClaim(itemIndex, claimantIndex);
    if (record != -1) {
      claimStatus[record] = CLAIM_STATUS_APPROVED;
      claimClaimedAt[record] = (long)time(0);
      claimDetail[record] = sanitizeField(secretAnswer);
    }

    return CLAIM_APPROVED;
  }

  int record = findOrCreateClaim(itemIndex, claimantIndex);
  if (record == -1) {
    return CLAIM_STORAGE_FULL;
  }

  claimFailed[record]++;
  bool blocked = claimFailed[record] >= MAX_CLAIM_ATTEMPTS;
  claimStatus[record] = blocked ? CLAIM_STATUS_BLOCKED : CLAIM_STATUS_FAILED;
  // The RAT keeps the answer the claimant offered as claim evidence.
  claimDetail[record] = sanitizeField(secretAnswer);

  if (blocked) {
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

  if (!codeMatches(code, foundPickupCode[itemIndex])) {
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
    string code = trim(readLine("Masukkan kode ambil di loker (0 untuk kembali): "));
    if (code == "0") {
      return;
    }
    int result = submitPickupCode(claimantIndex, itemIndex, code);

    if (result == PICKUP_EXPIRED) {
      saveAll();
      cout << "Kode ambil kedaluwarsa; barang kembali tersedia.\n";
      return;
    }
    if (result == PICKUP_LOCKED) {
      saveAll();
      cout
          << "Kode ambil terkunci; admin telah diberi penanda verifikasi.\n";
      return;
    }
    if (result == PICKUP_WRONG_CODE) {
      cout << "Kode salah. Percobaan tersisa: "
                << MAX_CLAIM_ATTEMPTS - foundPickupFailed[itemIndex] << "\n";
      saveAll();
      continue;
    }

    addLockerLog(foundLocker[itemIndex], itemIndex, LOG_PINTU_AMBIL,
                 foundName[itemIndex]);
    foundPickupCodeUsedAt[itemIndex] = (long)time(0);
    cout << "[OK] Pintu loker terbuka.\n";
    while (true) {
      int sensor = readSensorConfirm(
          "KONFIRMASI PENGAMBILAN", "Barang sudah diambil dan pintu tertutup",
          "Saya belum selesai");

      if (sensor == 1) {
        addLockerLog(foundLocker[itemIndex], itemIndex, LOG_AMBIL_OK,
                     foundName[itemIndex]);
        foundPickedUpAt[itemIndex] = (long)time(0);
        int record = findClaimRecord(itemIndex, claimantIndex);
        if (record != -1) {
          claimStatus[record] = CLAIM_STATUS_DONE;
        }
        foundStatus[itemIndex] = STATUS_PICKED_UP;
        clearPickupDetails(itemIndex);
        saveAll();
        cout << "[OK] Pengambilan selesai. Loker kembali tersedia.\n";
        return;
      }
      if (sensor == 3) {
        addLockerLog(foundLocker[itemIndex], itemIndex, LOG_MASALAH_LOKER,
                     foundName[itemIndex]);
        foundStatus[itemIndex] = STATUS_NEEDS_VERIFICATION;
        foundPickupCode[itemIndex] = "";
        foundPickupExpiry[itemIndex] = 0;
        saveAll();
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
  } else if (result == REG_EMAIL_DUPLICATE) {
    cout << "Gagal: Email sudah digunakan.\n";
  } else if (result == REG_PASSWORD_SHORT) {
    cout << "Gagal: Password minimal " << MIN_PASSWORD_LENGTH
              << " karakter.\n";
  } else if (result == REG_PHONE_INVALID) {
    cout << "Gagal: Nomor telepon hanya boleh berisi angka.\n";
  } else if (result == REG_FIELD_TOO_LONG) {
    cout << "Gagal: NIM maksimal 15, nama dan email 100, jurusan 50, dan "
            "telepon 20 karakter.\n";
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
    cout << "Akun terkunci selama 15 menit karena terlalu banyak percobaan gagal.\n";
    saveAll();
  } else {
    cout << "Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
  return -1;
}

// Runs the keypad, door, and sensor steps of a deposit reservation.
void showDepositFlow(int itemIndex) {
  saveAll();
  cout << "\nRESERVASI LOKER\n" << UI_DIVIDER << "\n";
  cout << "[OK] Loker " << lockerLabel(foundLocker[itemIndex])
       << " dicadangkan selama 10 menit.\n";
  cout << "Kode titip: " << issuedCode << "\n";

  while (foundDepositFailed[itemIndex] < MAX_CLAIM_ATTEMPTS) {
    string code = trim(readLine("Masukkan kode pada keypad loker: "));
    if ((long)time(0) > foundReservationExpiry[itemIndex]) {
      cancelDepositReservation(itemIndex);
      saveAll();
      cout << "Reservasi habis; kode titip dihanguskan.\n";
      return;
    }

    if (codeMatches(code, foundDepositCode[itemIndex])) {
      addLockerLog(foundLocker[itemIndex], itemIndex, LOG_PINTU_TITIP,
                   foundName[itemIndex]);
      foundDepositCodeUsedAt[itemIndex] = (long)time(0);
      cout << "[OK] Pintu loker terbuka.\n";
      break;
    }

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
    return;
  }

  while (true) {
    int sensor = readSensorConfirm(
        "KONFIRMASI PENITIPAN", "Barang sudah ditaruh dan pintu tertutup",
        "Saya masih menata barang");

    if (sensor == 1) {
      foundStatus[itemIndex] = STATUS_AVAILABLE;
      foundDepositCode[itemIndex] = "";
      foundReservationExpiry[itemIndex] = 0;
      foundDepositFailed[itemIndex] = 0;
      addLockerLog(foundLocker[itemIndex], itemIndex, LOG_TITIP_OK,
                   foundName[itemIndex]);
      foundDepositedAt[itemIndex] = (long)time(0);
      saveAll();
      cout << "[OK] Penitipan selesai. Barang sekarang dapat dicari.\n";
      break;
    }

    if (sensor == 3) {
      foundStatus[itemIndex] = STATUS_NEEDS_VERIFICATION;
      foundDepositCode[itemIndex] = "";
      foundReservationExpiry[itemIndex] = 0;
      foundDepositFailed[itemIndex] = 0;
      addLockerLog(foundLocker[itemIndex], itemIndex, LOG_MASALAH_LOKER,
                   foundName[itemIndex]);
      saveAll();
      cout << "Anomali tercatat; loker dikunci untuk verifikasi admin.\n";
      break;
    }
  }
}

// Runs the terminal locker deposit screen.
void showReportFoundScreen(int userIndex) {
  printHeader("Titip Barang");

  string name = readLine("Nama barang: ");
  string category = selectCategory(false);
  string location = selectLocation(false);
  string date = readLine("Tanggal (YYYY-MM-DD): ");
  string description = readLine("Deskripsi (boleh kosong): ");
  string secretDetail =
      readLine("Detail rahasia (untuk verifikasi klaim): ");

  int result = reserveFoundItem(userIndex, name, category, location, date,
                                description, secretDetail);

  if (result == ITEM_EMPTY_FIELD) {
    cout << "Gagal: Semua kolom wajib harus diisi.\n";
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
  if (result == ITEM_FIELD_TOO_LONG) {
    cout << "Gagal: Nama barang maksimal 100 karakter.\n";
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

  showDepositFlow(foundCount - 1);
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
  string location = selectLocation(false);
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
  } else if (result == ITEM_FIELD_TOO_LONG) {
    cout << "Gagal: Nama barang maksimal 100 karakter.\n";
  } else if (result == ITEM_FULL) {
    cout << "Gagal: Penyimpanan laporan hilang sudah penuh.\n";
  } else {
    cout << "Gagal: Terjadi kesalahan yang tidak diketahui.\n";
  }

  pauseScreen();
}

void showMyClaimHistory(int userIndex) {
  cout << "\nRIWAYAT KLAIM SAYA\n" << UI_DIVIDER << "\n";
  bool hasClaims = false;
  for (int i = 0; i < foundCount; i++) {
    if (foundClaimant[i] != userIndex) {
      continue;
    }
    hasClaims = true;
    cout << "\n" << foundName[i] << "\n";
    cout << "Status   : " << foundStatus[i] << "\n";
  }
  for (int i = 0; i < claimCount; i++) {
    if (claimUser[i] != userIndex || claimItem[i] < 0 ||
        claimItem[i] >= foundCount) {
      continue;
    }
    hasClaims = true;
    cout << "\n" << foundName[claimItem[i]] << "\n";
    cout << "Status   : " << claimStatus[i] << "\n";
  }
  if (!hasClaims) {
    cout << "Kamu belum memiliki riwayat klaim.\n";
  }
}

// Shows the active user’s found-item reports and claims.
// Shows the found items this user reported.
void showMyFoundReports(int userIndex) {
  int count = listItemsByReporter(userIndex);

  cout << "\nLAPORAN TEMUAN SAYA\n" << UI_DIVIDER << "\n";
  if (count == 0) {
    cout << "Kamu belum memiliki laporan barang temuan.\n";
  }

  for (int i = 0; i < searchResultCount; i++) {
    int index = searchResult[i];

    cout << "\n[" << toDisplayNo(index) << "] " << foundName[index] << "\n"
         << UI_DIVIDER << "\n";
    cout << "Lokasi   : " << foundLocation[index] << "\n";
    cout << "Tanggal  : " << foundDate[index] << "\n";
    cout << "Status   : " << foundStatus[index] << "\n";
  }
}

// Shows the lost reports this user filed.
void showMyLostReports(int userIndex) {
  cout << "\nLAPORAN HILANG SAYA\n" << UI_DIVIDER << "\n";

  int lostShown = 0;
  for (int i = 0; i < lostCount; i++) {
    if (lostReporter[i] != userIndex) {
      continue;
    }

    lostShown++;
    cout << "\n" << lostName[i] << "\n";
    cout << "Lokasi   : " << lostLocation[i] << "\n";
    cout << "Tanggal  : " << lostDate[i] << "\n";
    cout << "Status   : " << lostStatus[i] << "\n";
    cout << "Deskripsi: " << lostDescription[i] << "\n";
  }

  if (lostShown == 0) {
    cout << "Kamu belum memiliki laporan barang hilang.\n";
  }
}

// Shows the active user's found items, lost reports, and claims.
void showMyReportsScreen(int userIndex) {
  printHeader("Laporan Saya");

  showMyFoundReports(userIndex);
  showMyLostReports(userIndex);
  showMyClaimHistory(userIndex);
  pauseScreen();
}

// Runs the terminal search, claim, and pickup flow.
// Lists the claims this user may pick up; true when the caller should stop.
bool showPendingClaims(int userIndex) {
  int pendingClaimCount = 0;
  for (int i = 0; i < foundCount; i++) {
    if (foundStatus[i] == STATUS_CLAIM_PENDING &&
        foundClaimant[i] == userIndex) {
      pendingClaimCount++;
    }
  }

  if (pendingClaimCount == 0) {
    return false;
  }

  cout << "\nKLAIM DISETUJUI BELUM DIAMBIL\n" << UI_DIVIDER << "\n";
  for (int i = 0; i < foundCount; i++) {
    if (foundStatus[i] == STATUS_CLAIM_PENDING &&
        foundClaimant[i] == userIndex) {
      cout << "  [" << toDisplayNo(i) << "] " << foundName[i] << "\n";
    }
  }

  int choice = readInt("1. Cari dan klaim barang baru\n"
                       "2. Ambil barang\n"
                       "Pilih: ",
                       1, 2);
  if (choice == 1) {
    return false;
  }

  int displayNo = readInt("Nomor barang yang akan diambil: ", 1, foundCount);
  int itemIndex = toIndex(displayNo);
  if (foundStatus[itemIndex] != STATUS_CLAIM_PENDING ||
      foundClaimant[itemIndex] != userIndex) {
    cout << "Barang tersebut tidak memiliki klaim aktifmu.\n";
    pauseScreen();
    return true;
  }

  showPickupFlow(userIndex, itemIndex);
  return true;
}

// Asks the three search filters, passing them back by reference.
void askSearchFilters(string &keyword, string &category, string &location) {
  keyword = readLine("Nama atau deskripsi barang: ");
  category = selectCategory(true);
  location = selectLocation(true);
}

// Offers a lost report when the search found nothing.
void showNoResultOffer(int userIndex, string keyword) {
  cout << "\nBarang tidak ditemukan\n";
  int choice = readInt("Apakah anda ingin membuat laporan barang hilang?\n"
                       "1. Ya\n"
                       "2. Tidak\n"
                       "Pilih: ",
                       1, 2);

  if (choice == 1) {
    showReportLostScreen(userIndex, keyword);
  }
}

// Prints the search result cards and returns the chosen item index.
int showSearchResults() {
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

  return toIndex(displayNo);
}

// Checks whether a slot is one of the items currently listed by the search.
bool isSearchResult(int itemIndex) {
  for (int i = 0; i < searchResultCount; i++) {
    if (searchResult[i] == itemIndex) {
      return true;
    }
  }

  return false;
}

// Asks the secret answer, submits the claim, and prints the outcome.
void showClaimAttempt(int userIndex, int itemIndex) {
  string secretAnswer =
      readLine("Masukkan detail khusus untuk membuktikan kepemilikan: ");

  int claimResult = submitClaim(userIndex, itemIndex, secretAnswer);

  if (claimResult == CLAIM_APPROVED) {
    cout << "\nKlaim berhasil disetujui.\n";
    if (foundLocker[itemIndex] >= 0) {
      cout << "Loker: " << lockerLabel(foundLocker[itemIndex]) << "\n";
    }
    cout << "Kode ambil berlaku 24 jam: " << issuedCode << "\n";
    saveAll();
    showPickupFlow(userIndex, itemIndex);

  } else if (claimResult == CLAIM_WRONG_DETAIL) {
    cout << "\nDetail yang kamu masukkan salah.\n";
    cout << "Silahkan periksa kembali informasi barang.\n";

    saveAll();

  } else if (claimResult == CLAIM_TOO_MANY_ATTEMPTS) {
    cout << "\nKamu sudah mencapai batas percobaan klaim; hubungi admin.\n";
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

// Runs the terminal search, claim, and pickup flow.
void showClaimFlowScreen(int userIndex) {
  printHeader("Cari dan Klaim Barang");

  if (showPendingClaims(userIndex)) {
    return;
  }

  string keyword = "";
  string category = "";
  string location = "";
  askSearchFilters(keyword, category, location);

  if (searchAvailableItems(keyword, category, location) == 0) {
    showNoResultOffer(userIndex, keyword);
    return;
  }

  int itemIndex = showSearchResults();

  if (!isSearchResult(itemIndex)) {
    cout << "\nNomor barang tidak tersedia dalam hasil pencarian.\n";
    pauseScreen();
    return;
  }
  if (foundReporter[itemIndex] == userIndex) {
    cout << "\nKamu tidak dapat mengklaim barang temuanmu sendiri.\n";
    pauseScreen();
    return;
  }
  if (getFailedClaimCount(itemIndex, userIndex) >= MAX_CLAIM_ATTEMPTS) {
    cout << "\nKamu sudah mencapai batas percobaan klaim; hubungi admin.\n";
    pauseScreen();
    return;
  }

  showClaimAttempt(userIndex, itemIndex);
}

int showBlockedClaims() {
  int blockedCount = 0;
  cout << "\nKLAIM DIBLOKIR\n" << UI_DIVIDER << "\n";
  for (int i = 0; i < claimCount; i++) {
    if (claimFailed[i] < MAX_CLAIM_ATTEMPTS ||
        claimStatus[i] != CLAIM_STATUS_BLOCKED) {
      continue;
    }

    int itemIndex = claimItem[i];
    int claimantIndex = claimUser[i];
    cout << "  [" << i + 1 << "] " << foundName[itemIndex]
         << "\n      Pengklaim: " << userFullName[claimantIndex] << " ("
         << userName[claimantIndex] << ")\n";
    if (claimDetail[i] != "") {
      cout << "      Detail  : " << claimDetail[i] << "\n";
    }
    blockedCount++;
  }
  if (blockedCount == 0) {
    cout << "Tidak ada klaim diblokir.\n";
  }
  return blockedCount;
}

// Runs the terminal admin anomaly and locker workflow.
// Lists the items that need an admin decision.
void showAdminAnomalies() {
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
  if (!found) {
    cout << "Tidak ada anomali aktif.\n";
  }
}

// Lets the admin resolve the item in the given slot.
void handleAdminItem(int item) {
  if (foundStatus[item] == STATUS_PICKUP_LOCKED) {
    int verified = readInt("Pemilik terverifikasi?\n1. Ya, kode baru\n2. "
                           "Tidak, batalkan klaim\nPilih: ",
                           1, 2);
    if (verified == 1) {
      foundStatus[item] = STATUS_CLAIM_PENDING;
      issuedCode = makeCode(STATUS_CLAIM_PENDING, foundPickupCode);
      foundPickupCode[item] = hashText(issuedCode);
      foundPickupExpiry[item] = (long)time(0) + PICKUP_VALID_SECONDS;
      foundPickupFailed[item] = 0;
      foundPickupCodeUsedAt[item] = 0;
      cout << "Kode ambil baru: " << issuedCode << "\n";
    } else {
      foundStatus[item] = STATUS_AVAILABLE;
      clearPickupDetails(item);
    }
    saveAll();
  } else if (foundStatus[item] == STATUS_NEEDS_VERIFICATION) {
    int action = readInt(
        "1. Reset: barang tersedia\n2. Tandai loker rusak\nPilih: ", 1, 2);
    if (action == 1) {
      addLockerLog(foundLocker[item], item, LOG_RESET_ADMIN, foundName[item]);
      foundStatus[item] = STATUS_AVAILABLE;
      clearPickupDetails(item);
    } else if (foundLocker[item] >= 0) {
      addLockerLog(foundLocker[item], item, LOG_LOKER_RUSAK, foundName[item]);
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
}

// Lets the admin verify or reject one blocked claim; false means "go back".
bool handleAdminBlockedClaim() {
  if (showBlockedClaims() == 0) {
    return false;
  }

  int claim = toIndex(readInt("Nomor klaim diblokir: ", 1, claimCount));
  if (claimFailed[claim] < MAX_CLAIM_ATTEMPTS) {
    cout << "Klaim tersebut tidak diblokir.\n";
    return false;
  }

  int item = claimItem[claim];
  int verified = readInt("Pemilik terverifikasi?\n1. Ya, kode ambil baru\n2. "
                         "Tidak, tolak klaim\nPilih: ",
                         1, 2);
  if (verified == 1) {
    if (foundStatus[item] != STATUS_AVAILABLE) {
      cout << "Barang tidak lagi tersedia untuk diklaim.\n";
      return false;
    }
    foundStatus[item] = STATUS_CLAIM_PENDING;
    foundClaimant[item] = claimUser[claim];
    issuedCode = makeCode(STATUS_CLAIM_PENDING, foundPickupCode);
    foundPickupCode[item] = hashText(issuedCode);
    foundPickupExpiry[item] = (long)time(0) + PICKUP_VALID_SECONDS;
    foundPickupFailed[item] = 0;
    foundPickupCodeUsedAt[item] = 0;
    claimFailed[claim] = 0;
    claimStatus[claim] = CLAIM_STATUS_APPROVED;
    claimVerifiedAt[claim] = (long)time(0);
    saveAll();
    cout << "Kode ambil baru: " << issuedCode << "\n";
  } else {
    claimFailed[claim] = 0;
    claimStatus[claim] = CLAIM_STATUS_REJECTED;
    saveAll();
    cout << "Klaim ditolak dan dicatat.\n";
  }

  return true;
}

// Shows the 3 x 3 locker wall with its current state.
void showAdminLockers() {
  cout << "\nLOKER 3 x 3\n" << UI_DIVIDER << "\n";
  for (int i = 0; i < MAX_LOCKERS; i++) {
    string status = lockerBroken[i]
                        ? "DEFECTIVE"
                        : (lockerIsOccupied(i) ? "FULL" : "READY");
    cout << "  " << lockerLabel(i) << " [" << status << "]";
    if (i % 3 == 2) {
      cout << "\n";
    }
  }
}

// Shows the 20 newest locker events with their label and note.
void showAdminLogs() {
  cout << "\nLOG LOKER (20 TERBARU)\n" << UI_DIVIDER << "\n";
  if (logCount == 0) {
    cout << "Belum ada log loker.\n";
  }
  int shown = 0;
  for (int i = logCount - 1; i >= 0 && shown < 20; i--) {
    shown++;
    cout << "  " << formatLogTime(logTime[i]) << "  ";
    if (logLocker[i] >= 0 && logLocker[i] < MAX_LOCKERS) {
      cout << lockerLabel(logLocker[i]);
    } else {
      cout << "-";
    }
    cout << "  " << logType[i];
    if (logNote[i] != "") {
      cout << "  " << logNote[i];
    }
    cout << "\n";
  }
}

// Runs the student menu until the user logs out.
void showStudentMenu(int userIndex) {
  const string options[4] = {"Titip barang", "Cari dan klaim barang",
                             "Riwayat saya", "Logout"};

  while (true) {
    int choice = showMenu("MENU MAHASISWA", options, 4);
    if (choice == 1) {
      showReportFoundScreen(userIndex);
    } else if (choice == 2) {
      showClaimFlowScreen(userIndex);
    } else if (choice == 3) {
      showMyReportsScreen(userIndex);
    } else {
      return;
    }
  }
}

// Sends a logged in user to the menu of their role.
void routeUser(int userIndex) {
  if (userRole[userIndex] == ROLE_ADMIN) {
    showAdminMenu();
  } else {
    showStudentMenu(userIndex);
  }
}

// Runs the guest menu until the program is closed.
void showMainMenu() {
  const string options[3] = {"Register mahasiswa", "Login", "Keluar"};

  while (true) {
    int choice = showMenu("MAIN MENU", options, 3);
    if (choice == 1) {
      showRegisterScreen();
    } else if (choice == 2) {
      int userIndex = showLoginScreen();
      if (userIndex >= 0) {
        routeUser(userIndex);
      }
    } else {
      return;
    }
  }
}

// Runs the terminal admin menu.
void showAdminMenu() {
  const string options[6] = {"Lihat anomali", "Tangani barang",
                             "Tangani klaim diblokir", "Status loker",
                             "Lihat log loker", "Logout"};
  bool active = true;
  while (active) {
    int choice = showMenu("MENU ADMIN", options, 6);

    if (choice == 1) {
      showAdminAnomalies();
    } else if (choice == 2) {
      handleAdminItem(toIndex(readInt("Nomor barang: ", 1, foundCount)));
    } else if (choice == 3) {
      if (!handleAdminBlockedClaim()) {
        pauseScreen();
        continue;
      }
    } else if (choice == 4) {
      showAdminLockers();
    } else if (choice == 5) {
      showAdminLogs();
    } else {
      active = false;
      continue;
    }
    pauseScreen();
  }
}

// Starts the application and routes guest, student, and admin menus.
int main() {
  if (!dbConnect()) {
    cout << "Gunakan LF_DB_HOST, LF_DB_PORT, LF_DB_USER, LF_DB_PASS, dan "
            "LF_DB_NAME.\n";
    return 1;
  }

  loadAll();
  ensureAdmin();
  saveAll();
  showMainMenu();
  dbClose();

  return 0;
}
