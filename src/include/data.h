#ifndef DATA_H
#define DATA_H

#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <curl/curl.h>
#include <ctime>
#include <chrono>
#include <vector>
#include <map>
#include <iomanip>
#include "rapidjson/document.h"

// Struct with stock metrics (note that this can also be used for futures)
struct StockMetrics
{
    std::string name = "-";      // Company name (extracted from "displayName", or else from "shortName", which is usually the case with futures)
    std::string currency = "-";  // Currency
    std::string symbol = "-";    // Symbol
    double marketCap = 0;        // Market capitalization of the company's outstanding shares (amount of shares * price of share)
    double dividendYield = 0;    // Dividend yield as a percentage
    double peRatio = 0;          // Price-to-earnings ratio (P/E ratio) indicating stock valuation
    double latestPrice = 0;      // Latest price
    double latestChange = 0;     // Latest price change in percentage (compared to open price of that day)
    double openPrice = 0;        // Open price
    double dayLow = 0;           // Lowest price on day
    double dayHigh = 0;          // Highest price on day
    double prevClose = 0;        // Previous closing price
    double fiftyTwoWeekLow = 0;  // 52 week low (lowest price in last 52 weeks)
    double fiftyTwoWeekHigh = 0; // 52 week high (highest price in last 52 weeks)
    double MA_50 = 0;            // Moving Average 50 Days
    double MA_200 = 0;           // moving Average 200 Days
};

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

/// Function to fetch the latest price and % of change compared to the opening price of a stock from Yahoo Finance.
/// Data will be returned in a string as follows: "The latest price of {symbol}: {latestPrice} (%change)".
/// If something went wrong, it will return the following string: "Could not fetch latest price data. Symbol may be invalid."
/// @param symbol The symbol of the stock.
/// @param markdown When set to true, the formatted string contains Markdown syntax to make it more visually appealing.
/// @return A string containing the latest price and % change information.
std::string getFormattedStockPrice(const std::string &symbol, bool markdown = false);

/// Function to fetch historical stock data from Yahoo Finance and write to a txt file.
/// The txt file will be named {symbol}_{duration}.txt and will be saved in the "data" folder.
/// @param symbol The symbol of the stock.
/// @param duration The duration string in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
void fetchAndWriteStockData(const std::string &symbol, const std::string &duration);

/// Function to fetch stock metrics from Yahoo Finance API for a single symbol.
/// @param symbol The symbol of the stock.
/// @return StockMetrics struct containing latest price info, dividend yield, moving averages and more.
/// @note This function does not work with indices (for example ^DJI, ^GSPC). This is because the query used
///       to fetch the data only works with stocks and futures. To get (historical) price data for indices,
///       the function fetchOHLCData can be used.
StockMetrics fetchStockMetrics(const std::string &symbol);

/// Function to get stock metrics in a readable way.
/// When data is not available, it will return "Could not fetch data. Symbol may be invalid.".
/// @param symbol The symbol of the stock.
/// @param markdown When set to true, the formatted string contains Markdown syntax to make it more visually appealing.
/// @return A string with the metrics.
std::string getFormattedStockMetrics(const std::string &symbol, bool markdown = false);

/// Function that fetches latest price info of 10 major indices and formats it in a readable way
/// so that it can be printed. The indices are: S&P 500, Dow Jones Industrial Average, NASDAQ Composite, 
/// FTSE 100, DAX PERFORMANCE-INDEX, Nikkei 225, HANG SENG INDEX, SSE Composite Index, CAC 40 and S&P/ASX 200.
/// Besides price info, it will also contain a short bit of info about the index.
/// @param markdown When set to true, the formatted string will contain Markdown syntax to make it more visually appealing.
/// @return string with price and index info.
std::string getFormattedMajorIndices(bool markdown = false);

#endif // DATA_H