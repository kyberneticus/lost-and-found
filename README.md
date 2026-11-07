# Lost and Found Management System

Kerangka proyek C++ terminal untuk tim mahasiswa semester 1. Program akan mencatat barang temuan, laporan kehilangan, dan klaim barang. Kerangka ini sengaja berisi deklarasi data, kontrak fungsi, stub aman, dan TODO kecil agar setiap anggota dapat mengisi logika bisnis secara bertahap.

## Build dan jalankan

**Patokan utama:** seluruh program berada di `src/main.cpp`; kompilasi file itu dengan perintah berikut.

Install the MariaDB Connector/C development package, migrate the schema, then provide database credentials through the environment:

```bash
sudo dnf install mariadb-connector-c-devel
mariadb -u <user> -p < database.sql

export DB_HOST=127.0.0.1
export DB_PORT=3306
export DB_NAME=lostfound
export DB_USER=<user>
export DB_PASSWORD=<password>

g++ -std=c++11 -Wall -Wextra src/main.cpp $(mariadb_config --cflags --libs) -o lostfound
./lostfound
```

Pilih `3` pada menu tamu untuk keluar. Setelah TODO diisi, jalankan ulang skenario di `tests/manual_tests.md`.

## Struktur kode

Semua kode ada dalam satu file: `src/main.cpp`. Setiap pemilik bekerja hanya pada bagian yang diberi banner namanya; library dan `using namespace std;` hanya ada sekali di bagian paling atas.

## Kepemilikan bagian

| Anggota | Bagian |
| --- | --- |
| A | data, string_utils, id_utils, storage |
| B | auth, auth_screen |
| C | date_utils, items, item_screen |
| D | input_helpers, claims, claim_screen, main() |

## Aturan Git

1. Jalankan `git pull --rebase` sebelum mulai bekerja.
2. Buat satu branch per fitur, misalnya `feature/auth-register`.
3. Hanya edit bagian milik sendiri; jika harus mengubah bagian anggota lain, beri tahu pemiliknya lebih dahulu.
4. Jangan memformat ulang seluruh `src/main.cpp`.
5. Buka PR kecil untuk pekerjaan yang siap diuji.
6. Review silang berputar: **A → B → C → D → A**.
7. Jangan merge langsung ke branch utama tanpa review.

## Disederhanakan dengan sengaja

- Fase 1 **tidak memakai** `.h`, `struct`, `vector`, class, pointer, referensi, atau template. Semua data ada pada array paralel global.
- Semua bagian sengaja digabung dalam satu `main.cpp`; banner kepemilikan menjaga pembagian kerja tetap jelas.
- Password dan kode akses disimpan sebagai hash SHA-256; aplikasi tetap hanya cocok untuk latihan, bukan produksi.
- Akun yang terkunci tidak memiliki jalur buka dari menu; satu-satunya cara saat ini adalah mengedit data.
- Penyimpanan memakai MariaDB/MySQL melalui `loadAll`/`saveAll`; kredensial hanya dibaca dari variabel `DB_*` saat runtime.
- Tidak ada penghapusan data; indeks array tidak bergeser. Admin demo (`admin` / `admin123`) saat ini hanya memiliki menu logout sampai modul loker dan anomali ditambahkan.

Baca `docs/data-model.md` sebelum mengubah array dan `docs/TASKS.md` sebelum mulai mengisi TODO.

azhri hytam
mudho