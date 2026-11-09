# Model Data: Array Paralel

Semua data dideklarasikan **sekali** di `src/models/data.cpp`. Satu record adalah satu indeks yang sama di semua array dalam kelompoknya. Contoh: data barang temuan indeks `5` memakai `foundReporter[5]`, `foundName[5]`, `foundDate[5]`, sampai `foundStatus[5]`.

## Aturan emas

1. Jangan pernah mengubah satu array pasangan tanpa mengubah pasangan lain pada indeks yang sama.
2. Naikkan `userCount`, `foundCount`, `lostCount`, atau `claimCount` hanya setelah seluruh record selesai diisi.
3. MVP tidak menghapus data, jadi indeks tidak pernah bergeser.
4. Nomor yang dilihat pengguna adalah `indeks + 1`; gunakan `toDisplayNo` dan `toIndex`.
5. `searchResult` hanya tempat sementara berisi **indeks barang temuan**, bukan data barang baru.
6. `foundSecret` tidak boleh tampil di layar atau hasil pencarian.

## Pengguna

| Array             | Isi pada indeks `i`                          |
| ----------------- | -------------------------------------------- |
| `userName[i]`     | NIM/ID login (nama array lama dipertahankan) |
| `userPassword[i]` | password polos (latihan saja)                |
| `userPhone[i]`    | nomor telepon                                |
| `userFullName[i]` | nama lengkap mahasiswa                       |
| `userMajor[i]`    | jurusan mahasiswa                            |
| `userEmail[i]`    | email mahasiswa                              |
| `userRole[i]`     | peran `Mahasiswa` atau `Admin`               |
| `userFailed[i]`   | jumlah login salah berturut-turut            |
| `userLocked[i]`   | status terkunci                              |

Record pengguna valid berada pada indeks `0` sampai `userCount - 1`.

## Barang temuan

| Array                       | Isi pada indeks `i`                                                                                                                                                            |
| --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `foundReporter[i]`          | indeks pengguna pelapor/penemu                                                                                                                                                 |
| `foundName[i]`              | nama barang                                                                                                                                                                    |
| `foundCategory[i]`          | kategori                                                                                                                                                                       |
| `foundLocation[i]`          | lokasi ditemukan                                                                                                                                                               |
| `foundDate[i]`              | tanggal `YYYY-MM-DD`                                                                                                                                                           |
| `foundDescription[i]`       | deskripsi umum                                                                                                                                                                 |
| `foundSecret[i]`            | detail untuk verifikasi klaim, rahasia                                                                                                                                         |
| `foundStatus[i]`            | status, termasuk `Menunggu Dititipkan`, `Reservasi Batal`, `Tersedia`, `Dalam Klaim`, `Pengambilan Terkunci`, `Sudah Diambil`, `Perlu Verifikasi`, atau `Diserahkan ke Kampus` |
| `foundPhoto[i]`             | path foto bukti barang                                                                                                                                                         |
| `foundLocker[i]`            | indeks loker 0–8, atau `-1` bila tidak memakai loker                                                                                                                           |
| `foundDepositCode[i]`       | kode titip aktif, bila ada                                                                                                                                                     |
| `foundReservationExpiry[i]` | waktu kedaluwarsa reservasi sebagai Unix timestamp                                                                                                                             |
| `foundClaimant[i]`          | indeks pengguna pengklaim, atau `-1`                                                                                                                                           |
| `foundPickupCode[i]`        | kode ambil aktif, bila ada                                                                                                                                                     |
| `foundPickupExpiry[i]`      | waktu kedaluwarsa kode ambil sebagai Unix timestamp                                                                                                                            |
| `foundPickupFailed[i]`      | jumlah percobaan kode ambil salah                                                                                                                                              |

## Laporan hilang

| Array                | Isi pada indeks `i`     |
| -------------------- | ----------------------- |
| `lostReporter[i]`    | indeks pengguna pelapor |
| `lostName[i]`        | nama barang             |
| `lostCategory[i]`    | kategori                |
| `lostLocation[i]`    | lokasi terakhir         |
| `lostDate[i]`        | tanggal `YYYY-MM-DD`    |
| `lostDescription[i]` | deskripsi umum          |

## Percobaan klaim dan hasil pencarian

| Array             | Isi pada indeks `i`                            |
| ----------------- | ---------------------------------------------- |
| `claimItem[i]`    | indeks barang temuan                           |
| `claimUser[i]`    | indeks pengguna pengklaim                      |
| `claimFailed[i]`  | jawaban detail rahasia yang salah              |
| `searchResult[i]` | indeks barang temuan yang cocok pencarian/list |

`claimItem`, `claimUser`, dan `claimFailed` adalah satu record. Satu pasangan item-pengguna harus mempunyai satu baris percobaan klaim.

## Penyimpanan database

`src/main.cpp` memuat dan menyimpan data melalui MariaDB/MySQL. Konfigurasi dibaca saat runtime dari `DB_HOST`, `DB_PORT`, `DB_NAME`, `DB_USER`, dan `DB_PASSWORD`; lihat `README.md` untuk perintah build. Struktur relasional berada di `database.sql` dan mengikuti `docs/erd.drawio` serta `docs/rat.drawio`.

- `users`, `items`, dan `lost_reports` menyimpan record utama.
- `categories` dan `locations` menormalkan kategori serta lokasi yang dipilih pengguna.
- `deposits`, `claims`, dan `access_codes` menyimpan alur loker tanpa menyimpan password atau kode akses dalam plaintext.
- `item_photos` menyimpan foto; `locker_event_logs` disediakan untuk audit loker.
