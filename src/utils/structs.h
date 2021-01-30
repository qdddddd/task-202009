#ifndef UTILS_H
#define UTILS_H

#include <bits/stdint-uintn.h>
#include <cfloat>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>

struct RSquared {
    double sum{}, sq_sum{}, residual{};
    size_t cnt{};

    void Update(double y_true, double y_pred) {
        sum += y_true;
        sq_sum += (y_true * y_true);
        auto err = y_true - y_pred;
        residual += (err * err);
        cnt++;
    }

    double Result() {
        auto mean = sum / (double) cnt;
        auto tot  = sq_sum - sum * mean;
        return 1 - residual / tot;
    }
};

enum Status {
    NONE = 0, // nothing is parsed
    MARKET,   // only market data is parsed
    PRED,     // only prediction is parsed
    COMPLETE, // both market data and prediction are parsed.
};

struct MarketData {
    uint32_t id{};          // OpID indicating logging order
    uint64_t timestamp{};   // unix timestamp in milliseconds
    float a1{};             // A1
    float b1{};             // B1
    double y_pred{DBL_MAX}; // prediction
    double y_true{DBL_MAX}; // label

    Status status{NONE};

    void UpdateLabel(const MarketData& other) {
        if (y_true < DBL_MAX) {
            return;
        }

        auto mt  = (a1 + b1) / 2;
        auto mtn = (other.a1 + other.b1) / 2;
        y_true   = (mtn / mt - 1) * 1000;
    }

    void Clear() {
        id = 0, timestamp = 0, a1 = 0, b1 = 0, y_pred = DBL_MAX, y_true = DBL_MAX, status = NONE;
    }

    bool Ready() {
        return (y_pred < DBL_MAX) && (y_true < DBL_MAX);
    }

    bool Completed() {
        return status == COMPLETE;
    }

    bool HasMarketData() {
        return status == MARKET || status == COMPLETE;
    }

    std::string to_string() {
        std::stringstream ss;
        ss << "{";
        ss << " ID: " << id << ", ";
        ss << " ts: " << timestamp << ", ";
        ss << " B1: " << b1 << ", ";
        ss << " A1: " << a1 << ", ";
        ss << " pred: " << std::to_string(y_pred) << ", ";
        ss << " true: " << std::to_string(y_true);
        ss << " }";

        return ss.str();
    }
};

using MDPtr = std::shared_ptr<MarketData>;

/**
 * Key hasher for MDPtr
 */
template <>
struct std::hash<MDPtr> {
    const uint64_t operator()(const MDPtr& d) const {
        return d->id;
    }
};

/**
 * Comparators for MDPtr
 */
inline bool operator<(const MDPtr& fat, const uint32_t& light) {
    return fat->id < light;
}
inline bool operator<(const uint32_t& light, const MDPtr& fat) {
    return light < fat->id;
}
inline bool operator<(const MDPtr& fat1, const MDPtr& fat2) {
    return fat1->id < fat2->id;
}

#endif /* ifndef UTILS_H */
