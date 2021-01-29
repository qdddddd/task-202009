#include "deserialize.h"

/**
 * Defines the position of each
 * component in one line of the
 * log file.
 */
#define NLOGLEVEL 6       // "DEBUG "
#define NDATE 8           // date, i.e., "20200812"
#define NPLACEHOLDER_1 25 // content until the first '#'
#define NORDER 10         // id indicating logging order, i.e., "2135000001"
#define NPLACEHOLDER_2 6  // content until stock symbol
#define NSYMBOL 9
#define NPLACEHOLDER_3 5  // content until ETS
#define NTIME 12          // ETS, i.e., "09:25:00.000"
#define NPLACEHOLDER_4 2  // content until B1 or A1 value
#define NPLACEHOLDER_6 22 // content until prediction
#define NMAX std::numeric_limits<std::streamsize>::max()

bool operator>>(std::istream& fs, MarketData& md) {
    fs.ignore(NLOGLEVEL);

    char datetime[NDATE + NTIME + 1]{};
    fs.read(datetime, NDATE);
    fs.ignore(NPLACEHOLDER_1);

    if (fs.peek() != '#') { // for market data
        fs >> md.id;
        fs.ignore(NPLACEHOLDER_2 + NSYMBOL + NPLACEHOLDER_3);

        fs.read(datetime + NDATE, NTIME);
        md.timestamp = ToTimestamp(datetime);

        fs.ignore(NMAX, 'B');
        fs.ignore(NPLACEHOLDER_4);
        fs >> md.b1;

        fs.ignore(NMAX, 'A');
        fs.ignore(NPLACEHOLDER_4);
        fs >> md.a1;

        SkipToNext(fs);

        if (!md.status) {
            md.status = MARKET;
        } else {
            md.status = COMPLETE;
        }

        if (md.b1 >= md.a1) {
            return false;
        }

    } else { // for prediction
        fs.ignore(1 + NORDER + NPLACEHOLDER_6);
        fs >> md.y_pred;

        SkipToNext(fs);

        if (!md.status) {
            md.status = PRED;
        } else {
            md.status = COMPLETE;
        }
    }

    return true;
}

void SkipToNext(std::istream& fs) {
    fs.ignore(NMAX, '\n');
}

uint64_t ToTimestamp(const char* datetime_str) {
    struct tm t;
    strptime(datetime_str, "%Y%m%d%H:%M:%S", &t);
    uint64_t millsec = std::atoi(datetime_str + 20);
    t.tm_isdst       = 0;
    return mktime(&t) * (uint64_t) 1000 + millsec;
}

/**
 * Template of executing the given function
 * which may move the istream cursor and then
 * restore the cursor to the original position.
 */
template <typename FunctionType>
static typename std::result_of<FunctionType()>::type ExecAndRestoreCursor(std::istream& fs, FunctionType&& f) {
    auto start_pos = fs.tellg();
    auto ret       = f();
    fs.seekg(start_pos);
    return ret;
}

bool IsNextLinePrediction(std::istream& fs) {
    return ExecAndRestoreCursor(fs, [&fs] {
        // fs.ignore(NLOGLEVEL + NDATE + NPLACEHOLDER_1);
        fs.ignore(NMAX, '#');
        return fs.peek() == '#';
    });
}

std::string GetSymbol(std::istream& fs) {
    return ExecAndRestoreCursor(fs, [&fs] {
        if (IsNextLinePrediction(fs)) {
            fs.ignore(NLOGLEVEL + NDATE + NPLACEHOLDER_1 + 1 + NORDER + 1);
        } else {
            fs.ignore(NLOGLEVEL + NDATE + NPLACEHOLDER_1 + NORDER + NPLACEHOLDER_2);
        }
        char symbol[NSYMBOL + 1]{};
        fs.read(symbol, NSYMBOL);
        return std::string(symbol);
    });
}

uint32_t GetOpID(std::istream& fs) {
    return ExecAndRestoreCursor(fs, [&fs] {
        fs.ignore(NMAX, '#');
        uint32_t id;
        if (fs.peek() == '#') {
            fs.ignore(1);
        }
        fs >> id;
        return id;
    });
}

size_t GetSize(std::istream& fs) {
    return ExecAndRestoreCursor(fs, [&fs] {
        fs.ignore(std::numeric_limits<std::streamsize>::max());
        return fs.gcount();
    });
}
