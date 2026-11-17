#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// --- Configurable Thresholds (easy to adjust live during review) ---
const double SAFETY_BUFFER_PCT = 0.20;   // 20% buffer (village restock trips are less frequent)
const double STOCKOUT_EXTRA_UNITS = 2.0; // Extra units/kg suggested if an item stocked out
const double LOW_STOCK_THRESHOLD = 3.0;  // Flag items ending with <= this much stock
const int RUSH_WINDOW_MINUTES = 30;      // Bucket size for identifying peak rush window
// ------------------------------------------------------------------

struct ItemStats {
    double units_sold = 0.0;
    double revenue = 0.0;
    int tx_count = 0;
    int stockouts = 0;
    double min_stock_left = -1.0;
};

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string to_lower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}

vector<string> split_csv_line(const string& line) {
    vector<string> cols;
    stringstream ss(line);
    string cell;
    while (getline(ss, cell, ',')) {
        cols.push_back(trim(cell));
    }
    if (!line.empty() && line.back() == ',') {
        cols.push_back("");
    }
    return cols;
}

bool parse_hhmm(const string& ts_raw, int& out_minutes) {
    string time_part = ts_raw;
    size_t space_pos = ts_raw.find(' ');
    if (space_pos != string::npos) {
        time_part = trim(ts_raw.substr(space_pos + 1));
    }
    if (time_part.size() != 5 || time_part[2] != ':') return false;
    try {
        size_t p1 = 0, p2 = 0;
        int hh = stoi(time_part.substr(0, 2), &p1);
        int mm = stoi(time_part.substr(3, 2), &p2);
        if (p1 != 2 || p2 != 2) return false;
        if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return false;
        out_minutes = hh * 60 + mm;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_double_strict(const string& s, double& out_val) {
    if (s.empty()) return false;
    try {
        size_t pos = 0;
        double val = stod(s, &pos);
        if (pos != s.size()) return false;
        out_val = val;
        return true;
    } catch (...) {
        return false;
    }
}

string format_time(int total_minutes) {
    total_minutes = ((total_minutes % 1440) + 1440) % 1440;
    int hh = total_minutes / 60;
    int mm = total_minutes % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << hh << ":" << setw(2) << setfill('0') << mm;
    return oss.str();
}

int main(int argc, char* argv[]) {
    string csv_path = (argc > 1) ? argv[1] : "data.csv";
    ifstream file(csv_path);
    if (!file.is_open()) {
        cerr << "Error: File not found -> " << csv_path << "\n";
        return 1;
    }

    string header_line;
    if (!getline(file, header_line)) {
        cerr << "Error: CSV file is empty or missing headers.\n";
        return 1;
    }

    map<string, ItemStats> stats;
    map<int, int> window_counts;
    vector<string> bad_rows;
    int valid_rows = 0;

    string line;
    int line_num = 1;
    while (getline(file, line)) {
        line_num++;
        if (trim(line).empty()) continue;

        vector<string> cols = split_csv_line(line);
        if (cols.size() < 6) {
            bad_rows.push_back("Line " + to_string(line_num) + ": expected 6 columns, got " + to_string(cols.size()));
            continue;
        }

        int minutes = 0;
        if (!parse_hhmm(cols[0], minutes)) {
            bad_rows.push_back("Line " + to_string(line_num) + ": invalid timestamp '" + cols[0] + "'");
            continue;
        }

        string item = to_lower(cols[1]);
        if (item.empty()) {
            bad_rows.push_back("Line " + to_string(line_num) + ": empty item name");
            continue;
        }

        double qty_sold = 0.0;
        if (!parse_double_strict(cols[2], qty_sold) || qty_sold < 0) {
            bad_rows.push_back("Line " + to_string(line_num) + ": invalid qty_sold '" + cols[2] + "'");
            continue;
        }

        double price = 0.0;
        if (!parse_double_strict(cols[3], price) || price < 0) {
            bad_rows.push_back("Line " + to_string(line_num) + ": invalid unit_price_inr '" + cols[3] + "'");
            continue;
        }

        double stock_left = 0.0;
        if (!parse_double_strict(cols[4], stock_left) || stock_left < 0) {
            bad_rows.push_back("Line " + to_string(line_num) + ": invalid stock_left_after '" + cols[4] + "'");
            continue;
        }

        string flag = to_lower(cols[5]);
        bool stockout = false;
        if (flag == "yes" || flag == "y" || flag == "1" || flag == "true") {
            stockout = true;
        } else if (flag == "no" || flag == "n" || flag == "0" || flag == "false") {
            stockout = false;
        } else {
            bad_rows.push_back("Line " + to_string(line_num) + ": invalid stockout_flag '" + cols[5] + "'");
            continue;
        }

        valid_rows++;
        ItemStats& s = stats[item];
        s.units_sold += qty_sold;
        s.revenue += qty_sold * price;
        s.tx_count++;
        if (stockout) s.stockouts++;
        if (s.min_stock_left < 0 || stock_left < s.min_stock_left) {
            s.min_stock_left = stock_left;
        }

        int bucket_start = (minutes / RUSH_WINDOW_MINUTES) * RUSH_WINDOW_MINUTES;
        window_counts[bucket_start]++;
    }

    cout << "================================================================\n";
    cout << "VILLAGE GENERAL STORE REORDER & RUSH HELPER (C++)\n";
    cout << "================================================================\n";
    cout << "Valid rows processed : " << valid_rows << "\n";
    cout << "Bad rows skipped     : " << bad_rows.size() << "\n";
    cout << "Settings             : buffer=" << (int)(SAFETY_BUFFER_PCT * 100)
         << "%, stockout_bump=+" << STOCKOUT_EXTRA_UNITS
         << ", low_stock<=" << LOW_STOCK_THRESHOLD << "\n";

    if (!bad_rows.empty()) {
        cout << "\n[Skipped Bad Rows]\n";
        for (const string& err : bad_rows) {
            cout << "  - " << err << "\n";
        }
    }

    if (valid_rows == 0) {
        cout << "\nNo valid rows to analyze.\n";
        return 1;
    }

    if (!window_counts.empty()) {
        int peak_start = 0, peak_tx = -1;
        for (const auto& kv : window_counts) {
            if (kv.second > peak_tx) {
                peak_tx = kv.second;
                peak_start = kv.first;
            }
        }
        cout << "\nPeak " << RUSH_WINDOW_MINUTES << "-Min Rush Window: "
             << format_time(peak_start) << " - " << format_time(peak_start + RUSH_WINDOW_MINUTES)
             << " (" << peak_tx << " transactions)\n";
    }

    vector<pair<string, ItemStats>> sorted_items(stats.begin(), stats.end());
    sort(sorted_items.begin(), sorted_items.end(), [](const auto& a, const auto& b) {
        if (a.second.units_sold != b.second.units_sold)
            return a.second.units_sold > b.second.units_sold;
        return a.first < b.first;
    });

    cout << "\n[Suggested Next Restock Order]\n";
    cout << left << setw(18) << "Item"
         << right << setw(7) << "Sold"
         << setw(9) << "Rev(Rs)"
         << setw(10) << "Stockouts"
         << setw(9) << "MinLeft"
         << setw(10) << "OrderQty" << "  Note\n";
    cout << "----------------------------------------------------------------\n";

    double total_rev = 0.0;
    for (const auto& kv : sorted_items) {
        const string& item = kv.first;
        const ItemStats& s = kv.second;
        total_rev += s.revenue;

        double base_with_buffer = ceil(s.units_sold * (1.0 + SAFETY_BUFFER_PCT));
        double suggested = max(s.units_sold, base_with_buffer);
        if (s.stockouts > 0) {
            suggested += STOCKOUT_EXTRA_UNITS * s.stockouts;
        }

        string note = "";
        if (s.stockouts > 0) note += "STOCKOUT";
        if (s.min_stock_left >= 0 && s.min_stock_left <= LOW_STOCK_THRESHOLD) {
            if (!note.empty()) note += ", ";
            note += "LOW";
        }
        if (note.empty()) note = "OK";

        cout << left << setw(18) << item
             << right << setw(7) << fixed << setprecision(1) << s.units_sold
             << setw(9) << fixed << setprecision(1) << s.revenue
             << setw(10) << s.stockouts
             << setw(9) << fixed << setprecision(1) << s.min_stock_left
             << setw(10) << fixed << setprecision(1) << suggested << "  " << note << "\n";
    }

    cout << "----------------------------------------------------------------\n";
    cout << "Total Observed Revenue: Rs " << fixed << setprecision(2) << total_rev << "\n";
    return 0;
}