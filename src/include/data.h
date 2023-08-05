#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <curl/curl.h>
#include <ctime>
#include <chrono>
#include <vector>
#include <iomanip>
#include "rapidjson/document.h"

/// Callback function to write received data to a string.
/// @param ptr Pointer to the received data buffer.
/// @param size Represents the size of each data element in the buffer (typically the size of a single character, byte).
/// @param nmemb Number of data elements received (e.g., if nmemb is 4, the ptr buffer contains 4 elements of size 'size').
/// @param userdata Pointer to the string object where the received data will be stored; the function appends the received data to this string.
static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, std::string *userdata);

/// Function to convert a duration to time in seconds.
/// @param duration The duration in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
/// @return The duration in seconds.
std::time_t getDurationInSeconds(const std::string& duration);

/// Function to convert a normal date (day/month/year) to a Unix timestamp in seconds.
/// @param date The date in the format: "day/month/year".
/// @return The Unix timestamp in seconds.
std::time_t convertToUnixTimestamp(const std::string &date);

/// Function to perform an HTTP GET request using libcurl.
/// @param url The URL to make the GET request.
/// @return The HTTP response as a string.
static std::string httpGet(const std::string &url);

/// Function to fetch historical stock data from Yahoo Finance and store OHLC data (and dates and volumes) in a 2D vector.
/// @param symbol The symbol of the stock.
/// @param duration The duration/period in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
/// @return A 2D vector where each row contains the following data: date, open, high, low, close, volume (in that order).
/// @note Function is named fetch*OHLC*Data but this also includes dates (in format y/m/d) and volumes. 
std::vector<std::vector<std::string>> fetchOHLCData(const std::string& symbol, const std::string& duration);

/// Function to fetch the latest price of a stock from Yahoo Finance.
/// @param symbol The symbol of the stock.
/// @return The latest price as a double.
double fetchLatestStockPrice(const std::string &symbol);

/// Function to fetch the latest price and % of change compared to the opening price of a stock from Yahoo Finance.
/// Data will be returned in a string as follows: "The latest price of {symbol}: {latestPrice} (%change)".
/// If something went wrong, it will return the following string: "Could not fetch latest price data."
/// @param symbol The symbol or name of the stock.
/// @return A string containing the latest price and % change information.
std::string latestStockPriceAsString(const std::string &symbol);

/// Function to fetch historical stock data from Yahoo Finance and write to a txt file.
/// The txt file will be named {symbol}_{duration}.txt and will be saved in the "data" folder.
/// @param symbol The symbol or name of the stock.
/// @param duration The duration string in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
void fetchAndWriteStockData(const std::string &symbol, const std::string &duration);

#endif // DATA_H