#include "visualize.h"

void priceGraph(const std::vector<std::vector<std::string>> &ohlcData, int mode)
{
    if (mode != 1 && mode != 2 && mode != 3)
    {
        std::cout << "Invalid mode. Please use 1, 2, or 3." << std::endl;
        return;
    }

    std::vector<double> openingPrices;
    std::vector<double> closingPrices;
    std::vector<std::string> dates;
    std::vector<double> xAxis;

    // Extract opening and closing prices and fill xAxis vector (prices will be plotted against these points)
    double i = 0;
    for (const auto &row : ohlcData)
    {
        xAxis.push_back(i);
        if (row.size() >= 5)
        {
            openingPrices.push_back(std::stod(row[1])); // Opening price (column 1 in OHLC data)
            closingPrices.push_back(std::stod(row[4])); // Closing price (column 4 in OHLC data)
            dates.push_back(row[0]);                    // Date in format y/m/d (column 0 in OHLC data)
        }
        i++;
    }

    // Generate evenly spaced x values
    std::vector<double> x;
    std::vector<std::string> xtickLabels; // New vector for x-tick labels

    if (dates.size() <= 20)
    {
        // Show all dates if the duration is less than or equal to 20 days
        x = matplot::linspace(0, openingPrices.size() - 1, dates.size());
        xtickLabels = dates; // Use all dates as x-tick labels
    }
    else
    {
        // Show fewer ticks for longer durations
        int desiredTicks = 10; // You can adjust this value as needed
        int step = static_cast<int>(std::ceil(static_cast<double>(dates.size()) / desiredTicks));
        for (int i = 0; i < dates.size(); i += step)
        {
            x.push_back(static_cast<double>(i));
            xtickLabels.push_back(dates[i]); // Use selected dates as x-tick labels
        }
    }

    matplot::figure()->quiet_mode(true);
    matplot::hold(matplot::on);
    matplot::xlim({-1, xAxis[xAxis.size()-1]+1}); // To make sure the line does not cross the axis

    if (mode == 1 || mode == 3)
    {
        matplot::plot(xAxis, openingPrices)->color("blue").line_width(2).display_name("Open");
    }

    if (mode == 2 || mode == 3)
    {
        matplot::plot(xAxis, closingPrices)->color("black").line_width(2).display_name("Close");
    }

    ::matplot::legend({});
    matplot::xticks(x);
    matplot::xticklabels(xtickLabels);
    matplot::xtickangle(35);

    // Get the path to the "images" folder (assuming one level up from the executable)
    std::string exePath = std::filesystem::current_path().string();
    std::string imagePath = exePath + "/../images/";

    // Create the "images" folder if it doesn't exist
    std::filesystem::create_directory(imagePath);

    // Save the plot as "price_graph.png" in the "images" folder
    std::string filename = imagePath + "price_graph.png";
    matplot::save(filename);
    // matplot::show();
}

void createCandle(const std::vector<std::vector<std::string>> &ohlcData)
{
    std::vector<std::string> dates;
    std::vector<double> openingPrices;
    std::vector<double> closingPrices;
    std::vector<double> highPrices;
    std::vector<double> lowPrices;
    std::vector<std::string> candleColor;
    std::vector<double> xAxis;

    // Extract OHLC data and fill xAxis vector (prices will be plotted against these points)
    double i = 0;
    for (const auto &row : ohlcData)
    {
        xAxis.push_back(i);
        if (row.size() >= 5)
        {
            dates.push_back(row[0]); // Date in format y/m/d (column 0 in OHLC data)
            openingPrices.push_back(std::stod(row[1])); // Opening price (column 1 in OHLC data)
            highPrices.push_back(std::stod(row[2])); // High price (column 2 in OHLC data)
            lowPrices.push_back(std::stod(row[3]));  // Low price (column 3 in OHLC data)
            closingPrices.push_back(std::stod(row[4])); // Closing price (column 4 in OHLC data)
            if (std::stod(row[4]) >= std::stod(row[1]))
            {
                candleColor.push_back("green");
            }
            else
            {
                candleColor.push_back("red");
            }
        }
        i++;
    }

    // Calculate lowest and highest prices (used for y-axis range)
    double lowestPrice = *std::min_element(lowPrices.begin(), lowPrices.end());
    double highestPrice = *std::max_element(highPrices.begin(), highPrices.end());

    std::vector<double> xTicks;
    std::vector<std::string> xtickLabels; // New vector for x-tick labels

    if (dates.size() <= 20)
    {
        // Show all dates if the duration is less than or equal to 20 days
        xTicks = matplot::linspace(0, openingPrices.size() - 1, dates.size());
        xtickLabels = dates; // Use all dates as x-tick labels
    }
    else
    {
        // Show fewer ticks for longer durations
        int desiredTicks = 10; // You can adjust this value as needed
        int step = static_cast<int>(std::ceil(static_cast<double>(dates.size()) / desiredTicks));
        for (int i = 0; i < dates.size(); i += step)
        {
            xTicks.push_back(static_cast<double>(i));
            xtickLabels.push_back(dates[i]); // Use selected dates as x-tick labels
        }
    }

    // Create the candlestick chart
    matplot::figure()->quiet_mode(true);
    matplot::hold(matplot::on);
    matplot::ylim({+lowestPrice * 0.99, +highestPrice * 1.01});
    matplot::xlim({-1, xAxis[xAxis.size()-1]+1}); // To make sure the first and last candle are not in the axis

    for (int i = 0; i < xAxis.size(); i++)
    {
        // Draw the vertical line (wick) of the candlestick
        matplot::plot({xAxis[i], xAxis[i]}, {highPrices[i], lowPrices[i]})->line_width(1.2).color("black").marker(matplot::line_spec::marker_style::custom);//.marker_color("black").marker_size(6);
        
        // Calculate the top and bottom of the candlestick body
        double top = std::max(openingPrices[i], closingPrices[i]);
        double bottom = std::min(openingPrices[i], closingPrices[i]);
        
        // Draw the filled rectangle (candlestick body)
        float offsetX = 0.2;
        auto r = matplot::rectangle(xAxis[i]-offsetX, bottom, offsetX * 2, (top-bottom)); // TODO FIX THIS!
        r->fill(true);
        r->color(candleColor[i]);
    }

    // Set x-tick positions and labels
    matplot::xticks(xTicks);
    matplot::xticklabels(xtickLabels);
    matplot::xtickangle(35);

    // Get the path to the "images" folder (assuming one level up from the executable)
    std::string exePath = std::filesystem::current_path().string();
    std::string imagePath = exePath + "/../images/";

    // Create the "images" folder if it doesn't exist
    std::filesystem::create_directory(imagePath);

    // Save the plot as "price_graph.png" in the "images" folder
    std::string filename = imagePath + "candle_chart.png";
    matplot::save(filename);
    //matplot::show();
}