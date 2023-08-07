#ifndef VISUALIZE_H
#define VISUALIZE_H

#include <string>
#include <chrono>
#include <vector>
#include <matplot/matplot.h>

/// Plot Open and Close prices and saves the graph as price_graph.png in the folder 'images'.
/// This function takes OHLC (Open-High-Low-Close) data and plots the opening
/// and/or closing prices over time. It provides three display modes:
/// - Mode 1: Display only the open prices.
/// - Mode 2: Display only the close prices.
/// - Mode 3: Display both open and close prices.
/// @param ohlcData A 2D vector containing OHLC data, where each row represents
///                 a data point and the columns represent: date, open, high,
///                 low, and close prices (in this order).
/// @param mode An integer representing the display mode:
///             - 1: Display only the open prices.
///             - 2: Display only the close prices.
///             - 3: Display both open and close prices.
///             Any other value will result in an error message and no plot.
void priceGraph(const std::vector<std::vector<std::string>>& ohlcData, int mode);

/// Plot OHLC data and saves the candlestick chart as candle_chart.png in the folder 'images'.
/// This function takes OHLC (Open-High-Low-Close) data and creates a candlestick chart.
/// @param ohlcData A 2D vector containing OHLC data, where each row represents
///                 a data point and the columns represent: date, open, high,
///                 low, and close prices (in this order).
/// @note Please note that using OHLC data of a period >=6 months may lead
///       to a candlestick chart that is not clearly readable. In such cases, the candlesticks
///       may appear very thin or small, making it challenging to discern the details.
void createCandle(const std::vector<std::vector<std::string>>& ohlcData);

/// Plot OHLC and volume data in one figure and saves it as candle_volume.png in the folder 'images'.
/// This function takes OHLCV (Open-High-Low-Close-Volume) data and creates a candlestick chart with the volume bars
/// plotted at the bottom of the chart.
/// @param ohlcvData A 2D vector containing OHLC data, where each row represents
///                  a data point and the columns represent: date, open, high,
///                  low, close and volume (in this order).
/// @note Please note that using OHLCV data of a period >=6 months may lead
///       to a candlestick chart and volume graph that are not clearly readable. In such cases, the candlesticks
///       and volume bars may appear very thin or small, making it challenging to discern the details.
void createCandleAndVolume(const std::vector<std::vector<std::string>> &ohlcvData);


#endif // VISUALIZE_H