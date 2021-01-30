#ifndef DESERIALIZE_H
#define DESERIALIZE_H

#include "structs.h"

/**
 * Deserializer for MarketData from istream.
 * It is user's responsibility to check if
 * the next line of istream is market data
 * or prediction before calling this operator.
 */
bool operator>>(std::istream& fs, MarketData& md);

/**
 * Skip to the next line.
 */
void SkipToNext(std::istream& fs);

/**
 * Checks whether the next line
 * is prediction (return true)
 * or market data (return false).
 */
bool IsNextLinePrediction(std::istream& fs);

/**
 * Gets the stock symbol in the next line.
 * Does NOT move the istream cursor.
 */
std::string GetSymbol(std::istream& fs);

/**
 * Gets the operation id in the next line.
 * Does NOT move the istream cursor.
 */
uint32_t GetOpID(std::istream& fs);

/**
 * Gets the size of the file
 */
size_t GetSize(std::istream& fs);

/**
 * Converts a datetime string to
 * a timestamp in the unit ms.
 */
uint64_t ToTimestamp(const char* datetime_str);

#endif /* ifndef DESERIALIZE_H */
