#include <cctype>
#include <climits>
#include <ctime>
#include <fstream>
#include <iostream>
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
    foundPickupFailed[MAX_FOUND];
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

// Seeds demo accounts and found items when storage is empty.
void loadDemoData() {
  if (userCount == 0 && foundCount == 0 && lostCount == 0) {

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

    userRole[0] = ROLE_STUDENT;
    userRole[1] = ROLE_STUDENT;

    userName[2] = "admin";
    userPassword[2] = "admin123";
    userPhone[2] = "0000000000";
    userFullName[2] = "Administrator";
    userMajor[2] = "";
    userEmail[2] = "";
    userFailed[2] = 0;
    userLocked[2] = false;
    userRole[2] = ROLE_ADMIN;

    foundReporter[0] = 0;
    foundName[0] = "Payung Hitam";
    foundCategory[0] = "Aksesori";
    foundLocation[0] = "Kantin Utama";
    foundDate[0] = "2026-10-25";
    foundDescription[0] = "Payung lipat warna hitam merk standar";
    foundSecret[0] = "Ada stiker kecil bergambar bintang di pegangannya";
    foundStatus[0] = STATUS_AVAILABLE;
    foundPhoto[0] = "";
    foundLocker[0] = -1;
    foundDepositCode[0] = "";
    foundReservationExpiry[0] = 0;
    foundClaimant[0] = -1;
    foundPickupCode[0] = "";
    foundPickupExpiry[0] = 0;
    foundPickupFailed[0] = 0;

    foundReporter[1] = 1;
    foundName[1] = "Dompet Kulit Coklat";
    foundCategory[1] = "Dompet";
    foundLocation[1] = "Parkiran Fakultas";
    foundDate[1] = "2026-10-26";
    foundDescription[1] =
        "Dompet berisi kartu identitas dan beberapa kartu penting";
    foundSecret[1] = "Ada foto pelajar di dalam selipan kartu";
    foundStatus[1] = STATUS_AVAILABLE;
    foundPhoto[1] = "";
    foundLocker[1] = -1;
    foundDepositCode[1] = "";
    foundReservationExpiry[1] = 0;
    foundClaimant[1] = -1;
    foundPickupCode[1] = "";
    foundPickupExpiry[1] = 0;
    foundPickupFailed[1] = 0;

    userCount = 3;
    foundCount = 2;
  }
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

// Splits one delimited storage line into fields.
int splitLine(string line, char delimiter, string parts[],
              int maxParts) {
  string part = "";
  int amount = 0;

  int length = line.length();
  for (int i = 0; i < length; i++) {
    if (line[i] == delimiter) {
      if (amount < maxParts) {
        parts[amount] = part;
        amount++;
      }

      part = "";
    } else {
      part += line[i];
    }
  }

  if (amount < maxParts) {
    parts[amount] = part;
    amount++;
  }

  return amount;
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

// Loads persisted users, lockers, items, reports, and claims.
void loadAll() {
  userCount = 0;
  foundCount = 0;
  lostCount = 0;
  claimCount = 0;

  ifstream userFile("users.txt");
  if (userFile.is_open()) {
    string line;
    while (getline(userFile, line)) {
      if (userCount >= MAX_USERS) {
        break;
      }
      string parts[9];
      int partsCount = splitLine(line, '|', parts, 9);
      if (partsCount == 5 || partsCount == 8 || partsCount == 9) {
        userName[userCount] = parts[0];
        userPassword[userCount] = parts[1];
        userPhone[userCount] = parts[2];
        userFullName[userCount] = partsCount >= 8 ? parts[5] : "";
        userMajor[userCount] = partsCount >= 8 ? parts[6] : "";
        userEmail[userCount] = partsCount >= 8 ? parts[7] : "";
        userRole[userCount] = partsCount == 9 ? parts[8] : ROLE_STUDENT;

        int failedVal = 0;
        for (int i = 0; i < (int)parts[3].length(); i++) {
          if (parts[3][i] >= '0' && parts[3][i] <= '9') {
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

  ifstream lockerFile("lockers.txt");
  if (lockerFile.is_open()) {
    string line;
    for (int i = 0; i < MAX_LOCKERS && getline(lockerFile, line); i++) {
      lockerBroken[i] = (line == "1");
    }
    lockerFile.close();
  }

  ifstream foundFile("found_items.txt");
  if (foundFile.is_open()) {
    string line;
    while (getline(foundFile, line)) {
      if (foundCount >= MAX_FOUND) {
        break;
      }
      string parts[16];
      int partsCount = splitLine(line, '|', parts, 16);
      if (partsCount == 8 || partsCount == 12 || partsCount == 16) {
        int repIndex = 0;
        for (int i = 0; i < (int)parts[0].length(); i++) {
          if (parts[0][i] >= '0' && parts[0][i] <= '9') {
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
        foundPhoto[foundCount] = partsCount >= 12 ? parts[8] : "";
        foundLocker[foundCount] = -1;
        foundDepositCode[foundCount] = partsCount >= 12 ? parts[10] : "";
        foundReservationExpiry[foundCount] = 0;
        foundClaimant[foundCount] = -1;
        foundPickupCode[foundCount] = partsCount == 16 ? parts[13] : "";
        foundPickupExpiry[foundCount] = 0;
        foundPickupFailed[foundCount] = 0;

        if (partsCount >= 12) {
          int locker = 0;
          bool lockerValid = parts[9] != "";
          int start = 0;
          if (lockerValid && parts[9][0] == '-') {
            start = 1;
          }
          if (start == (int)parts[9].length()) {
            lockerValid = false;
          }
          for (int i = start; i < (int)parts[9].length(); i++) {
            if (parts[9][i] < '0' || parts[9][i] > '9') {
              lockerValid = false;
              break;
            }
            locker = locker * 10 + (parts[9][i] - '0');
            if (locker > MAX_LOCKERS) {
              lockerValid = false;
              break;
            }
          }
          if (lockerValid && parts[9][0] == '-') {
            locker = -locker;
          }
          if (lockerValid && locker >= -1 && locker < MAX_LOCKERS) {
            foundLocker[foundCount] = locker;
          }

          long expiry = 0;
          bool expiryValid = parts[11] != "";
          for (int i = 0; i < (int)parts[11].length(); i++) {
            if (parts[11][i] < '0' || parts[11][i] > '9') {
              expiryValid = false;
              break;
            }
            int digit = parts[11][i] - '0';
            if (expiry > (LONG_MAX - digit) / 10) {
              expiryValid = false;
              break;
            }
            expiry = expiry * 10 + digit;
          }
          if (expiryValid) {
            foundReservationExpiry[foundCount] = expiry;
          }
        }

        if (partsCount == 16) {
          if (isAllDigits(parts[12])) {
            int claimant = 0;
            bool claimantValid = true;
            for (int i = 0; i < (int)parts[12].length(); i++) {
              int digit = parts[12][i] - '0';
              if (claimant > (MAX_USERS - digit) / 10) {
                claimantValid = false;
                break;
              }
              claimant = claimant * 10 + digit;
            }
            if (claimantValid && claimant < MAX_USERS) {
              foundClaimant[foundCount] = claimant;
            }
          }

          long pickupExpiry = 0;
          bool pickupExpiryValid = isAllDigits(parts[14]);
          for (int i = 0; i < (int)parts[14].length(); i++) {
            int digit = parts[14][i] - '0';
            if (pickupExpiry > (LONG_MAX - digit) / 10) {
              pickupExpiryValid = false;
              break;
            }
            pickupExpiry = pickupExpiry * 10 + digit;
          }
          if (pickupExpiryValid) {
            foundPickupExpiry[foundCount] = pickupExpiry;
          }

          if (isAllDigits(parts[15])) {
            int failed = 0;
            bool failedValid = true;
            for (int i = 0; i < (int)parts[15].length(); i++) {
              int digit = parts[15][i] - '0';
              if (failed > (MAX_CLAIM_ATTEMPTS - digit) / 10) {
                failedValid = false;
                break;
              }
              failed = failed * 10 + digit;
            }
            if (failedValid && failed <= MAX_CLAIM_ATTEMPTS) {
              foundPickupFailed[foundCount] = failed;
            }
          }
        }

        foundCount++;
      }
    }
    foundFile.close();
  }

  ifstream lostFile("lost_reports.txt");
  if (lostFile.is_open()) {
    string line;
    while (getline(lostFile, line)) {
      if (lostCount >= MAX_LOST) {
        break;
      }
      string parts[6];
      int partsCount = splitLine(line, '|', parts, 6);
      if (partsCount == 6) {
        int repIndex = 0;
        for (int i = 0; i < (int)parts[0].length(); i++) {
          if (parts[0][i] >= '0' && parts[0][i] <= '9') {
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

  ifstream claimFile("claims.txt");
  if (claimFile.is_open()) {
    string line;
    while (getline(claimFile, line)) {
      if (claimCount >= MAX_CLAIMS) {
        break;
      }
      string parts[3];
      int partsCount = splitLine(line, '|', parts, 3);
      if (partsCount == 3) {
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

// Persists users, lockers, items, reports, and claims.
void saveAll() {
  ofstream userFile("users.txt");
  if (userFile.is_open()) {
    for (int i = 0; i < userCount; i++) {
      userFile << sanitizeField(userName[i]) << "|"
               << sanitizeField(userPassword[i]) << "|"
               << sanitizeField(userPhone[i]) << "|" << userFailed[i] << "|"
               << (userLocked[i] ? 1 : 0) << "|"
               << sanitizeField(userFullName[i]) << "|"
               << sanitizeField(userMajor[i]) << "|"
               << sanitizeField(userEmail[i]) << "|"
               << sanitizeField(userRole[i]) << "\n";
    }
    userFile.close();
  }

  ofstream foundFile("found_items.txt");
  if (foundFile.is_open()) {
    for (int i = 0; i < foundCount; i++) {
      foundFile << foundReporter[i] << "|" << sanitizeField(foundName[i]) << "|"
                << sanitizeField(foundCategory[i]) << "|"
                << sanitizeField(foundLocation[i]) << "|" << foundDate[i] << "|"
                << sanitizeField(foundDescription[i]) << "|"
                << sanitizeField(foundSecret[i]) << "|" << foundStatus[i] << "|"
                << sanitizeField(foundPhoto[i]) << "|" << foundLocker[i] << "|"
                << sanitizeField(foundDepositCode[i]) << "|"
                << foundReservationExpiry[i] << "|" << foundClaimant[i] << "|"
                << sanitizeField(foundPickupCode[i]) << "|"
                << foundPickupExpiry[i] << "|" << foundPickupFailed[i] << "\n";
    }
    foundFile.close();
  }

  ofstream lockerFile("lockers.txt");
  if (lockerFile.is_open()) {
    for (int i = 0; i < MAX_LOCKERS; i++) {
      lockerFile << (lockerBroken[i] ? 1 : 0) << "\n";
    }
    lockerFile.close();
  }

  ofstream lostFile("lost_reports.txt");
  if (lostFile.is_open()) {
    for (int i = 0; i < lostCount; i++) {
      lostFile << lostReporter[i] << "|" << sanitizeField(lostName[i]) << "|"
               << sanitizeField(lostCategory[i]) << "|"
               << sanitizeField(lostLocation[i]) << "|" << lostDate[i] << "|"
               << sanitizeField(lostDescription[i]) << "\n";
    }
    lostFile.close();
  }

  ofstream claimFile("claims.txt");
  if (claimFile.is_open()) {
    for (int i = 0; i < claimCount; i++) {
      claimFile << claimItem[i] << "|" << claimUser[i] << "|" << claimFailed[i]
                << "\n";
    }
    claimFile.close();
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

  if (userPassword[index] == password) {
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
    bool occupied = false;

    for (int i = 0; i < foundCount; i++) {
      if (foundLocker[i] == locker &&
          (foundStatus[i] == STATUS_DEPOSIT_PENDING ||
           foundStatus[i] == STATUS_AVAILABLE ||
           foundStatus[i] == STATUS_NEEDS_VERIFICATION)) {
        occupied = true;
        break;
      }
    }

    if (!occupied) {
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

  if (trim(code) != foundPickupCode[itemIndex]) {
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

    cout << "[OK] Pintu loker terbuka.\n";
    while (true) {
      int sensor = readInt("\nKONFIRMASI PENGAMBILAN\n" + UI_DIVIDER + "\n"
                           "  1. Barang sudah diambil dan pintu tertutup\n"
                           "  2. Saya belum selesai\n"
                           "  3. Laporkan masalah loker\n" + UI_DIVIDER + "\n"
                           "Pilih tindakan [1-3]: ",
                           1, 3);

      if (sensor == 1) {
        foundStatus[itemIndex] = STATUS_PICKED_UP;
        foundLocker[itemIndex] = -1;
        clearPickupDetails(itemIndex);
        saveAll();
        cout << "[OK] Pengambilan selesai. Loker kembali tersedia.\n";
        return;
      }
      if (sensor == 3) {
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

  int failedAttempts = 0;
  while (failedAttempts < MAX_CLAIM_ATTEMPTS) {
    if ((long)time(0) > foundReservationExpiry[itemIndex]) {
      cancelDepositReservation(itemIndex);
      saveAll();
      cout << "Reservasi habis; kode titip dihanguskan.\n";
      pauseScreen();
      return;
    }

    string code = trim(readLine("Masukkan kode pada keypad loker: "));
    if (code == foundDepositCode[itemIndex]) {
      cout << "[OK] Pintu loker terbuka.\n";
      break;
    }

    failedAttempts++;
    if (failedAttempts < MAX_CLAIM_ATTEMPTS) {
      cout << "Kode salah. Percobaan tersisa: "
                << MAX_CLAIM_ATTEMPTS - failedAttempts << "\n";
    }
  }

  if (failedAttempts >= MAX_CLAIM_ATTEMPTS) {
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
      saveAll();
      cout << "[OK] Penitipan selesai. Barang sekarang dapat dicari.\n";
      break;
    }

    if (sensor == 3) {
      foundStatus[itemIndex] = STATUS_NEEDS_VERIFICATION;
      foundDepositCode[itemIndex] = "";
      foundReservationExpiry[itemIndex] = 0;
      saveAll();
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

// Shows the active user’s found-item reports.
void showMyReportsScreen(int userIndex) {
  printHeader("Laporan Saya");

  int count = listItemsByReporter(userIndex);

  if (count == 0) {
    cout << "Kamu belum memiliki laporan barang temuan.\n";
    pauseScreen();
    return;
  }

  for (int i = 0; i < searchResultCount; i++) {
    int index = searchResult[i];

    cout << "\n[" << toDisplayNo(index) << "] " << foundName[index]
              << "\n" << UI_DIVIDER << "\n";
    cout << "Lokasi   : " << foundLocation[index] << "\n";
    cout << "Tanggal  : " << foundDate[index] << "\n";
    cout << "Status   : " << foundStatus[index] << "\n";
  }

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

// Runs the terminal admin anomaly and locker workflow.
void showAdminMenu() {
  bool active = true;
  while (active) {
    printHeader("MENU ADMIN");
    cout << "  1. Lihat anomali\n";
    cout << "  2. Tangani barang\n";
    cout << "  3. Status loker\n";
    cout << "  4. Logout\n" << UI_DIVIDER << "\n";
    int choice = readInt("Pilih menu [1-4]: ", 1, 4);
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
      cout << "\nLOKER 3 x 3\n" << UI_DIVIDER << "\n";
      for (int i = 0; i < MAX_LOCKERS; i++) {
        cout << "  " << lockerLabel(i) << " ["
             << (lockerBroken[i] ? "RUSAK" : " SIAP") << "]";
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
  loadAll();
  if (userCount == 0) {
    loadDemoData();
  }
  ensureDemoAdmin();

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

  return 0;
}
