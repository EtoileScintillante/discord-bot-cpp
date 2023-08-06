#include "data.h"

static size_t WriteCallback(char *ptr, size_t size, size_t nmemb, std::string *userdata)
{
    userdata->append(ptr, size * nmemb);
    return size * nmemb;
}

std::time_t getDurationInSeconds(const std::string &duration)
{
    if (duration == "1y")
    {
        return 31536000; // 1 year in seconds
    }
    else if (duration == "6mo")
    {
        return 15552000; // 6 months in seconds
    }
    else if (duration == "3mo")
    {
        return 7776000; // 3 months in seconds
    }
    else if (duration == "2mo")
    {
        return 5184000; // 2 months in seconds
    }
    else if (duration == "1mo")
    {
        return 2592000; // 1 month in seconds
    }
    else if (duration == "3w")
    {
        return 1814400; // 3 weeks in seconds
    }
    else if (duration == "2w")
    {
        return 1209600; // 2 weeks in seconds
    }
    else if (duration == "1w")
    {
        return 604800; // 1 week in seconds
    }
    else
    {
        std::cerr << "Invalid duration. Supported durations: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w, and 1w." << std::endl;
        return 0;
    }
}

std::time_t convertToUnixTimestamp(const std::string &date)
{
    std::tm timeStruct = {};
    std::istringstream ss(date);
    ss >> std::get_time(&timeStruct, "%d/%m/%Y");

    if (ss.fail())
    {
        std::cerr << "Failed to parse the date: " << date << std::endl;
        return 0;
    }

    std::time_t timestamp = std::mktime(&timeStruct);
    return timestamp;
}

static std::string httpGet(const std::string &url)
{
    std::string response;

    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

std::vector<std::vector<std::string>> fetchOHLCData(const std::string &symbol, const std::string &duration)
{
    // Define the current timestamp as the end time
    std::time_t endTime = std::time(nullptr);

    // Calculate the start time based on the duration
    std::time_t startTime = endTime;
    startTime -= getDurationInSeconds(duration);
    if (startTime == endTime)
    {
        std::cerr << "Invalid duration. Supported durations: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w and 1w." << std::endl;
        return std::vector<std::vector<std::string>>();
    }

    // Convert timestamps to strings
    std::ostringstream startTimestamp, endTimestamp;
    startTimestamp << startTime;
    endTimestamp << endTime;

    // Build the URL for historical stock data
    std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + symbol +
                      "?period1=" + startTimestamp.str() + "&period2=" + endTimestamp.str() +
                      "&interval=1d&events=history";

    // Fetch historical stock data using the URL
    std::string response = httpGet(url);
    
        // Check if response contains an error or is empty
    if (response.find("404 Not Found: No data found, symbol may be delisted") != std::string::npos)
    {
        std::cerr << "Symbol not found or delisted: " << symbol << std::endl;
        return std::vector<std::vector<std::string>>();
    }
    if (response.empty())
    {
        std::cerr << "Failed to fetch data from the server." << std::endl;
        return std::vector<std::vector<std::string>>();
    }

    // Parse the CSV response and store OHLC data in a 2D vector
    std::istringstream ss(response);
    std::string line;

    // Skip the header line
    std::getline(ss, line);

    std::vector<std::vector<std::string>> ohlcData; // 2D vector to store OHLC data

    while (std::getline(ss, line))
    {
        std::istringstream lineStream(line);
        std::string date, open, high, low, close, adjClose, volume;
        std::getline(lineStream, date, ',');
        std::getline(lineStream, open, ',');
        std::getline(lineStream, high, ',');
        std::getline(lineStream, low, ',');
        std::getline(lineStream, close, ',');
        std::getline(lineStream, adjClose, ',');
        std::getline(lineStream, volume, ',');

        // Store OHLC data for this row in a vector (as strings)
        std::vector<std::string> ohlcRow = {date, open, high, low, close, volume};
        ohlcData.push_back(ohlcRow);
    }

    return ohlcData;
}

double fetchLatestStockPrice(const std::string &symbol)
{
    std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + symbol +
                      "?period1=0&period2=9999999999&interval=1d&events=history";

    std::string response = httpGet(url);

    // Check if response contains an error or is empty
    if (response.find("404 Not Found: No data found, symbol may be delisted") != std::string::npos)
    {
        std::cerr << "Symbol not found or delisted: " << symbol << std::endl;
        return 0;
    }
    if (response.empty())
    {
        std::cerr << "Failed to fetch data from the server." << std::endl;
        return 0;
    }

    // Split the CSV response by lines to access individual data rows
    std::istringstream ss(response);
    std::string line;
    double latestPrice = 0.0; // Initialize with 0 in case no data is available

    // Iterate through the data rows and extract the closing price from the last row
    while (std::getline(ss, line))
    {
        // Skip the CSV header row
        if (line.find("Date,Open,High,Low,Close,Adj Close,Volume") != std::string::npos)
            continue;

        std::istringstream row(line);
        std::string date, open, high, low, close, adjClose, volume;

        // Extract data from the CSV row
        std::getline(row, date, ',');
        std::getline(row, open, ',');
        std::getline(row, high, ',');
        std::getline(row, low, ',');
        std::getline(row, close, ',');
        std::getline(row, adjClose, ',');
        std::getline(row, volume, ',');

        // Store the closing price from the last available date in the specified time period
        latestPrice = std::stod(close);
    }

    return latestPrice;
}

std::string latestStockPriceAsString(const std::string &symbol)
{
    std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + symbol +
                      "?period1=0&period2=9999999999&interval=1d&events=history";

    std::string response = httpGet(url);

    // Check if response contains an error or is empty
    if (response.find("404 Not Found: No data found, symbol may be delisted") != std::string::npos)
    {
        std::cerr << "Symbol not found or delisted: " << symbol << std::endl;
        return "Could not fetch latest price data.";
    }
    if (response.empty())
    {
        std::cerr << "Failed to fetch data from the server." << std::endl;
        return "Could not fetch latest price data.";
    }

    // Split the CSV response by lines to access individual data rows
    std::istringstream ss(response);
    std::string line;
    double latestPrice = 0.0;  // Initialize with 0 in case no data is available
    double openingPrice = 0.0; // Initialize with 0 in case no data is available

    // Iterate through the data rows and extract the closing price from the last row
    while (std::getline(ss, line))
    {
        // Skip the CSV header row
        if (line.find("Date,Open,High,Low,Close,Adj Close,Volume") != std::string::npos)
            continue;

        std::istringstream row(line);
        std::string date, open, high, low, close, adjClose, volume;

        // Extract data from the CSV row
        std::getline(row, date, ',');
        std::getline(row, open, ',');
        std::getline(row, high, ',');
        std::getline(row, low, ',');
        std::getline(row, close, ',');
        std::getline(row, adjClose, ',');
        std::getline(row, volume, ',');

        // Store the closing price and opening price from the last available date in the specified time period
        try
        {
            latestPrice = std::stod(close);
            openingPrice = std::stod(open);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error converting data to double: " << e.what() << std::endl;
            return "Could not fetch latest price data.";
        }
    }

    // Make sure openingPrice is not zero anymore (cannot divide by zero)
    if (openingPrice == 0.0)
    {
        return "Could not fetch latest price data.";
    }

    // Calculate the percentage change
    double percentageChange = ((latestPrice - openingPrice) / openingPrice) * 100.0;

    // Create the formatted string with the result
    std::ostringstream resultStream;
    resultStream << "The latest price of " << symbol << ": " << std::fixed << std::setprecision(2) << latestPrice
                 << " (" << (percentageChange >= 0 ? "+" : "") << std::fixed << std::setprecision(2) << percentageChange << "%)";

    return resultStream.str();
}

void fetchAndWriteStockData(const std::string &symbol, const std::string &duration)
{
    // Define the current timestamp as the end time
    std::time_t endTime = std::time(nullptr);

    // Calculate the start time based on the duration
    std::time_t startTime = endTime;
    startTime -= getDurationInSeconds(duration);
    if (startTime == endTime)
    {
        std::cerr << "Invalid duration. Supported durations: 1y, 6mo, 3mo, 2mo, 1mo, 3w, 2w and 1w." << std::endl;
        return;
    }

    // Convert timestamps to strings
    std::ostringstream startTimestamp, endTimestamp;
    startTimestamp << startTime;
    endTimestamp << endTime;

    // Build the URL for historical stock data
    std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + symbol +
                      "?period1=" + startTimestamp.str() + "&period2=" + endTimestamp.str() +
                      "&interval=1d&events=history";

    // Fetch historical stock data using the URL
    std::string response = httpGet(url);

    // Check if response contains an error or is empty
    if (response.find("404 Not Found: No data found, symbol may be delisted") != std::string::npos)
    {
        std::cerr << "Symbol not found or delisted: " << symbol << std::endl;
        return;
    }
    if (response.empty())
    {
        std::cerr << "Failed to fetch data from the server." << std::endl;
        return;
    }

    // Get the current directory where the executable is located
    std::string exePath = std::filesystem::current_path().string(); // Use std::experimental::filesystem in C++14

    // Construct the file path to the "data" folder
    std::string chartPath = exePath + "/../data/"; // Assuming the "data" folder is one level up from the executable

    // Create the "data" folder if it doesn't exist
    std::filesystem::create_directory(chartPath); // Use std::experimental::filesystem in C++14

    // Construct the file path for the output file inside the "data" folder
    std::string filename = chartPath + symbol + "_" + duration + ".txt";
    std::ofstream outFile(filename);

    if (outFile)
    {
        outFile << response;
        std::cout << "Stock data for " << symbol << " in the last " << duration << " has been written to " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    outFile.close();
}