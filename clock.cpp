#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <sstream>
#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std::chrono;
using Clock = std::chrono::system_clock;

std::atomic<bool> running{true};
std::atomic<bool> show_live_clock{false};
std::mutex alarm_mutex;
std::condition_variable alarm_cv;

struct Alarm {
    time_t trigger_time;
    std::string label;
    bool fired = false;
};

std::vector<Alarm> alarms;
std::mutex alarms_mutex;

std::string format_time_t(time_t t) {
    std::tm tm {};
    #if defined(_WIN32)
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string current_time_string() {
    auto now = Clock::to_time_t(Clock::now());
    return format_time_t(now);
}

void live_clock_thread() {
    while (running) {
        if (!show_live_clock) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        // Clear line and print time (simple)
        std::cout << "\rLive clock: " << current_time_string() << "    " << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void alarm_watcher_thread() {
    while (running) {
        std::unique_lock<std::mutex> lock(alarms_mutex);
        if (alarms.empty()) {
            // wait until something changes (or timeout to recheck running)
            alarm_cv.wait_for(lock, std::chrono::seconds(1));
        } else {
            auto now = std::time(nullptr);
            for (auto &a : alarms) {
                if (!a.fired && a.trigger_time <= now) {
                    a.fired = true;
                    // notify user
                    std::cout << "\n\n*** ALARM: " << a.label << " at " << format_time_t(a.trigger_time) << " ***\n";
                    std::cout << "Press Enter to continue..." << std::endl;
                }
            }
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

// Simple stopwatch
void run_stopwatch() {
    std::cout << "Stopwatch: press Enter to start, Enter to stop, then Enter to reset/return.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    auto start = high_resolution_clock::now();
    std::cout << "Running... press Enter to stop.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    auto stop = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(stop - start);
    long long ms = elapsed.count();
    int hours = (int)(ms / 3600000); ms %= 3600000;
    int minutes = (int)(ms / 60000); ms %= 60000;
    int seconds = (int)(ms / 1000); ms %= 1000;
    std::cout << "Elapsed: " << hours << "h " << minutes << "m " << seconds << "s " << ms << "ms\n";
    std::cout << "Press Enter to return to menu.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Countdown timer
void run_countdown() {
    std::cout << "Enter countdown seconds (integer): ";
    int secs;
    if (!(std::cin >> secs)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Returning.\n";
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Countdown started for " << secs << " seconds...\n";
    for (int i = secs; i >= 0; --i) {
        int h = i / 3600;
        int m = (i % 3600) / 60;
        int s = i % 60;
        std::cout << "\rTime left: " << std::setw(2) << h << ":" << std::setw(2) << m << ":" << std::setw(2) << s << "   " << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "\n*** Countdown finished! ***\n";
    std::cout << "Press Enter to continue.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Add alarm by absolute time (YYYY-MM-DD HH:MM:SS) or relative seconds
time_t parse_datetime(const std::string &s) {
    std::tm tm {};
    std::istringstream iss(s);
    iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (iss.fail()) return -1;
    #if defined(_WIN32)
        time_t t = _mkgmtime(&tm); // treat as local? _mkgmtime makes it UTC - this is platform-detaily.
        // To ensure localtime conversion, use mktime (treats tm as local).
        t = mktime(&tm);
    #else
        time_t t = mktime(&tm); // tm is local time
    #endif
    return t;
}

void add_alarm() {
    std::cout << "Add alarm:\n";
    std::cout << "1) Absolute datetime (YYYY-MM-DD HH:MM:SS)\n";
    std::cout << "2) After N seconds\n";
    std::cout << "Choose option (1 or 2): ";
    int opt;
    if (!(std::cin >> opt)) { std::cin.clear(); std::cin.ignore(10000,'\n'); std::cout << "Invalid.\n"; return; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (opt == 1) {
        std::string line;
        std::cout << "Enter datetime (YYYY-MM-DD HH:MM:SS): ";
        std::getline(std::cin, line);
        time_t t = parse_datetime(line);
        if (t < 0) {
            std::cout << "Invalid format.\n";
            return;
        }
        std::cout << "Label for alarm: ";
        std::string label;
        std::getline(std::cin, label);
        {
            std::lock_guard<std::mutex> lg(alarms_mutex);
            alarms.push_back({t, label, false});
        }
        alarm_cv.notify_one();
        std::cout << "Alarm set for " << format_time_t(t) << "\n";
    } else if (opt == 2) {
        std::cout << "Enter seconds from now: ";
        int sec;
        if (!(std::cin >> sec)) { std::cin.clear(); std::cin.ignore(10000,'\n'); std::cout << "Invalid.\n"; return; }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        time_t t = std::time(nullptr) + sec;
        std::cout << "Label for alarm: ";
        std::string label;
        std::getline(std::cin, label);
        {
            std::lock_guard<std::mutex> lg(alarms_mutex);
            alarms.push_back({t, label, false});
        }
        alarm_cv.notify_one();
        std::cout << "Alarm set for " << format_time_t(t) << "\n";
    } else {
        std::cout << "Invalid option.\n";
    }
}

void list_alarms() {
    std::lock_guard<std::mutex> lg(alarms_mutex);
    if (alarms.empty()) {
        std::cout << "No alarms set.\n";
        return;
    }
    std::cout << "Alarms:\n";
    int i = 1;
    for (const auto &a : alarms) {
        std::cout << i++ << ") " << format_time_t(a.trigger_time) << " - " << a.label;
        if (a.fired) std::cout << " (fired)";
        std::cout << "\n";
    }
}

void remove_alarm() {
    list_alarms();
    std::cout << "Enter alarm number to remove (0 to cancel): ";
    int idx;
    if (!(std::cin >> idx)) { std::cin.clear(); std::cin.ignore(10000,'\n'); std::cout << "Invalid.\n"; return; }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (idx == 0) return;
    std::lock_guard<std::mutex> lg(alarms_mutex);
    if (idx > 0 && idx <= (int)alarms.size()) {
        alarms.erase(alarms.begin() + (idx - 1));
        std::cout << "Removed.\n";
    } else {
        std::cout << "Index out of range.\n";
    }
}

void menu_loop() {
    while (running) {
        std::cout << "\n\n=== Clock App ===\n";
        std::cout << "Current time: " << current_time_string() << "\n";
        std::cout << "1) Toggle live clock display (updates every second)\n";
        std::cout << "2) Stopwatch\n";
        std::cout << "3) Countdown timer\n";
        std::cout << "4) Add alarm\n";
        std::cout << "5) List alarms\n";
        std::cout << "6) Remove alarm\n";
        std::cout << "7) Exit\n";
        std::cout << "Choose option: ";
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        switch (choice) {
            case 1:
                show_live_clock = !show_live_clock;
                if (show_live_clock) std::cout << "Live clock ON (press option 1 again to stop)\n";
                else std::cout << "Live clock OFF\n";
                break;
            case 2:
                run_stopwatch();
                break;
            case 3:
                run_countdown();
                break;
            case 4:
                add_alarm();
                break;
            case 5:
                list_alarms();
                break;
            case 6:
                remove_alarm();
                break;
            case 7:
                running = false;
                alarm_cv.notify_one();
                break;
            default:
                std::cout << "Unknown option.\n";
                break;
        }
    }
}

int main() {
    std::thread t_live(live_clock_thread);
    std::thread t_alarm(alarm_watcher_thread);

    menu_loop();

    // clean up
    running = false;
    alarm_cv.notify_one();
    if (t_live.joinable()) t_live.join();
    if (t_alarm.joinable()) t_alarm.join();
    std::cout << "Goodbye!\n";
    return 0;
}