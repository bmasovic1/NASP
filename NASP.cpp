#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>

 #pragma comment(linker, "/STACK:4000000000")
namespace fs = std::filesystem;
using namespace std;
using namespace std::chrono;

// --- QUICKSORT ---

int particija_det(vector<double>& arr, int low, int high) {
    double pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) { i++; swap(arr[i], arr[j]); }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

void quicksort_det(vector<double>& arr, int low, int high) {
    if (low < high) {
        int pi = particija_det(arr, low, high);
        quicksort_det(arr, low, pi - 1);
        quicksort_det(arr, pi + 1, high);
    }
}

int particija_rand(vector<double>& arr, int low, int high) {
    static mt19937 rng(time(0));
    uniform_int_distribution<int> dist(low, high);
    int random_idx = dist(rng);
    swap(arr[random_idx], arr[high]);
    return particija_det(arr, low, high);
}

void quicksort_rand(vector<double>& arr, int low, int high) {
    if (low < high) {
        int pi = particija_rand(arr, low, high);
        quicksort_rand(arr, low, pi - 1);
        quicksort_rand(arr, pi + 1, high);
    }
}

// --- SHELLSORT ---

void shellsort_std(vector<double>& arr) {
    int n = arr.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            double temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) arr[j] = arr[j - gap];
            arr[j] = temp;
        }
    }
}

void shellsort_rand(vector<double>& arr) {
    int n = arr.size();
    static mt19937 rng(time(0));
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = 0; i < n - gap; i++) {
            if (rng() % 2 == 0) { if (arr[i] > arr[i + gap]) swap(arr[i], arr[i + gap]); }
        }
        for (int i = gap; i < n; i++) {
            double temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) arr[j] = arr[j - gap];
            arr[j] = temp;
        }
    }
}

// --- BUCKET SORT ---

void bucket_sort_std(vector<double>& arr) {
    int n = arr.size();
    if (n <= 1) return;
    double min_v = arr[0], max_v = arr[0];
    for (double x : arr) { min_v = min(min_v, x); max_v = max(max_v, x); }
    if (min_v == max_v) return;

    int num_buckets = n / 10;
    if (num_buckets < 1) num_buckets = 1;
    vector<vector<double>> buckets(num_buckets);
    double range = max_v - min_v;

    for (double x : arr) {
        int idx = (int)((x - min_v) / range * (num_buckets - 1));
        buckets[idx].push_back(x);
    }
    arr.clear();
    for (int i = 0; i < num_buckets; i++) {
        if (!buckets[i].empty()) {
            quicksort_rand(buckets[i], 0, buckets[i].size() - 1);
            for (double val : buckets[i]) arr.push_back(val);
        }
    }
}

void bucket_sort_rand(vector<double>& arr) {
    int n = arr.size();
    if (n <= 100) { bucket_sort_std(arr); return; }

    int sample_size = sqrt(n);
    vector<double> sample;
    static mt19937 rng(time(0));
    for (int i = 0; i < sample_size; i++) sample.push_back(arr[rng() % n]);
    sort(sample.begin(), sample.end());

    int num_buckets = 10;
    vector<double> splitters;
    for (int i = 1; i < num_buckets; i++) splitters.push_back(sample[i * sample_size / num_buckets]);

    vector<vector<double>> buckets(num_buckets);
    for (double x : arr) {
        bool placed = false;
        for (int i = 0; i < splitters.size(); i++) {
            if (x < splitters[i]) { buckets[i].push_back(x); placed = true; break; }
        }
        if (!placed) buckets[num_buckets - 1].push_back(x);
    }
    arr.clear();
    for (int i = 0; i < num_buckets; i++) {
        if (!buckets[i].empty()) {
            quicksort_rand(buckets[i], 0, buckets[i].size() - 1);
            for (double val : buckets[i]) arr.push_back(val);
        }
    }
}

// --- POMOCNE FUNKCIJE ---

vector<double> ucitaj_podatke(string putanja) {
    vector<double> rezultati;
    ifstream fajl(putanja);
    if (!fajl.is_open()) return rezultati;
    string sadrzaj;
    stringstream ss_buf;
    ss_buf << fajl.rdbuf();
    sadrzaj = ss_buf.str();
    replace(sadrzaj.begin(), sadrzaj.end(), ',', ' ');
    stringstream ss(sadrzaj);
    double broj;
    while (ss >> broj) rezultati.push_back(broj);
    return rezultati;
}

void upisi_u_csv(ofstream& fajl, string alg, string ver, string dist, string dat, int n, long long t) {
    fajl << alg << "," << ver << "," << dist << "," << dat << "," << n << "," << t << "ms\n";
}

int main() {

    string path = "Testiranje"; 

    if (!fs::exists(path)) { 
        cout << "GRESKA: Putanja ne postoji!" << endl; 
        return 1; 
    }

    ofstream csv_fajl("rezultati_benchmarka.csv");
    csv_fajl << "Algoritam,Verzija,Distribucija,Datoteka,N,Vrijeme_ms\n";

    vector<fs::path> sve_datoteke;
    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) sve_datoteke.push_back(entry.path());
    }
    sort(sve_datoteke.begin(), sve_datoteke.end());

    int broj_pokretanja = 5; 
    cout << "Testiranje pokrenuto. Svaki algoritam se izvrsava " << broj_pokretanja << " puta radi prosjeka." << endl;

    for (const auto& f_path : sve_datoteke) {

        vector<double> originalni_podaci = ucitaj_podatke(f_path.string());

        if (originalni_podaci.empty()) 
            continue;

        string datoteka_ime = f_path.filename().string();
        string distribucija = f_path.parent_path().filename().string();
        int N = originalni_podaci.size();

        cout << "Obradjujem: " << datoteka_ime << " (N=" << N << ")" << endl;

        auto benchmark = [&](string alg, string ver, auto funkcija) {

            long long ukupno_vrijeme = 0;

            for (int i = 0; i < broj_pokretanja; i++) {
                vector<double> kopija = originalni_podaci; 
                auto start = high_resolution_clock::now();
                funkcija(kopija);
                auto stop = high_resolution_clock::now();
                ukupno_vrijeme += duration_cast<milliseconds>(stop - start).count();
            }

            double prosjek = (double)ukupno_vrijeme / broj_pokretanja;
            csv_fajl << alg << "," << ver << "," << distribucija << "," << datoteka_ime << "," << N << "," << prosjek << "ms\n";
            csv_fajl.flush();

            };

        benchmark("Quick", "Random", [&](vector<double>& a) { quicksort_rand(a, 0, a.size() - 1); });
        benchmark("Quick", "Det", [&](vector<double>& a) { quicksort_det(a, 0, a.size() - 1); });
        benchmark("Shell", "Random", shellsort_rand);
        benchmark("Shell", "Std", shellsort_std);
        benchmark("Bucket", "Random", bucket_sort_rand);
        benchmark("Bucket", "Std", bucket_sort_std);
    }

    csv_fajl.close();
    cout << "\nTestiranje zavrseno! Rezultati su upisani u 'rezultati_benchmarka.csv'." << endl;
    return 0;
}