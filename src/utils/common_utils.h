#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <cstdio>
#include <fstream>
#include <shared_mutex>
#include <unordered_map>

#define READER_LOCK(mu) std::shared_lock<std::shared_mutex> reader(mu);
#define WRITER_LOCK(mu) std::unique_lock<std::shared_mutex> writer(mu);
#define LOG(msg, ...) printf(("[" + Now() + "] " + msg + "\n").c_str(), ##__VA_ARGS__)

enum DownloadStatus {
    NOT_STARTED = 0,
    INCOMPLETE,
    COMPLETED,
};

static std::string Now() {
    time_t raw_time;
    struct tm* timeinfo;
    char buffer[80];

    time(&raw_time);
    timeinfo = localtime(&raw_time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

template <typename K, typename V>
static bool Contains(const std::unordered_map<K, V>& m, const K& key, std::shared_mutex* mu = nullptr) {
    if (mu) {
        READER_LOCK(*mu);
    }

    if (m.find(key) != m.end()) {
        return true;
    }

    return false;
}

static void Unzip(const std::string& filename) {
    std::string unzip_cmd = "gunzip " + filename;
    auto i                = system(unzip_cmd.c_str());
}

static bool FileExists(const std::string& name) {
    std::ifstream f(name);
    return f.good();
}

#endif /* ifndef COMMON_UTILS_H */
