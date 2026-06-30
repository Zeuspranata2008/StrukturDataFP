#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <stack>
#include <queue>
#include <sstream>
#include <ctime>
#include <cctype>

using namespace std;

namespace DataLayer {
    struct Transaksi {
        int id;
        string tanggal;
        string tipe;
        string kategori;
        double nominal;
    };

    vector<Transaksi> bukuKas;
    stack<Transaksi> riwayatHapus;
    queue<Transaksi> antreanTransaksi;

    double saldoSistem = 0;
    int autoId = 1;
    string namaUser = "USER";
}

namespace LogicLayer {
    using namespace DataLayer;

    string pinSistem = "";

    bool isPinExist() {
        ifstream file("pin.txt");
        if (file >> pinSistem) {
            file.close();
            return true;
        }
        return false;
    }

    void createPin(string newPin) {
        ofstream file("pin.txt");
        file << newPin;
        pinSistem = newPin;
        file.close();
    }

    bool verifikasiPIN(string pinInput) {
        return pinInput == pinSistem;
    }

    bool hapusDataPIN() {
        if (remove("pin.txt") == 0) {
            pinSistem = "";
            return true;
        }
        return false;
    }

    void tambahDataTransaksi(string tipe, string tgl, string kat, double nominal) {
        Transaksi t = { autoId++, tgl, tipe, kat, nominal };
        bukuKas.push_back(t);

        if (tipe == "Pemasukan") saldoSistem += nominal;
        else saldoSistem -= nominal;
    }

    void tambahAntrean(string tipe, string tgl, string kat, double nominal) {
        Transaksi t = { autoId++, tgl, tipe, kat, nominal };
        antreanTransaksi.push(t);
    }

    int prosesSemuaAntrean() {
        int count = 0;
        while (!antreanTransaksi.empty()) {
            Transaksi t = antreanTransaksi.front();
            bukuKas.push_back(t);

            if (t.tipe == "Pemasukan") saldoSistem += t.nominal;
            else saldoSistem -= t.nominal;

            antreanTransaksi.pop();
            count++;
        }
        return count;
    }

    bool prosesUpdateKategori(int id, string katBaru) {
        for (auto& t : bukuKas) {
            if (t.id == id) {
                t.kategori = katBaru;
                return true;
            }
        }
        return false;
    }

    bool hapusDataTransaksi(int id) {
        for (auto it = bukuKas.begin(); it != bukuKas.end(); ++it) {
            if (it->id == id) {
                if (it->tipe == "Pemasukan") saldoSistem -= it->nominal;
                else saldoSistem += it->nominal;

                riwayatHapus.push(*it);

                bukuKas.erase(it);
                return true;
            }
        }
        return false;
    }

    bool undoHapusTransaksi() {
        if (riwayatHapus.empty()) return false;

        Transaksi t = riwayatHapus.top();
        riwayatHapus.pop();

        bukuKas.push_back(t);
        if (t.tipe == "Pemasukan") saldoSistem += t.nominal;
        else saldoSistem -= t.nominal;

        return true;
    }

    void urutkanBerdasarkanID() {
        sort(bukuKas.begin(), bukuKas.end(), [](const Transaksi& a, const Transaksi& b) {
            return a.id < b.id;
            });
    }

    void urutkanBerdasarkanNominal(bool asc = true) {
        if (asc) {
            sort(bukuKas.begin(), bukuKas.end(), [](const Transaksi& a, const Transaksi& b) {
                return a.nominal < b.nominal;
                });
        }
        else {
            sort(bukuKas.begin(), bukuKas.end(), [](const Transaksi& a, const Transaksi& b) {
                return a.nominal > b.nominal;
                });
        }
    }

    string terbilang(long long n) {
        string bilangan[] = { "", "Satu", "Dua", "Tiga", "Empat", "Lima", "Enam", "Tujuh", "Delapan", "Sembilan", "Sepuluh", "Sebelas" };
        if (n == 0) return "Nol";
        if (n < 12) return bilangan[n];
        if (n < 20) return terbilang(n - 10) + " Belas";
        if (n < 100) return terbilang(n / 10) + " Puluh " + (n % 10 != 0 ? terbilang(n % 10) : "");
        if (n < 200) return "Seratus " + (n - 100 != 0 ? terbilang(n - 100) : "");
        if (n < 1000) return terbilang(n / 100) + " Ratus " + (n % 100 != 0 ? terbilang(n % 100) : "");
        if (n < 2000) return "Seribu " + (n - 1000 != 0 ? terbilang(n - 1000) : "");
        if (n < 1000000) return terbilang(n / 1000) + " Ribu " + (n % 1000 != 0 ? terbilang(n % 1000) : "");
        if (n < 1000000000) return terbilang(n / 1000000) + " Juta " + (n % 1000000 != 0 ? terbilang(n % 1000000) : "");
        if (n < 1000000000000) return terbilang(n / 1000000000) + " Miliar " + (n % 1000000000 != 0 ? terbilang(n % 1000000000) : "");
        return "";
    }

    bool eksporLaporanKePDF_HTML() {
        ofstream file("Laporan_Keuangan_MoneyMate.html");
        if (!file.is_open()) return false;

        time_t now = time(0);
        tm ltm;
        localtime_s(&ltm, &now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &ltm);
        string waktuUpdate(buffer);

        file << "<!DOCTYPE html><html><head><title>Laporan Keuangan</title>";
        file << "<style>";
        file << "body { font-family: Arial, sans-serif; margin: 40px; color: #333; position: relative; }";
        file << ".header-info { position: absolute; top: -20px; left: 0; font-size: 14px; color: #333; line-height: 1.6; }";
        file << "h2 { text-align: center; border-bottom: 2px solid #333; padding-bottom: 10px; margin-top: 60px; }";
        file << "table { width: 100%; border-collapse: collapse; margin-top: 20px; }";
        file << "th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }";
        file << "th { background-color: #f2f2f2; }";
        file << ".uang { text-align: right; }";
        file << ".saldo-akhir { font-weight: bold; font-size: 18px; margin-top: 20px; text-align: right; }";
        file << ".terbilang { font-style: italic; font-size: 15px; text-align: right; color: #555; margin-top: 5px; }";
        file << "</style></head><body>";

        file << "<div class='header-info'>";
        file << "<strong>Nama Akun:</strong> " << namaUser << "<br>";
        file << "<strong>Terakhir Update:</strong> " << waktuUpdate;
        file << "</div>";

        file << "<h2>LAPORAN LABA RUGI & NERACA KAS - MONEYMATE</h2>";
        file << "<table><tr><th>ID</th><th>Tanggal</th><th>Kategori</th><th>Pemasukan</th><th>Pengeluaran</th><th>Sisa Saldo</th></tr>";

        double runningSaldo = 0;
        for (const auto& t : bukuKas) {
            double masuk = (t.tipe == "Pemasukan") ? t.nominal : 0;
            double keluar = (t.tipe == "Pengeluaran") ? t.nominal : 0;
            runningSaldo += (masuk - keluar);

            file << "<tr>";
            file << "<td>" << t.id << "</td>";
            file << "<td>" << t.tanggal << "</td>";
            file << "<td>" << t.kategori << "</td>";
            file << "<td class='uang'>Rp " << fixed << setprecision(0) << masuk << "</td>";
            file << "<td class='uang'>Rp " << keluar << "</td>";
            file << "<td class='uang'>Rp " << runningSaldo << "</td>";
            file << "</tr>";
        }
        file << "</table>";

        file << "<div class='saldo-akhir'>Total Saldo Akhir: Rp " << fixed << setprecision(0) << saldoSistem << "</div>";

        long long saldoBulat = (long long)saldoSistem;
        string hasilTerbilang = (saldoBulat == 0) ? "Nol Rupiah" : terbilang(saldoBulat) + " Rupiah";
        if (saldoBulat < 0) hasilTerbilang = "Minus " + terbilang(abs(saldoBulat)) + " Rupiah";

        file << "<div class='terbilang'>(Terbilang: " << hasilTerbilang << ")</div>";
        file << "</body></html>";

        file.close();
        return true;
    }

    bool eksporLaporanCSV() {
        ofstream file("Laporan_MoneyMate.csv");
        if (!file.is_open()) return false;

        file << "sep=;\n";
        file << "ID;Tanggal;Kategori;Pemasukan;Pengeluaran;Sisa Saldo\n";

        double runningSaldo = 0;
        for (const auto& t : bukuKas) {
            double masuk = (t.tipe == "Pemasukan") ? t.nominal : 0;
            double keluar = (t.tipe == "Pengeluaran") ? t.nominal : 0;
            runningSaldo += (masuk - keluar);

            file << t.id << ";"
                << t.tanggal << ";"
                << t.kategori << ";"
                << "Rp " << fixed << setprecision(0) << masuk << ";"
                << "Rp " << keluar << ";"
                << "Rp " << runningSaldo << "\n";
        }

        file << ";;;;\n";
        file << ";;;;Total Saldo Akhir:;Rp " << fixed << setprecision(0) << saldoSistem << "\n";

        file.close();
        return true;
    }
}

namespace UILayer {
    using namespace LogicLayer;
    using namespace DataLayer;

    bool renderHalamanAwal() {
        int menuAwal;
        string pinInput;
        while (true) {
            system("cls");
            cout << "=================================\n";
            cout << "          HALAMAN LOGIN      \n";
            cout << "=================================\n";
            cout << "1. Buat PIN / Masuk Sistem\n";
            cout << "2. Reset PIN (Hapus Data)\n";
            cout << "0. Keluar Program\n";
            cout << "Pilih menu: ";
            cin >> menuAwal;

            if (menuAwal == 1) {
                if (!isPinExist()) {
                    system("cls");
                    cout << "=================================\n";
                    cout << "        BUAT AKUN    \n";
                    cout << "=================================\n";
                    cout << "Masukkan Nama Akun Anda: ";
                    cin.ignore();
                    getline(cin, namaUser);
                    cout << "Masukkan 4 Digit PIN baru: ";
                    cin >> pinInput;
                    createPin(pinInput);
                    cout << "\n>> PIN tersimpan! Tekan ENTER untuk lanjut Login...";
                    cin.ignore(); cin.get();
                }

                system("cls");
                int percobaan = 0;
                while (percobaan < 2) {
                    cout << "Masukkan PIN Keamanan: ";
                    cin >> pinInput;
                    if (verifikasiPIN(pinInput)) return true;
                    percobaan++;
                    cout << ">> SALAH! (" << percobaan << "/2)\n";
                }
                return false;
            }
            else if (menuAwal == 2) {
                hapusDataPIN();
                cout << "\n[SISTEM] Data PIN dihapus. Tekan ENTER...";
                cin.ignore(); cin.get();
            }
            else if (menuAwal == 0) {
                return false;
            }
        }
    }

    void renderInput(bool isAntrean = false) {
        int pilihanTipe, pilKat;
        string tgl, kat, tipe;
        double nom;

        system("cls");
        cout << "=================================\n";
        cout << (isAntrean ? "    TAMBAH ANTREAN TRANSAKSI    \n" : "      INPUT TRANSAKSI BARU    \n");
        cout << "=================================\n";
        cout << "1. Pemasukan\n2. Pengeluaran\nPilih Tipe: "; cin >> pilihanTipe;
        tipe = (pilihanTipe == 1) ? "Pemasukan" : "Pengeluaran";
        cout << "Tanggal (DD-MM-YYYY) : "; cin >> tgl;

        if (pilihanTipe == 1) {
            cout << "\n--- Pilih Kategori Pemasukan ---\n";
            cout << "1. Gaji\n2. Side Job\n3. Diberi Orang Tua\n4. Investasi\n5. Lainnya\n";
            cout << "Pilih Kategori (1-5): "; cin >> pilKat;

            switch (pilKat) {
            case 1: kat = "Gaji"; break;
            case 2: kat = "Side Job"; break;
            case 3: kat = "Diberi Orang Tua"; break;
            case 4: kat = "Investasi"; break;
            default:
                cout << "Ketik Kategori Pemasukan : ";
                cin.ignore();
                getline(cin, kat);
                break;
            }
        }
        else {
            cout << "Kategori Pengeluaran : "; cin.ignore(); getline(cin, kat);
        }

        cout << "Nominal (Rp)         : "; cin >> nom;

        if (isAntrean) {
            tambahAntrean(tipe, tgl, kat, nom);
            cout << "\n>> Masuk Antrean! (Belum memotong/menambah saldo).\n";
        }
        else {
            tambahDataTransaksi(tipe, tgl, kat, nom);
            cout << "\n>> Berhasil! Saldo diupdate ke sistem.\n";
        }
        cout << ">> Tekan ENTER untuk kembali..."; cin.ignore(); cin.get();
    }

    void renderUpdate() {
        int id;
        string katBaru;
        system("cls");
        cout << "=================================\n";
        cout << "       UPDATE KATEGORI DATA      \n";
        cout << "=================================\n";
        cout << "Masukkan ID Transaksi yang akan diedit: ";
        cin >> id;
        cout << "Masukkan Kategori Baru: ";
        cin.ignore();
        getline(cin, katBaru);

        if (!katBaru.empty()) {
            katBaru[0] = toupper(katBaru[0]);
            for (size_t i = 1; i < katBaru.length(); i++) {
                katBaru[i] = tolower(katBaru[i]);
            }
        }

        if (prosesUpdateKategori(id, katBaru)) {
            cout << "\n>> Berhasil! Kategori telah diupdate menjadi: " << katBaru << "\n";
        }
        else {
            cout << "\n>> Gagal! ID tidak ditemukan.\n";
        }
        cout << ">> Tekan ENTER untuk kembali...";
        cin.get();
    }

    void renderSearch() {
        string keyword;
        system("cls");
        cout << "=================================\n";
        cout << "         CARI TRANSAKSI          \n";
        cout << "=================================\n";
        cout << "Masukkan Nama/Kategori: ";
        cin.ignore();
        getline(cin, keyword);

        cout << "\nHasil Pencarian:\n";
        cout << "----------------------------------------------------------------------\n";
        cout << left << setw(4) << "ID" << " | " << setw(12) << "Tanggal" << " | "
            << setw(20) << "Kategori" << " | " << "Nominal\n";
        cout << "----------------------------------------------------------------------\n";

        bool ditemukan = false;
        for (const auto& t : bukuKas) {
            if (t.kategori.find(keyword) != string::npos) {
                ditemukan = true;
                string tanda = (t.tipe == "Pemasukan") ? "+ Rp " : "- Rp ";
                cout << left << setw(4) << t.id << " | "
                    << setw(12) << t.tanggal << " | "
                    << setw(20) << t.kategori << " | "
                    << tanda << fixed << setprecision(0) << t.nominal << "\n";
            }
        }

        if (!ditemukan) cout << ">> Transaksi tidak ditemukan!\n";
        cout << "----------------------------------------------------------------------\n";
        cout << ">> Tekan ENTER untuk kembali...";
        cin.get();
    }

    void renderProsesAntrean() {
        system("cls");
        cout << "=================================\n";
        cout << "         PROSES ANTREAN          \n";
        cout << "=================================\n";
        int diproses = prosesSemuaAntrean();
        if (diproses > 0) {
            cout << ">> Berhasil memproses " << diproses << " transaksi dari antrean!\n";
            cout << ">> Saldo utama telah diperbarui.\n";
        }
        else {
            cout << ">> Antrean kosong. Tidak ada yang diproses.\n";
        }
        cout << ">> Tekan ENTER untuk kembali...";
        cin.ignore(); cin.get();
    }

    void renderHapusDanUndo() {
        int opsi;
        system("cls");
        cout << "=================================\n";
        cout << "        HAPUS & UNDO DATA        \n";
        cout << "=================================\n";
        cout << "1. Hapus Transaksi (Delete Saldo)\n";
        cout << "2. Batal Hapus / Undo (Stack)\n";
        cout << "Pilih: "; cin >> opsi;

        if (opsi == 1) {
            int id;
            cout << "Masukkan ID Transaksi: "; cin >> id;
            if (hapusDataTransaksi(id)) cout << "\n>> Berhasil dihapus!\n";
            else cout << "\n>> Gagal! ID tidak ditemukan.\n";
        }
        else if (opsi == 2) {
            if (undoHapusTransaksi()) cout << "\n>> UNDO SUKSES! Transaksi dikembalikan.\n";
            else cout << "\n>> UNDO GAGAL! Tidak ada riwayat hapus.\n";
        }
        cout << ">> Tekan ENTER untuk kembali...";
        cin.ignore(); cin.get();
    }

    void renderDetailTransaksi() {
        int opsiSort;
        system("cls");
        cout << "================================================================================================\n";
        cout << "                            BUKU BESAR (DETAIL TRANSAKSI & SALDO)                               \n";
        cout << "================================================================================================\n";
        cout << "Pengurutan Data:\n1. Sesuai Input Awal\n2. Nominal Terkecil ke Terbesar\n3. Nominal Terbesar ke Terkecil\nPilih (1-3): ";
        cin >> opsiSort;

        if (opsiSort == 1) urutkanBerdasarkanID();
        else if (opsiSort == 2) urutkanBerdasarkanNominal(true);
        else if (opsiSort == 3) urutkanBerdasarkanNominal(false);

        cout << "------------------------------------------------------------------------------------------------\n";

        cout << left << setw(4) << "ID" << " | " << setw(12) << "Tanggal" << " | "
            << setw(20) << "Kategori" << " | " << setw(18) << "Pemasukan" << " | "
            << "Pengeluaran\n";
        cout << "------------------------------------------------------------------------------------------------\n";

        double totalMasuk = 0, totalKeluar = 0;
        for (const auto& t : bukuKas) {
            double masuk = (t.tipe == "Pemasukan") ? t.nominal : 0;
            double keluar = (t.tipe == "Pengeluaran") ? t.nominal : 0;
            totalMasuk += masuk;
            totalKeluar += keluar;

            stringstream ssMasuk, ssKeluar;
            ssMasuk << "Rp " << fixed << setprecision(0) << masuk;
            ssKeluar << "Rp " << fixed << setprecision(0) << keluar;

            cout << left << setw(4) << t.id << " | "
                << setw(12) << t.tanggal << " | "
                << setw(20) << t.kategori << " | "
                << setw(18) << ssMasuk.str() << " | "
                << ssKeluar.str() << "\n";
        }
        cout << "------------------------------------------------------------------------------------------------\n";
        cout << right << setw(75) << "SISA SALDO AKTUAL: Rp " << saldoSistem << "\n";

        cout << right << setw(75) << "(Ada " << antreanTransaksi.size() << " transaksi di antrean)\n";
        cout << "================================================================================================\n";

        cout << "\n>> Tekan ENTER untuk kembali...";
        cin.ignore(); cin.get();
    }

    void renderEkspor() {
        int pilihanFormat;
        system("cls");
        cout << "=================================\n";
        cout << "        EKSPOR LAPORAN           \n";
        cout << "=================================\n";
        cout << "1. Ekspor ke Web/PDF\n";
        cout << "2. Ekspor ke Excel\n";
        cout << "Pilih format output: ";
        cin >> pilihanFormat;

        if (pilihanFormat == 1) {
            if (eksporLaporanKePDF_HTML()) {
                cout << "\n>> Sukses diekspor ke HTML!\n";
                cout << ">> Buka file 'Laporan_Keuangan_MoneyMate.html' di folder program.\n";
            }
            else {
                cout << "\n>> Terjadi kesalahan saat mengekspor HTML!\n";
            }
        }
        else if (pilihanFormat == 2) {
            if (eksporLaporanCSV()) {
                cout << "\n>> Sukses diekspor ke format Excel!\n";
            }
            else {
                cout << "\n>> Gagal mengekspor! Pastikan file Excel sedang DITUTUP.\n";
            }
        }
        cout << "\n>> Tekan ENTER untuk kembali ke menu...";
        cin.ignore(); cin.get();
    }

    void renderMenuUtama() {
        int pilihan;
        do {
            system("cls");
            cout << "====================================\n";
            cout << "         DASHBOARD MONEYMATE        \n";
            cout << " Hai, " << namaUser << "!\n";
            cout << " Saldo Aktual: Rp " << fixed << setprecision(0) << saldoSistem << "\n";
            cout << "====================================\n";
            cout << "1. Input Transaksi Langsung\n";
            cout << "2. Tambah ke Antrean\n";
            cout << "3. Proses Antrean\n";
            cout << "4. Edit / Update Kategori\n";
            cout << "5. Cari Transaksi\n";
            cout << "6. Hapus / Undo Data\n";
            cout << "7. Lihat Buku Besar\n";
            cout << "8. Ekspor Cetak PDF/CSV\n";
            cout << "0. Keluar\n";
            cout << "Pilih Menu: ";
            cin >> pilihan;

            switch (pilihan) {
            case 1: renderInput(false); break;
            case 2: renderInput(true); break;
            case 3: renderProsesAntrean(); break;
            case 4: renderUpdate(); break;
            case 5: renderSearch(); break;
            case 6: renderHapusDanUndo(); break;
            case 7: renderDetailTransaksi(); break;
            case 8: renderEkspor(); break;
            case 0: cout << "Keluar sistem...\n"; break;
            default: cout << "Pilihan tidak valid.\n"; cin.ignore(); cin.get();
            }
        } while (pilihan != 0);
    }
}

int main() {
    if (UILayer::renderHalamanAwal()) {
        UILayer::renderMenuUtama();
    }
    return 0;
}