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

// Struct with stock metrics
// Note that this can also be used for futures and indices, but in that case some attributes will remain empty
struct StockMetrics
{
    std::string name = "-";      // Name of stock(company)/future/index (extracted from "displayName", or else from "shortName", which is usually the case with futures/indices)
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
std::time_t getDurationInSeconds(const std::string &duration);

/// Function to convert a normal date (day/month/year) to a Unix timestamp in seconds.
/// @param date The date in the format: "day/month/year".
/// @return The Unix timestamp in seconds.
std::time_t convertToUnixTimestamp(const std::string &date);

/// Function to perform an HTTP GET request using libcurl.
/// @param url The URL to make the GET request.
/// @return The HTTP response as a string.
static std::string httpGet(const std::string &url);

/// Function to fetch historical stock/future/index data from Yahoo Finance and store OHLC data (and dates and volumes) in a 2D vector.
/// The interval of the data is one day.
/// @param symbol The symbol of the stock/future/index.
/// @param duration The duration/period in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
/// @return A 2D vector where each row contains the following data: date, open, high, low, close, volume (in that order).
/// @note Function is named fetch*OHLC*Data but this also includes dates (in format y/m/d) and volumes.
std::vector<std::vector<std::string>> fetchOHLCData(const std::string &symbol, const std::string &duration);

/// Function to fetch the latest price and % of change compared to the opening price of a stock/future/index from Yahoo Finance.
/// Data will be returned in a string as follows: "The latest price of {symbol}: {latestPrice} (%change)".
/// If something went wrong, it will return the following string: "Could not fetch latest price data. Symbol may be invalid."
/// @param symbol The symbol of the stock/future/index.
/// @param markdown When set to true, the formatted string contains Markdown syntax to make it more visually appealing.
/// @return A string containing the latest price and % change information.
std::string getFormattedStockPrice(const std::string &symbol, bool markdown = false);

/// Function to fetch historical stock/future/index data from Yahoo Finance and write to a txt file.
/// The txt file will be named {symbol}_{duration}.txt and will be saved in the "data" folder.
/// The interval of the data is one day.
/// @param symbol The symbol of the stock/future/index.
/// @param duration The duration string in the format: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, or 1w.
void fetchAndWriteStockData(const std::string &symbol, const std::string &duration);

/// Function to fetch stock/future/index metrics from Yahoo Finance API for a single symbol.
/// @param symbol The symbol of the stock/future/index.
/// @return StockMetrics struct containing latest price info, dividend yield, moving averages and more.
StockMetrics fetchStockMetrics(const std::string &symbol);

/// Function to get stock/future/index metrics in a readable way.
/// When data is not available, it will return "Could not fetch data. Symbol may be invalid.".
/// @param symbol The symbol of the stock/future/index.
/// @param markdown When set to true, the formatted string contains Markdown syntax to make it more visually appealing.
/// @return A string with the metrics.
std::string getFormattedStockMetrics(const std::string &symbol, bool markdown = false);

/// Function that takes a vector of symbols, loops over all of them and fetches their latest price data.
/// It then formats the data (latest price + percentage of change compared to open price) in a readable way.
/// If no symbols are provided, it will return "No data available.".
/// If it cannot fetch the data (for example if a symbol is invalid), the formatted string will not contain any info about that symbol.
/// Meaning that if it could not fetch any data at all, the returned string will be empty.
/// @param indicesSymbols Vector of stock/index/future symbols.
/// @param indicesNames Vector of names of the stocks(companies)/futures/indices. If names are given, they will be added instead of the symbols.
/// @param indicesDescriptions Vector of descriptions of the stocks(companies)/futures/indices. These will be added under the symbol (or name).
/// @param markdown When set to true, the formatted string will contain Markdown syntax to make it more visually appealing.
/// @return A string with the formatted price data (and optionally descriptions).
std::string getFormattedPrices(std::vector<std::string> indicesSymbols, std::vector<std::string> indicesNames = {},
                               std::vector<std::string> indicesDescriptions = {}, bool markdown = false);

/// This function reads JSON data containing major indices information for different regions
/// from a JSON file. It extracts the symbols, names, and descriptions of major indices for the specified region, 
/// and formats the data using Markdown syntax if requested. If an error occurs, it will return an error message.
/// @param pathToJson Path to the JSON file.
/// @param region The region for which to retrieve and format major indices data. Allowed values: "US", "EU", "Asia".
/// @param markdown When set to true, the formatted string will contain Markdown syntax to make it more visually appealing.
/// @return A string with the formatted price data and descriptions.
std::string getFormattedMajorIndices(const std::string &pathToJson, const std::string &region, bool markdown = false);

#endif // DATA_H