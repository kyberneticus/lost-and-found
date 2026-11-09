# Flowchart tests

Skrip Expect ini membagi `docs/flowchart.md` menurut alurnya. Setiap skrip membangun `build/lostfound-test`, memakai data unik berdasarkan PID, dan memeriksa keluaran penting.

| Skrip | Alur yang dicakup |
| --- | --- |
| `01-authentication.exp` | Registrasi, 3 gagal login mengunci akun, login, logout |
| `02-deposit.exp` | Reservasi loker, PIN salah, PIN valid, penitipan selesai |
| `03-claim-pickup.exp` | Cari barang, klaim, kembali dengan `0`, lanjutkan klaim aktif, PIN ambil salah, pengambilan selesai |
| `04-lost-report.exp` | Pencarian tanpa hasil dan laporan barang hilang |
| `05-history.exp` | Riwayat titipan mahasiswa |
| `06-admin.exp` | Anomali loker, klaim diblokir, persetujuan admin, reset klaim aktif, status loker |

## Menjalankan

Gunakan **database uji khusus** yang sudah diinisialisasi dengan skema `database.sql`. Skrip menambah akun, barang, klaim, dan laporan uji; data tersebut tidak dihapus otomatis.

```sh
export DB_HOST=...
export DB_PORT=3306
export DB_NAME=<nama_database_uji>
export DB_USER=...
export DB_PASSWORD=...
export ALLOW_MUTATING_TEST_DATABASE=1

expect test/01-authentication.exp
expect test/02-deposit.exp
expect test/03-claim-pickup.exp
expect test/04-lost-report.exp
expect test/05-history.exp
expect test/06-admin.exp
```

`database.sql` memakai schema `lostfound`; jika memakai nama lain, buat/import salinan skema itu ke database uji terlebih dahulu. `ALLOW_MUTATING_TEST_DATABASE=1` adalah pengaman agar skrip tidak sengaja dijalankan pada database non-uji.
