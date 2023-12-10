/// @file visualize.h
/// @author EtoileScintillante
/// @brief The following file contains functions to visualize equity data.      
/// @date 2023-08-09

#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "data.h"
#include <matplot/matplot.h>

/// Plots Open and Close prices and saves the graph as price_graph.png in the folder 'images'.
/// This function fetches OHLC (Open-High-Low-Close) data and plots the opening
/// and/or closing prices over time. It provides three display modes:
/// - Mode 1: Display only the open prices.
/// - Mode 2: Display only the close prices.
/// - Mode 3: Display both open and close prices.
/// @param symbol The symbol of the stock/future/index/crypto.
/// @param duration The duration/period in the format: 1y, 6mo, 2w, 12d, etc.
/// @param mode An integer representing the display mode (1, 2, or 3).
///             Any other value will result in an error message and no plot.
void priceGraph(std::string symbol, std::string duration, int mode);

/// Plots OHLC data and saves the candlestick chart as candle_chart.png in the folder 'images'.
/// This function fetches OHLC (Open-High-Low-Close) data and creates a candlestick chart.
/// The interval of the data is one day.
/// @param symbol The symbol of the stock/future/index/crypto.
/// @param duration The duration/period in the format: 1y, 6mo, 2w, 12d, etc.
/// @note Please note that using OHLC data of a period >6 months may lead
///       to a candlestick chart that is not clearly readable. In such cases, the candlesticks
///       may appear very thin or small, making it hard to discern the details.
void createCandle(std::string symbol, std::string duration);

/// Plots OHLCV data and saves the candlestick chart as candle_volume.png in the folder 'images'.
/// This function fetches OHLCV (Open-High-Low-Close-Volume) data and creates a candlestick chart with the volume bars
/// plotted at the bottom of the chart. The interval of the data is one day.
/// @param symbol The symbol of the stock/future/index/crypto.
/// @param duration The duration/period in the format: 1y, 6mo, 2w, 12d, etc.
/// @note Please note that using OHLCV data of a period >6 months may lead
///       to a candlestick chart and volume graph that are not clearly readable. In such cases, the candlesticks
///       and volume bars may appear very thin or small, making it hard to discern the details.
void createCandleAndVolume(std::string symbol, std::string duration);


#endif // VISUALIZE_H