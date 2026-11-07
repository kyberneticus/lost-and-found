-- MySQL 8+ schema derived from docs/erd.drawio and docs/rat.drawio.
-- The script is rerunnable and does not drop existing data.

CREATE DATABASE IF NOT EXISTS lostfound
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE lostfound;

CREATE TABLE IF NOT EXISTS users (
  nim VARCHAR(15) NOT NULL,
  nama VARCHAR(100) NOT NULL,
  jurusan VARCHAR(50) NOT NULL,
  email VARCHAR(100) NOT NULL,
  no_hp VARCHAR(20) NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  role VARCHAR(10) NOT NULL DEFAULT 'Mahasiswa',
  failed_login INT UNSIGNED NOT NULL DEFAULT 0,
  locked_until DATETIME NULL,
  PRIMARY KEY (nim),
  UNIQUE KEY uq_users_email (email),
  CONSTRAINT chk_users_role CHECK (role IN ('Mahasiswa', 'Admin'))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS categories (
  id_kategori INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nama_kategori VARCHAR(50) NOT NULL,
  PRIMARY KEY (id_kategori),
  UNIQUE KEY uq_categories_nama (nama_kategori)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS locations (
  id_lokasi INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nama_gedung VARCHAR(100) NOT NULL,
  detail VARCHAR(150) NOT NULL,
  PRIMARY KEY (id_lokasi)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS lockers (
  id_locker INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nomor_loker INT UNSIGNED NOT NULL,
  lokasi_unit VARCHAR(50) NOT NULL,
  status VARCHAR(15) NOT NULL DEFAULT 'READY',
  PRIMARY KEY (id_locker),
  UNIQUE KEY uq_lockers_nomor (nomor_loker),
  CONSTRAINT chk_lockers_status CHECK (status IN ('READY', 'BROKEN', 'OCCUPIED'))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS items (
  id_item INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nim_penemu VARCHAR(15) NOT NULL,
  id_kategori INT UNSIGNED NOT NULL,
  id_lokasi INT UNSIGNED NOT NULL,
  nama_barang VARCHAR(100) NOT NULL,
  ciri_ciri TEXT NULL,
  tanggal_temuan DATE NOT NULL,
  data_verifikasi TEXT NOT NULL,
  status VARCHAR(20) NOT NULL DEFAULT 'Menunggu Dititipkan',
  PRIMARY KEY (id_item),
  KEY idx_items_search (status, id_kategori, id_lokasi, tanggal_temuan),
  CONSTRAINT fk_items_penemu
    FOREIGN KEY (nim_penemu) REFERENCES users (nim),
  CONSTRAINT fk_items_kategori
    FOREIGN KEY (id_kategori) REFERENCES categories (id_kategori),
  CONSTRAINT fk_items_lokasi
    FOREIGN KEY (id_lokasi) REFERENCES locations (id_lokasi),
  CONSTRAINT chk_items_status CHECK (status IN (
    'Menunggu Dititipkan', 'Reservasi Batal', 'Tersedia', 'Dalam Klaim',
    'Pengambilan Terkunci', 'Sudah Diambil', 'Perlu Verifikasi',
    'Diserahkan ke Kampus', 'Dikembalikan'
  ))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS item_photos (
  id_foto INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_item INT UNSIGNED NOT NULL,
  path_foto VARCHAR(255) NOT NULL,
  urutan INT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (id_foto),
  UNIQUE KEY uq_item_photos_order (id_item, urutan),
  CONSTRAINT fk_item_photos_item
    FOREIGN KEY (id_item) REFERENCES items (id_item) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS deposits (
  id_deposit INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_item INT UNSIGNED NOT NULL,
  id_locker INT UNSIGNED NOT NULL,
  reserved_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  reserve_expires_at DATETIME NOT NULL,
  deposited_at DATETIME NULL,
  picked_up_at DATETIME NULL,
  status VARCHAR(15) NOT NULL DEFAULT 'RESERVED',
  PRIMARY KEY (id_deposit),
  KEY idx_deposits_locker_status (id_locker, status),
  CONSTRAINT fk_deposits_item
    FOREIGN KEY (id_item) REFERENCES items (id_item),
  CONSTRAINT fk_deposits_locker
    FOREIGN KEY (id_locker) REFERENCES lockers (id_locker),
  CONSTRAINT chk_deposits_status CHECK (status IN (
    'RESERVED', 'DEPOSITED', 'CANCELLED', 'PICKED_UP', 'VERIFY'
  ))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS claims (
  id_claim INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_item INT UNSIGNED NOT NULL,
  nim_pengklaim VARCHAR(15) NOT NULL,
  detail_claim TEXT NOT NULL,
  failed_count INT UNSIGNED NOT NULL DEFAULT 0,
  status VARCHAR(15) NOT NULL DEFAULT 'PENDING',
  claimed_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  verified_at DATETIME NULL,
  PRIMARY KEY (id_claim),
  KEY idx_claims_item_status (id_item, status),
  KEY idx_claims_pengklaim (nim_pengklaim),
  CONSTRAINT fk_claims_item
    FOREIGN KEY (id_item) REFERENCES items (id_item),
  CONSTRAINT fk_claims_pengklaim
    FOREIGN KEY (nim_pengklaim) REFERENCES users (nim),
  CONSTRAINT chk_claims_status CHECK (status IN (
    'PENDING', 'APPROVED', 'REJECTED', 'LOCKED', 'CANCELLED'
  ))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS access_codes (
  id_code INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_deposit INT UNSIGNED NULL,
  id_claim INT UNSIGNED NULL,
  tipe VARCHAR(10) NOT NULL,
  code_hash VARCHAR(255) NOT NULL,
  expires_at DATETIME NOT NULL,
  used_at DATETIME NULL,
  failed_count INT UNSIGNED NOT NULL DEFAULT 0,
  status VARCHAR(15) NOT NULL DEFAULT 'ACTIVE',
  PRIMARY KEY (id_code),
  KEY idx_access_codes_deposit (id_deposit),
  KEY idx_access_codes_claim (id_claim),
  CONSTRAINT fk_access_codes_deposit
    FOREIGN KEY (id_deposit) REFERENCES deposits (id_deposit),
  CONSTRAINT fk_access_codes_claim
    FOREIGN KEY (id_claim) REFERENCES claims (id_claim),
  CONSTRAINT chk_access_codes_source CHECK (
    (tipe = 'DEPOSIT' AND id_deposit IS NOT NULL AND id_claim IS NULL) OR
    (tipe = 'PICKUP' AND id_claim IS NOT NULL AND id_deposit IS NULL)
  ),
  CONSTRAINT chk_access_codes_status CHECK (status IN (
    'ACTIVE', 'USED', 'EXPIRED', 'LOCKED', 'CANCELLED'
  ))
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS locker_event_logs (
  id_log INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_locker INT UNSIGNED NOT NULL,
  id_deposit INT UNSIGNED NULL,
  tipe_event VARCHAR(20) NOT NULL,
  waktu DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  keterangan VARCHAR(255) NULL,
  PRIMARY KEY (id_log),
  KEY idx_locker_event_logs_locker_waktu (id_locker, waktu),
  KEY idx_locker_event_logs_deposit (id_deposit),
  CONSTRAINT fk_locker_event_logs_locker
    FOREIGN KEY (id_locker) REFERENCES lockers (id_locker),
  CONSTRAINT fk_locker_event_logs_deposit
    FOREIGN KEY (id_deposit) REFERENCES deposits (id_deposit)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS lost_reports (
  id_laporan INT UNSIGNED NOT NULL AUTO_INCREMENT,
  nim_pelapor VARCHAR(15) NOT NULL,
  id_kategori INT UNSIGNED NOT NULL,
  id_lokasi INT UNSIGNED NOT NULL,
  nama_barang VARCHAR(100) NOT NULL,
  ciri_ciri TEXT NULL,
  tanggal_hilang DATE NOT NULL,
  status VARCHAR(15) NOT NULL DEFAULT 'OPEN',
  PRIMARY KEY (id_laporan),
  KEY idx_lost_reports_search (status, id_kategori, id_lokasi, tanggal_hilang),
  CONSTRAINT fk_lost_reports_pelapor
    FOREIGN KEY (nim_pelapor) REFERENCES users (nim),
  CONSTRAINT fk_lost_reports_kategori
    FOREIGN KEY (id_kategori) REFERENCES categories (id_kategori),
  CONSTRAINT fk_lost_reports_lokasi
    FOREIGN KEY (id_lokasi) REFERENCES locations (id_lokasi),
  CONSTRAINT chk_lost_reports_status CHECK (status IN ('OPEN', 'MATCHED', 'CLOSED'))
) ENGINE=InnoDB;

-- Reference data used by the terminal application. Existing rows are left unchanged.
INSERT IGNORE INTO categories (nama_kategori) VALUES
  ('Aksesori'),
  ('Dokumen'),
  ('Dompet'),
  ('Elektronik'),
  ('Kunci'),
  ('Pakaian');

INSERT IGNORE INTO lockers (nomor_loker, lokasi_unit) VALUES
  (1, 'Loker 3x3'),
  (2, 'Loker 3x3'),
  (3, 'Loker 3x3'),
  (4, 'Loker 3x3'),
  (5, 'Loker 3x3'),
  (6, 'Loker 3x3'),
  (7, 'Loker 3x3'),
  (8, 'Loker 3x3'),
  (9, 'Loker 3x3');
