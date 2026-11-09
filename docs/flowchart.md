---
config:
  theme: base
  flowchart:
    curve: stepAfter
---

flowchart TD

    %% ==========================
    %% MAIN: MENU UTAMA, REGISTER, LOGIN
    %% ==========================

    subgraph MAIN["Menu Utama, Register, dan Login"]
        MULAI(["Mulai"]) --> GM{"Pilihan menu utama"}

        GM -->|"1 Register"| REG1[/"Input NIM, nama, jurusan, email, no HP, password"/]
        REG1 --> REG2{"Data valid dan NIM belum dipakai?"}

        REG2 -->|"Tidak"| REG3[/"Pesan gagal register"/]
        REG3 --> MZ

        REG2 -->|"Ya"| REG4["Simpan akun mahasiswa, password disimpan sebagai hash SHA-256"]
        REG4 --> REG5[/"Pesan: registrasi berhasil, silakan login"/]
        REG5 --> MZ

        GM -->|"2 Login"| LOG1[/"Input NIM dan password"/]
        LOG1 --> LOG2{"Hasil login?"}

        LOG2 -->|"Gagal"| LOG3[/"Pesan gagal; 3 kali salah mengunci akun 15 menit"/]
        LOG3 --> MZ

        LOG2 -->|"Berhasil"| ROLE{"Peran pengguna?"}
        ROLE -->|"Mahasiswa"| MENU{"Pilihan menu mahasiswa"}

        MENU -->|"4 Logout"| MZ
        GM -->|"3 Keluar"| MZ(["Selesai"])
    end

    ROLE -->|"Admin"| ADM1

    MENU -->|"1 Titip Barang"| T1
    MENU -->|"2 Cari dan Klaim"| KA
    MENU -->|"3 Riwayat Saya"| R1


    %% ==========================
    %% PROSES 1: TITIP BARANG
    %% ==========================

    subgraph TITIP["Proses 1: Titip Barang"]
        T1[/"Input nama, kategori, lokasi temuan, tanggal, path foto, detail rahasia; ciri-ciri opsional"/]
        T1 --> T2{"Data wajib terisi, tanggal valid, foto diisi?"}

        T2 -->|"Tidak"| T2A[/"Pesan gagal"/]
        T2A --> TZ

        T2 -->|"Ya"| T3["Cek ketersediaan loker"]
        T3 --> T4{"Ada loker READY?"}

        T4 -->|"Tidak"| T5[/"Pesan: loker penuh"/]
        T5 --> TZ

        T4 -->|"Ya"| T6["Reservasi loker 10 menit, buat kode titip, simpan barang berstatus Menunggu Dititipkan"]
        T6 --> T7[/"Tampilkan nomor loker dan kode titip"/]
        T7 --> T8[/"Input kode titip di keypad loker"/]
        T8 --> T9{"Hasil validasi kode?"}

        T9 -->|"Salah, percobaan tersisa; simpan event PIN_FAILURE"| T8

        T9 -->|"Salah 3 kali atau reservasi habis"| T10["Batalkan reservasi, kode hangus, loker kembali READY"]
        T10 --> TZ

        T9 -->|"Valid"| T11["Pintu loker terbuka"]
        T11 --> T12[/"Taruh barang dan tutup pintu, atau laporkan masalah loker"/]
        T12 --> T13{"Hasil konfirmasi?"}

        T13 -->|"Belum selesai"| T12

        T13 -->|"Laporkan masalah"| T14["Status Perlu Verifikasi; simpan event ANOMALY"]
        T14 --> TZ

        T13 -->|"Barang ditaruh, pintu tertutup"| T15["Status Tersedia, kode hangus, loker OCCUPIED; simpan event DROPOFF"]
        T15 --> T16[/"Tampilkan barang di daftar publik tanpa detail rahasia"/]
        T16 --> TZ(["Selesai"])
    end


    %% ==========================
    %% PROSES 2: CARI DAN KLAIM
    %% ==========================

    subgraph KLAIM["Proses 2: Cari, Klaim, dan Verifikasi"]
        KA{"Ada klaim disetujui yang belum diambil?"}

        KA -->|"Tidak"| K1
        KA -->|"Ya"| KB{"Pilihan?"}

        KB -->|"1 Cari dan klaim barang baru"| K1
        KB -->|"2 Ambil barang: pilih nomor dari daftar"| A1

        K1[/"Input kata kunci, kategori, lokasi"/]
        K1 --> K2["Cari barang berstatus Tersedia"]
        K2 --> K3{"Ada hasil?"}

        K3 -->|"Tidak"| K4{"Buat laporan kehilangan?"}

        K4 -->|"Ya"| K5[/"Input data barang hilang"/]
        K5 --> K6["Simpan laporan kehilangan"]
        K6 --> KZ

        K4 -->|"Tidak"| KZ

        K3 -->|"Ya"| K7[/"Tampilkan daftar: nama, kategori, lokasi, tanggal, ciri umum"/]
        K7 --> K8[/"Pilih nomor barang"/]
        K8 --> K9{"Nomor ada di hasil, barang Tersedia, bukan temuan sendiri, klaim belum diblokir?"}

        K9 -->|"Tidak"| K10[/"Pesan: barang tidak dapat diklaim, atau hubungi admin jika diblokir"/]
        K10 --> KZ

        K9 -->|"Ya"| K11[/"Input detail verifikasi non-publik"/]
        K11 --> K12{"Hasil pencocokan dengan data penemu?"}

        K12 -->|"Tidak cocok"| K14[/"Pesan: detail salah"/]
        K14 --> KZ

        K12 -->|"Tidak cocok untuk ke-3 kali"| K16["Klaim diblokir; tampil di menu admin"]
        K16 --> K17[/"Pesan: hubungi admin"/]
        K17 --> KZ

        K12 -->|"Cocok"| K13["Klaim disetujui, status barang Dalam Klaim, buat kode ambil berlaku 24 jam"]
        K13 --> K15[/"Tampilkan nomor loker dan kode ambil"/]

        KZ(["Selesai"])
    end

    K15 --> A1


    %% ==========================
    %% PROSES 3: AMBIL BARANG
    %% ==========================

    subgraph AMBIL["Proses 3: Ambil Barang"]
        A1[/"Input kode ambil di keypad loker; 0 untuk kembali"/]
        A1 --> A2{"Hasil validasi kode?"}

        A2 -->|"0 Kembali"| AZ

        A2 -->|"Salah, percobaan tersisa; simpan event PIN_FAILURE"| A1

        A2 -->|"Kedaluwarsa"| A3["Klaim dibatalkan, status barang kembali Tersedia"]
        A3 --> AZ

        A2 -->|"Salah 3 kali"| A4["Status Pengambilan Terkunci; simpan event PIN_FAILURE; tampil di menu admin"]
        A4 --> AZ

        A2 -->|"Valid"| A5["Pintu loker terbuka"]
        A5 --> A6[/"Ambil barang dan tutup pintu, atau laporkan masalah loker"/]
        A6 --> A7{"Hasil konfirmasi?"}

        A7 -->|"Belum selesai"| A6

        A7 -->|"Laporkan masalah"| A8["Status Perlu Verifikasi; simpan event ANOMALY"]
        A8 --> AZ

        A7 -->|"Barang diambil, pintu tertutup"| A9["Status Sudah Diambil, klaim APPROVED, loker READY; simpan event PICKUP"]
        A9 --> A10[/"Pesan: pengambilan selesai"/]
        A10 --> AZ(["Selesai"])
    end


    %% ==========================
    %% PROSES: RIWAYAT SAYA
    %% ==========================

    subgraph RIWAYAT["Riwayat Saya"]
        R1[/"Tampilkan laporan temuan saya dan riwayat klaim saya beserta status"/]
        R1 --> RZ(["Selesai"])
    end


    %% ==========================
    %% PROSES 4: ADMIN
    %% ==========================

    subgraph ADMIN["Proses 4: Admin"]
        ADM1{"Pilihan menu admin"}

        ADM1 -->|"1 Lihat anomali"| ADM2[/"Tampilkan barang Perlu Verifikasi, Pengambilan Terkunci, dan Tersedia lebih dari 30 hari"/]
        ADM2 --> ADZ

        ADM1 -->|"2 Tangani barang"| ADM3[/"Pilih nomor barang"/]
        ADM3 --> ADM4{"Status barang?"}

        ADM4 -->|"Pengambilan Terkunci"| ADM5{"Pemilik terverifikasi?"}

        ADM5 -->|"Ya"| ADM6["Terbitkan kode ambil baru 24 jam"]
        ADM6 --> ADZ

        ADM5 -->|"Tidak"| ADM7["Batalkan klaim, barang Tersedia"]
        ADM7 --> ADZ

        ADM4 -->|"Perlu Verifikasi"| ADM8{"Tindakan?"}

        ADM8 -->|"Reset"| ADM9["Barang Tersedia, data pengklaim dihapus"]
        ADM9 --> ADZ

        ADM8 -->|"Tandai loker rusak"| ADM10["Loker BROKEN"]
        ADM10 --> ADZ

        ADM4 -->|"Tersedia lebih dari 30 hari"| ADM11["Serahkan ke pengelola kampus"]
        ADM11 --> ADZ

        ADM4 -->|"Lainnya"| ADM12[/"Pesan: tidak perlu tindakan admin"/]
        ADM12 --> ADZ

        ADM1 -->|"3 Tangani klaim diblokir"| ADM13[/"Pilih klaim diblokir"/]
        ADM13 --> ADM14{"Pemilik terverifikasi?"}

        ADM14 -->|"Ya"| ADM15["Jika barang masih Tersedia: setujui klaim, terbitkan kode ambil baru, buka blokir"]
        ADM15 --> ADZ

        ADM14 -->|"Tidak"| ADM16["Tolak klaim, buka blokir"]
        ADM16 --> ADZ

        ADM1 -->|"4 Status loker"| ADM17[/"Tampilkan status 9 loker: READY, OCCUPIED, BROKEN"/]
        ADM17 --> ADZ

        ADM1 -->|"5 Logout"| ADZ(["Selesai"])
    end


    %% ==========================
    %% STYLING HITAM PUTIH
    %% ==========================

    classDef default fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px;
    linkStyle default stroke:#000000,stroke-width:1px;

    style MAIN fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
    style TITIP fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
    style KLAIM fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
    style AMBIL fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
    style RIWAYAT fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
    style ADMIN fill:#ffffff,stroke:#000000,color:#000000,stroke-width:1px
