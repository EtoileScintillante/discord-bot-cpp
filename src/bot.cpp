#include "bot.h"

Bot::Bot(const std::string &token) : bot(token, dpp::i_default_intents | dpp::i_message_content)
{
    bot.on_log(dpp::utility::cout_logger());
}

void Bot::run()
{
    setupBot();
    bot.start(dpp::st_wait);
}

void Bot::setupBot()
{
    bot.on_slashcommand([this](const dpp::slashcommand_t &event)
                        { commandHandler(event); });

    bot.on_ready([this](const dpp::ready_t &event)
                 { onReady(event); });
}

void Bot::commandHandler(const dpp::slashcommand_t &event)
{
    if (event.command.get_command_name() == "latestprice")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string priceStr = getFormattedStockPrice(symbol, true);
        event.reply(priceStr);
    }
    else if (event.command.get_command_name() == "pricegraph")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string period = std::get<std::string>(event.get_parameter("period"));
        std::string mode = std::get<std::string>(event.get_parameter("mode"));
        std::vector<std::vector<std::string>> ohlcData = fetchOHLCData(symbol, period);
        if (ohlcData.empty()) // No data, so do not try to create graph
        {
            dpp::message errorMsg{"Could not fetch data. Symbol may be invalid."};
            event.reply(errorMsg);
        }
        else // Create graph
        {
            // Use std::async to create the graph asynchronously
            auto future = std::async(std::launch::async, [&]()
                                    {
                    priceGraph(ohlcData, std::stoi(mode)); });

            // Wait for the graph creation to finish
            future.wait();

            // Additional delay to make sure the file is fully written to disk
            // Without this delay the bot sends an empty image file
            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            const std::string imagePath = "../images/price_graph.png";
            const int maxAttempts = 5;
            int attempts = 0;

            // Check if the file exists
            while (!std::filesystem::exists(imagePath) && attempts < maxAttempts)
            {
                attempts++;
            }

            // If the file exists, add it to the message
            if (std::filesystem::exists(imagePath))
            {
                // Convert to all caps because that looks better
                for (char &c : symbol) 
                {
                    c = std::toupper(c);
                }
                dpp::message msg{"### Price Graph for " + symbol};
                msg.add_file("price_graph.png", dpp::utility::read_file(imagePath));
                event.reply(msg);
                
                // Delete the file after sending the message
                std::filesystem::remove(imagePath);
            }
            else
            {
                // If the file doesn't exist, reply with an error message
                dpp::message errorMsg{"Oops! Something went wrong while creating the figure."};
                event.reply(errorMsg);
            }
        }
    }
    else if (event.command.get_command_name() == "candlestick")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string period = std::get<std::string>(event.get_parameter("period"));
        std::string showV = std::get<std::string>(event.get_parameter("volume"));
        std::vector<std::vector<std::string>> ohlcData = fetchOHLCData(symbol, period);
        if (ohlcData.empty()) // No data, so do not try to create the figure
        {
            dpp::message errorMsg{"Could not fetch data. Symbol may be invalid."};
            event.reply(errorMsg);
        }
        else // Create figure
        {
            // Use std::async to create the graph asynchronously
            auto future = std::async(std::launch::async, [&]()
                                    {
                    
                    (showV == "n") ? createCandle(ohlcData) : createCandleAndVolume(ohlcData);
                    });

            // Wait for the graph creation to finish
            future.wait();

            // Additional delay to make sure the file is fully written to disk
            // Without this delay the bot sends an empty image file
            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            std::string imagePath = (showV == "n") ? "../images/candle_chart.png" : "../images/candle_volume.png";
            const int maxAttempts = 5;
            int attempts = 0;

            // Check if the file exists
            while (!std::filesystem::exists(imagePath) && attempts < maxAttempts)
            {
                attempts++;
            }

            // If the file exists, add it to the message
            if (std::filesystem::exists(imagePath))
            {
                // Convert to all caps because that looks better
                for (char &c : symbol) 
                {
                    c = std::toupper(c);
                }
                dpp::message msg{"### Candlestick Chart for " + symbol};
                msg.add_file(showV == "n" ? "candle_chart.png" : "candle_volume.png", dpp::utility::read_file(imagePath));
                event.reply(msg);
                
                // Delete the file after sending the message
                std::filesystem::remove(imagePath);
            }
            else
            {
                // If the file doesn't exist, reply with an error message
                dpp::message errorMsg{"Oops! Something went wrong while creating the figure."};
                event.reply(errorMsg);
            }
        }
    }
    else if (event.command.get_command_name() == "metrics")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string metrics = getFormattedStockMetrics(symbol, true);
        event.reply(metrics);
    }
    else if (event.command.get_command_name() == "majorindices")
    {
        std::string info;
        std::string region = std::get<std::string>(event.get_parameter("region"));
        if (region == "a")
        {
            info = getFormattedMajorIndices("../data/indices.json", "Asia", true);
        }
        if (region == "e")
        {
            info = getFormattedMajorIndices("../data/indices.json", "EU", true);
        }
        if (region == "u")
        {
            info = getFormattedMajorIndices("../data/indices.json", "US", true);
        }
        event.reply(info);
    }
}

void Bot::onReady(const dpp::ready_t &event)
{
    if (dpp::run_once<struct register_bot_commands>())
    {
        registerCommands();
    }
}

void Bot::registerCommands()
{
    // Create slash command
    dpp::slashcommand latestprice("latestprice", "Get the latest price of a stock, future or index", bot.me.id);
    latestprice.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));

    // Create slash command
    dpp::slashcommand pricegraph("pricegraph", "Get a graph of the closing and/or open price of a stock, future or index", bot.me.id);
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "period", "Period/time span", true).
        add_choice(dpp::command_option_choice("1 year", std::string("1y"))).
        add_choice(dpp::command_option_choice("6 months", std::string("6mo"))).
        add_choice(dpp::command_option_choice("3 months", std::string("3mo"))).
        add_choice(dpp::command_option_choice("2 months", std::string("2mo"))).
        add_choice(dpp::command_option_choice("1 month", std::string("1mo"))).
        add_choice(dpp::command_option_choice("3 weeks", std::string("3w"))).
        add_choice(dpp::command_option_choice("2 weeks", std::string("2w"))).
        add_choice(dpp::command_option_choice("1 week", std::string("1w"))));
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "mode", "Price type", true).
        add_choice(dpp::command_option_choice("Only open", std::string("1"))).
        add_choice(dpp::command_option_choice("Only close", std::string("2"))).
        add_choice(dpp::command_option_choice("Both", std::string("3"))));

    // Create slash command
    dpp::slashcommand candlestick("candlestick", "Get a candlestick chart for a stock, future or index (optionally with volumes)", bot.me.id);
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "period", "Period/time span", true).
        add_choice(dpp::command_option_choice("6 months", std::string("6mo"))).
        add_choice(dpp::command_option_choice("3 months", std::string("3mo"))).
        add_choice(dpp::command_option_choice("2 months", std::string("2mo"))).
        add_choice(dpp::command_option_choice("1 month", std::string("1mo"))).
        add_choice(dpp::command_option_choice("3 weeks", std::string("3w"))).
        add_choice(dpp::command_option_choice("2 weeks", std::string("2w"))).
        add_choice(dpp::command_option_choice("1 week", std::string("1w"))));
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "volume", "Show volumes", true).
        add_choice(dpp::command_option_choice("Yes", std::string("y"))).
        add_choice(dpp::command_option_choice("No", std::string("n"))));
    
    // Create slash command
    dpp::slashcommand majorindices("majorindices", "Get latest price info for major indices of a certain region", bot.me.id);
    majorindices.add_option(
        dpp::command_option(dpp::co_string, "region", "Region", true).
        add_choice(dpp::command_option_choice("Asia", std::string("a"))).
        add_choice(dpp::command_option_choice("Europe", std::string("e"))).
        add_choice(dpp::command_option_choice("US", std::string("u"))));

    // Create slash command
    dpp::slashcommand metrics("metrics", "Get metrics of a stock, future or index", bot.me.id);
    metrics.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));

    // Add commands to vector and register them
    std::vector<dpp::slashcommand> commands;
    commands.push_back(latestprice);
    commands.push_back(pricegraph);
    commands.push_back(candlestick);
    commands.push_back(majorindices);
    commands.push_back(metrics);
    bot.global_bulk_command_create(commands);
}
