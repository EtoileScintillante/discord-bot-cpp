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

std::string getFormattedStockPrice(const std::string &symbol, bool markdown)
{
    // Also fetch metrics to get the currency
    StockMetrics stockMetrics = fetchStockMetrics(symbol);

    // Check if there is a price
    if (stockMetrics.latestPrice == 0)
    {
        return "Could not fetch latest price data. Symbol may be invalid.";
    }

    // Create the formatted string with the result
    std::ostringstream resultStream;
    if (!markdown)
    {
        resultStream << "Latest Price for " << stockMetrics.name << ": " << std::fixed << std::setprecision(2) << stockMetrics.latestPrice << " " << stockMetrics.currency
                     << " (" << (stockMetrics.latestChange >= 0 ? "+" : "") << std::fixed << std::setprecision(2) << stockMetrics.latestChange << "%)";
    }
    else
    {
        resultStream << "### Latest Price for " << stockMetrics.name << "\n"
                     << "`" << std::fixed << std::setprecision(2) << stockMetrics.latestPrice << " " << stockMetrics.currency;

        if (stockMetrics.latestChange >= 0)
        {
            resultStream << " (+" << std::fixed << std::setprecision(2) << stockMetrics.latestChange << "%)`:chart_with_upwards_trend:";
        }
        else
        {
            resultStream << " (" << std::fixed << std::setprecision(2) << stockMetrics.latestChange << "%)`:chart_with_downwards_trend:";
        }
    }

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
        std::cout << "Data for " << symbol << " in the last " << duration << " has been written to " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }

    outFile.close();
}

StockMetrics fetchStockMetrics(const std::string &symbol)
{
    StockMetrics stockMetrics;

    // Construct the Yahoo Finance API URL with the symbol
    std::string apiUrl = "https://query1.finance.yahoo.com/v7/finance/options/" + symbol;

    // Fetch data from the Yahoo Finance API
    std::string response = httpGet(apiUrl);

    // Check if response contains an error or is empty
    if (response.find("{\"optionChain\":{\"result\":[],\"error\":null}}") != std::string::npos)
    {
        std::cerr << "Symbol not found or delisted: " << symbol << std::endl;
        return stockMetrics;
    }
    if (response.empty())
    {
        std::cerr << "Failed to fetch data from the server." << std::endl;
        return stockMetrics;
    }

    // Parse the JSON response
    rapidjson::Document document;
    document.Parse(response.c_str());

    if (!document.HasParseError() && document.HasMember("optionChain"))
    {
        const rapidjson::Value &optionChain = document["optionChain"]["result"][0];
        const rapidjson::Value &quote = optionChain["quote"];

        // Extract metrics from quote object
        if (quote.HasMember("displayName") && quote["displayName"].IsString()) // name from displayName
        {
            stockMetrics.name = quote["displayName"].GetString();
        }
        else if ((quote.HasMember("shortName") && quote["shortName"].IsString())) // name from shortName
        {
            stockMetrics.name = quote["shortName"].GetString();
        }
        if (quote.HasMember("symbol") && quote["symbol"].IsString()) // symbol
        {
            stockMetrics.symbol = quote["symbol"].GetString();
        }
        if (quote.HasMember("currency") && quote["currency"].IsString()) // currency
        {
            stockMetrics.currency = quote["currency"].GetString();
        }
        if (quote.HasMember("marketCap") && quote["marketCap"].IsNumber()) // marketCap
        {
            stockMetrics.marketCap = quote["marketCap"].GetDouble();
        }
        if (quote.HasMember("dividendYield") && quote["dividendYield"].IsNumber()) // dividendYield
        {
            stockMetrics.dividendYield = quote["dividendYield"].GetDouble();
        }
        if (quote.HasMember("trailingPE") && quote["trailingPE"].IsNumber()) // peRatio
        {
            stockMetrics.peRatio = quote["trailingPE"].GetDouble();
        }
        if (quote.HasMember("regularMarketPrice") && quote["regularMarketPrice"].IsNumber()) // latestPrice
        {
            stockMetrics.latestPrice = quote["regularMarketPrice"].GetDouble();
        }
        if (quote.HasMember("regularMarketChangePercent") && quote["regularMarketChangePercent"].IsNumber()) // latestChange
        {
            stockMetrics.latestChange = quote["regularMarketChangePercent"].GetDouble();
        }
        if (quote.HasMember("regularMarketOpen") && quote["regularMarketOpen"].IsNumber()) // openPrice
        {
            stockMetrics.openPrice = quote["regularMarketOpen"].GetDouble();
        }
        if (quote.HasMember("regularMarketDayLow") && quote["regularMarketDayLow"].IsNumber()) // dayLow
        {
            stockMetrics.dayLow = quote["regularMarketDayLow"].GetDouble();
        }
        if (quote.HasMember("regularMarketDayHigh") && quote["regularMarketDayHigh"].IsNumber()) // dayHigh
        {
            stockMetrics.dayHigh = quote["regularMarketDayHigh"].GetDouble();
        }
        if (quote.HasMember("regularMarketPreviousClose") && quote["regularMarketPreviousClose"].IsNumber()) // prevClose
        {
            stockMetrics.prevClose = quote["regularMarketPreviousClose"].GetDouble();
        }
        if (quote.HasMember("fiftyTwoWeekLow") && quote["fiftyTwoWeekLow"].IsNumber()) // fiftyTwoWeekLow
        {
            stockMetrics.fiftyTwoWeekLow = quote["fiftyTwoWeekLow"].GetDouble();
        }
        if (quote.HasMember("fiftyTwoWeekHigh") && quote["fiftyTwoWeekHigh"].IsNumber()) // fiftyTwoWeekHigh
        {
            stockMetrics.fiftyTwoWeekHigh = quote["fiftyTwoWeekHigh"].GetDouble();
        }
        if (quote.HasMember("fiftyDayAverage") && quote["fiftyDayAverage"].IsNumber()) // MA_50
        {
            stockMetrics.MA_50 = quote["fiftyDayAverage"].GetDouble();
        }
        if (quote.HasMember("twoHundredDayAverage") && quote["twoHundredDayAverage"].IsNumber()) // MA_200
        {
            stockMetrics.MA_200 = quote["twoHundredDayAverage"].GetDouble();
        }
    }
    else
    {
        std::cerr << "JSON parsing error or missing member" << std::endl;
    }

    return stockMetrics;
}

std::string getFormattedStockMetrics(const std::string &symbol, bool markdown)
{
    // Fetch stock metrics for the given symbol
    StockMetrics metrics = fetchStockMetrics(symbol);

    // Check if data is valid
    if (metrics.symbol == "-")
    {
        return "Could not fetch data. Symbol may be invalid.";
    }

    // Create a string stream to hold the formatted metrics
    std::ostringstream formattedMetrics;

    // Format the stock metrics
    if (!markdown)
    {
        formattedMetrics << "Metrics for " << metrics.name << ":\n";
        formattedMetrics << std::fixed << std::setprecision(2);
        formattedMetrics << "- Market Cap:         " << metrics.marketCap << " " << metrics.currency << "\n";
        formattedMetrics << "- Dividend Yield:     " << metrics.dividendYield << "%"
                         << "\n";
        formattedMetrics << "- P/E Ratio:          " << metrics.peRatio << "\n";
        formattedMetrics << "- Latest Price:       " << metrics.latestPrice << " " << metrics.currency << "\n";
        formattedMetrics << "- Open price:         " << metrics.openPrice << " " << metrics.currency << "\n";
        formattedMetrics << "- Day Low:            " << metrics.dayLow << " " << metrics.currency << "\n";
        formattedMetrics << "- Day High:           " << metrics.dayHigh << " " << metrics.currency << "\n";
        formattedMetrics << "- Previous Close:     " << metrics.prevClose << " " << metrics.currency << "\n";
        formattedMetrics << "- 52 Week Low:        " << metrics.fiftyTwoWeekLow << " " << metrics.currency << "\n";
        formattedMetrics << "- 52 Week High:       " << metrics.fiftyTwoWeekHigh << " " << metrics.currency << "\n";
        formattedMetrics << "- MA50:               " << metrics.MA_50 << " " << metrics.currency << "\n";
        formattedMetrics << "- MA200:              " << metrics.MA_200 << " " << metrics.currency << "\n";
    }
    else // It looks weird but this way in Discord the spaces between the names and values are even
    {
        formattedMetrics << "### Metrics for " << metrics.name << "\n";
        formattedMetrics << std::fixed << std::setprecision(2);
        formattedMetrics << "- Market Cap:          `" << metrics.marketCap << " " << metrics.currency << "`\n";
        formattedMetrics << "- Dividend Yield:     `" << metrics.dividendYield << "%`\n";
        formattedMetrics << "- P/E Ratio:              `" << metrics.peRatio << "`\n";
        formattedMetrics << "- Latest Price:         `" << metrics.latestPrice << " " << metrics.currency << "`\n";
        formattedMetrics << "- Open price:           `" << metrics.openPrice << " " << metrics.currency << "`\n";
        formattedMetrics << "- Day Low:                `" << metrics.dayLow << " " << metrics.currency << "`\n";
        formattedMetrics << "- Day High:               `" << metrics.dayHigh << " " << metrics.currency << "`\n";
        formattedMetrics << "- Previous Close:    `" << metrics.prevClose << " " << metrics.currency << "`\n";
        formattedMetrics << "- 52 Week Low:       `" << metrics.fiftyTwoWeekLow << " " << metrics.currency << "`\n";
        formattedMetrics << "- 52 Week High:      `" << metrics.fiftyTwoWeekHigh << " " << metrics.currency << "`\n";
        formattedMetrics << "- MA50:                    `" << metrics.MA_50 << " " << metrics.currency << "`\n";
        formattedMetrics << "- MA200:                 `" << metrics.MA_200 << " " << metrics.currency << "`\n";
    }

    // Return the formatted metrics as a string
    return formattedMetrics.str();
}

std::string getFormattedPrices(std::vector<std::string> indicesSymbols, std::vector<std::string> indicesNames, std::vector<std::string> indicesDescriptions, bool markdown)
{
    // Check if data is available
    if (indicesSymbols.empty())
    {
        return "No data available.";
    }

    // Check if there are names and descriptions available
    bool addNames = false;
    bool addDescription = false;
    if ((!indicesNames.empty()) && (indicesNames.size() == indicesSymbols.size()))
    {
        addNames = true;
    }
    if ((!indicesDescriptions.empty()) && (indicesDescriptions.size() == indicesSymbols.size()))
    {
        addDescription = true;
    }

    std::ostringstream formattedString;

    for (int i = 0; i < indicesSymbols.size(); i++)
    {
        // First fetch price data
        StockMetrics data = fetchStockMetrics(indicesSymbols[i]);

        // Now create string
        if (!markdown)
        {
            if (addNames) // Display name
            {
                formattedString << indicesNames[i] << std::endl;
            }
            else // Otherwise just add the symbol
            {
                formattedString << indicesSymbols[i] << std::endl;
            }
            if (addDescription) // Add description if available
            {
                formattedString << indicesDescriptions[i] << std::endl;
            }
            formattedString << std::fixed << std::setprecision(2);
            formattedString << "- Latest price: " << data.latestPrice << " " << data.currency;
            if (data.latestChange >= 0)
            {
                formattedString << " (+" << data.latestChange << "%)\n";
            }
            else
            {
                formattedString << " (" << data.latestChange << "%)\n";
            }
        }
        else
        {
            if (addNames) // Display name
            {
                formattedString << "### " << indicesNames[i] << std::endl;
            }
            else // Otherwise just add the symbol
            {
                formattedString << "### " << indicesSymbols[i] << std::endl;
            }
            if (addDescription) // Add description if available
            {
                formattedString << indicesDescriptions[i] << std::endl;
            }
            formattedString << std::fixed << std::setprecision(2);
            formattedString << "- Latest price: `" << data.latestPrice << " " << data.currency;
            if (data.latestChange >= 0)
            {
                formattedString << " (+" << data.latestChange << "%)`:chart_with_upwards_trend:\n";
            }
            else
            {
                formattedString << " (" << data.latestChange << "%)`:chart_with_downwards_trend:\n";
            }
        }
    }

    return formattedString.str();
}

std::string getFormattedMajorIndices(const std::string &pathToJson, const std::string &region, bool description, bool markdown)
{
    // Load JSON data from a file
    std::ifstream file(pathToJson);
    if (!file.is_open())
    {
        return "Error: Unable to open JSON file.";
    }

    // Read JSON data from the file
    std::string jsonData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    rapidjson::Document document;
    document.Parse(jsonData.c_str());

    if (!document.IsObject())
    {
        return "Error: Invalid JSON data.";
    }

    rapidjson::Value::ConstMemberIterator regionIt = document.FindMember(region.c_str());
    if (regionIt == document.MemberEnd() || !regionIt->value.IsArray())
    {
        return "Error: Invalid region or region data.";
    }

    const rapidjson::Value &regionData = regionIt->value;

    std::vector<std::string> indicesSymbols;
    std::vector<std::string> indicesNames;
    std::vector<std::string> indicesDescriptions;

    // Put data in vectors
    for (rapidjson::SizeType i = 0; i < regionData.Size(); ++i)
    {
        const rapidjson::Value &indexData = regionData[i];
        if (indexData.HasMember("symbol") && indexData.HasMember("name") && indexData.HasMember("description"))
        {
            indicesSymbols.push_back(indexData["symbol"].GetString());
            indicesNames.push_back(indexData["name"].GetString());
            if (description)
            {
                indicesDescriptions.push_back(indexData["description"].GetString());
            }
        }
    }

    return getFormattedPrices(indicesSymbols, indicesNames, indicesDescriptions, markdown);
}