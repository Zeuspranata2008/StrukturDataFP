#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstdio>

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
    double saldoSistem = 0;
    int autoId = 1;
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

                bukuKas.erase(it);
                return true;
            }
        }
        return false;
    }

    bool eksporLaporanKePDF_HTML() {
        ofstream file("Laporan_Keuangan_MoneyMate.html");
        if (!file.is_open()) return false;

        file << "<!DOCTYPE html><html><head><title>Laporan Keuangan</title>";
        file << "<style>";
        file << "body { font-family: Arial, sans-serif; margin: 40px; color: #333; }";
        file << "h2 { text-align: center; border-bottom: 2px solid #333; padding-bottom: 10px; }";
        file << "table { width: 100%; border-collapse: collapse; margin-top: 20px; }";
        file << "th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }";
        file << "th { background-color: #f2f2f2; }";
        file << ".uang { text-align: right; }";
        file << ".saldo-akhir { font-weight: bold; font-size: 18px; margin-top: 20px; text-align: right; }";
        file << "</style></head><body>";

        file << "<h2>LAPORAN KEUANGAN - MONEYMATE</h2>";
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
                    cout << "        BUAT PIN KEAMANAN    \n";
                    cout << "=================================\n";
                    cout << "\n[SISTEM] Anda belum memiliki PIN.\n";
                    cout << "Masukkan 4 Digit PIN baru: ";
                    cin >> pinInput;

                    createPin(pinInput);

                    cout << "\n>> PIN tersimpan! Tekan ENTER untuk lanjut Login...";
                    cin.ignore(); cin.get();
                }

                system("cls");
                cout << "=================================\n";
                cout << "  SECURITY LOGIN - MONEYMATE  \n";
                cout << "=================================\n";

                int percobaan = 0;
                while (percobaan < 2) {
                    cout << "Masukkan PIN Keamanan: ";
                    cin >> pinInput;

                    if (verifikasiPIN(pinInput)) {
                        cout << "\n>> NAH SIP MONGGO MASUK\n";
                        return true;
                    }
                    else {
                        percobaan++;
                        cout << ">> SALAH WOI! (" << percobaan << "/2)\n";
                    }
                }

                cout << ">> TAK BLOKIR KI LHO. BALIK RA!\n";
                cin.ignore(); cin.get();
            }
            else if (menuAwal == 2) {
                system("cls");
                cout << "=================================\n";
                cout << "           RESET PIN         \n";
                cout << "=================================\n";
                if (hapusDataPIN()) {
                    cout << "\n[SISTEM] Sukses! Data PIN telah dihancurkan.\n";
                }
                else {
                    cout << "\n[SISTEM] Gagal! Tidak ada PIN yang terdeteksi untuk dihapus.\n";
                }
                cout << "\nTekan ENTER untuk kembali ke menu awal...";
                cin.ignore(); cin.get();
            }
            else if (menuAwal == 0) {
                cout << "Menutup program...\n";
                return false;
            }
            else {
                cout << "Pilihan tidak valid!\n";
                cin.ignore(); cin.get();
            }
        }
    }

    void renderInput() {
        int pilihanTipe;
        string tgl, kat, tipe;
        double nom;

        cout << "=================================\n";
        cout << "      1. INPUT TRANSAKSI BARU    \n";
        cout << "=================================\n";
        cout << "1. Pemasukan\n2. Pengeluaran\nPilih Tipe: "; cin >> pilihanTipe;
        tipe = (pilihanTipe == 1) ? "Pemasukan" : "Pengeluaran";

        cout << "Tanggal (DD-MM-YYYY) : "; cin >> tgl;

        if (pilihanTipe == 1) {
            int pilKat;
            cout << "\n--- Pilih Kategori Pemasukan ---\n";
            cout << "1. Gaji\n";
            cout << "2. Side Job\n";
            cout << "3. Orang Tua\n";
            cout << "4. Investasi\n";
            cout << "Pilih Kategori (1-4): "; cin >> pilKat;

            switch (pilKat) {
            case 1: kat = "Gaji"; break;
            case 2: kat = "Side Job"; break;
            case 3: kat = "Orang Tua"; break;
            case 4: kat = "Investasi"; break;
            default: kat = "Lainnya"; break;
            }
            cout << "--------------------------------\n";
        }
        else {
            cin.ignore();
            cout << "Kategori             : "; getline(cin, kat);
        }

        cout << "Nominal (Rp)         : "; cin >> nom;

        tambahDataTransaksi(tipe, tgl, kat, nom);
        cout << "\n>> Berhasil! Saldo diupdate ke sistem.\n";

        cout << ">> Tekan ENTER untuk kembali ke menu...";
        cin.ignore(); cin.get();
    }

    void renderUpdate() {
        int id;
        string katBaru;
        cout << "=================================\n";
        cout << "    2. UPDATE KATEGORI TRANSAKSI \n";
        cout << "=================================\n";
        cout << "Masukkan ID Transaksi yang ingin diubah: "; cin >> id;
        cout << "Ketik Kategori Baru: "; cin.ignore(); getline(cin, katBaru);

        if (prosesUpdateKategori(id, katBaru)) {
            cout << "\n>> Sukses! Kategori pada ID " << id << " berhasil diperbarui.\n";
        }
        else {
            cout << "\n>> Gagal! Transaksi dengan ID tersebut tidak ditemukan.\n";
        }

        cout << ">> Tekan ENTER untuk kembali ke menu...";
        cin.get();
    }

    void renderDelete() {
        int id;
        cout << "=================================\n";
        cout << "       3. HAPUS TRANSAKSI        \n";
        cout << "=================================\n";
        cout << "Masukkan ID Transaksi yang ingin dihapus: "; cin >> id;

        if (hapusDataTransaksi(id)) {
            cout << "\n>> Sukses dihapus! Saldo otomatis dikalkulasi ulang.\n";
        }
        else {
            cout << "\n>> Gagal! Transaksi dengan ID tersebut tidak ditemukan.\n";
        }

        cout << ">> Tekan ENTER untuk kembali ke menu...";
        cin.ignore(); cin.get();
    }

    void renderDetailTransaksi() {
        cout << "=======================================================================================\n";
        cout << "                       4. BUKU BESAR (DETAIL TRANSAKSI & SALDO)                        \n";
        cout << "=======================================================================================\n";

        cout << left << setw(4) << "ID" << " | "
            << setw(12) << "Tanggal" << " | "
            << setw(15) << "Kategori" << " | "
            << setw(15) << "Pemasukan" << " | "
            << setw(15) << "Pengeluaran" << " | "
            << "Saldo Berjalan\n";
        cout << "---------------------------------------------------------------------------------------\n";

        if (bukuKas.empty()) {
            cout << "Belum ada riwayat transaksi. Buku kas masih kosong.\n";
        }
        else {
            double runningSaldo = 0;
            for (const auto& t : bukuKas) {
                double masuk = (t.tipe == "Pemasukan") ? t.nominal : 0;
                double keluar = (t.tipe == "Pengeluaran") ? t.nominal : 0;
                runningSaldo += (masuk - keluar);

                cout << left << setw(4) << t.id << " | "
                    << setw(12) << t.tanggal << " | "
                    << setw(15) << t.kategori << " | "
                    << "Rp " << setw(12) << fixed << setprecision(0) << masuk << " | "
                    << "Rp " << setw(12) << keluar << " | "
                    << "Rp " << runningSaldo << "\n";
            }
        }
        cout << "---------------------------------------------------------------------------------------\n";
        cout << right << setw(70) << "SISA SALDO AKTUAL: Rp " << saldoSistem << "\n";
        cout << "=======================================================================================\n";

        cout << "\n>> Tekan ENTER untuk kembali ke menu...";
        cin.ignore(); cin.get();
    }

    void renderEkspor() {
        int pilihanFormat;
        cout << "=================================\n";
        cout << "       5. EKSPOR LAPORAN         \n";
        cout << "=================================\n";
        cout << "1. Ekspor ke Web/PDF (HTML)\n";
        cout << "2. Ekspor ke Excel (CSV)\n";
        cout << "Pilih format output: ";
        cin >> pilihanFormat;

        if (pilihanFormat == 1) {
            if (eksporLaporanKePDF_HTML()) {
                cout << "\n>> Sukses diekspor ke HTML!\n";
                cout << ">> Buka file 'Laporan_Keuangan_MoneyMate.html' di folder program.\n";
                cout << ">> (Tips: Buka di Chrome/Edge, lalu tekan CTRL+P untuk Save as PDF).\n";
            }
            else {
                cout << "\n>> Terjadi kesalahan saat mengekspor HTML!\n";
            }
        }
        else if (pilihanFormat == 2) {
            if (eksporLaporanCSV()) {
                cout << "\n>> Sukses diekspor ke format Excel!\n";
                cout << ">> Buka file 'Laporan_MoneyMate.csv' langsung pakai Microsoft Excel.\n";
            }
            else {
                cout << "\n>> Gagal mengekspor! Pastikan file Excel sedang DITUTUP sebelum di-ekspor.\n";
            }
        }
        else {
            cout << "\n>> Pilihan format tidak valid!\n";
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
            cout << " Saldo Aktual: Rp " << fixed << setprecision(0) << saldoSistem << "\n";
            cout << "====================================\n";
            cout << "1. Input Transaksi     (Create)\n";
            cout << "2. Update Kategori     (Update)\n";
            cout << "3. Hapus Transaksi     (Delete)\n";
            cout << "4. Detail & Cek Saldo  (Read)\n";
            cout << "5. Ekspor Cetak PDF/CSV(Print)\n";
            cout << "0. Keluar\n";
            cout << "Pilih Menu: ";
            cin >> pilihan;

            if (pilihan >= 1 && pilihan <= 5) {
                system("cls");
            }

            switch (pilihan) {
            case 1: renderInput(); break;
            case 2: renderUpdate(); break;
            case 3: renderDelete(); break;
            case 4: renderDetailTransaksi(); break;
            case 5: renderEkspor(); break;
            case 0:
                system("cls");
                cout << "Keluar sistem... Terima kasih!\n";
                break;
            default:
                cout << "Pilihan tidak valid. Tekan ENTER untuk coba lagi...\n";
                cin.ignore(); cin.get();
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