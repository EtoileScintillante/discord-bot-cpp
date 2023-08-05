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
    if (event.command.get_command_name() == "lateststockprice")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string priceStr = latestStockPriceAsString(symbol);
        event.reply(priceStr);
    }
    else if (event.command.get_command_name() == "pricegraph")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string period = std::get<std::string>(event.get_parameter("period"));
        std::string mode = std::get<std::string>(event.get_parameter("mode"));
        dpp::message msg{"Here is your graph for " + symbol};

        // Use std::async to create the graph asynchronously
        auto future = std::async(std::launch::async, [&]()
                                 {
                std::vector<std::vector<std::string>> ohlcData = fetchOHLCData(symbol, period);
                priceGraph(ohlcData, std::stoi(mode)); });

        // Wait for the graph creation to finish
        future.wait();

        // Additional delay to make sure the file is fully written to disk
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        const std::string imagePath = "../images/price_graph.png";
        const int maxAttempts = 5;
        const int timeoutMs = 1000; // 1 second
        int attempts = 0;

        // Check if the file exists, with a timeout
        while (!std::filesystem::exists(imagePath) && attempts < maxAttempts)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{timeoutMs});
            attempts++;
        }

        // If the file exists, add it to the message
        if (std::filesystem::exists(imagePath))
        {
            msg.add_file("price_graph.png", dpp::utility::read_file(imagePath));
            event.reply(msg);
            // Delete the file after sending the message
            std::filesystem::remove(imagePath);
        }
        else
        {
            // If the file doesn't exist, reply with an error message
            dpp::message errorMsg{"Oops! Something went wrong while creating the graph."};
            event.reply(errorMsg);
        }
    }
    else if (event.command.get_command_name() == "candlestick")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string period = std::get<std::string>(event.get_parameter("period"));
        std::string showV = std::get<std::string>(event.get_parameter("volume"));

        // Use std::async to create the graph asynchronously
        auto future = std::async(std::launch::async, [&]()
                                 {
                std::vector<std::vector<std::string>> ohlcData = fetchOHLCData(symbol, period);
                 if (showV == "n")
                {
                    createCandle(ohlcData);
                }
                else
                {
                    createCandleAndVolume(ohlcData);
                }});

        // Wait for the graph creation to finish
        future.wait();

        // Additional delay to make sure the file is fully written to disk
        std::this_thread::sleep_for(std::chrono::milliseconds{500});

        std::string imagePath;
        if (showV == "n")
        {
            imagePath = "../images/candle_chart.png";
        }
        else
        {
            imagePath = "../images/candle_volume.png";
        }
        const int maxAttempts = 5;
        const int timeoutMs = 1000; // 1 second
        int attempts = 0;

        // Check if the file exists, with a timeout
        while (!std::filesystem::exists(imagePath) && attempts < maxAttempts)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{timeoutMs});
            attempts++;
        }

        // If the file exists, add it to the message
        if (std::filesystem::exists(imagePath))
        {
            if (showV == "n")
            {
                dpp::message msg{"Here is your candlestick chart for " + symbol};
                msg.add_file("candle_chart.png", dpp::utility::read_file(imagePath));
                event.reply(msg);
            }
            else
            {
                dpp::message msg{"Here are your candlestick chart and volume graph for " + symbol};
                msg.add_file("candle_volume.png", dpp::utility::read_file(imagePath));
                event.reply(msg);
            }
            
            // Delete the file after sending the message
            std::filesystem::remove(imagePath);
        }
        else
        {
            // If the file doesn't exist, reply with an error message
            dpp::message errorMsg{"Oops! Something went wrong while creating the candlestick chart (and volume graph)."};
            event.reply(errorMsg);
        }
    }
    // New commands here...
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
    // create slash command
    dpp::slashcommand newcommand("lateststockprice", "Get the latest price of a stock from Yahoo Finance", bot.me.id);
    newcommand.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Stock symbol", true));

    // Create slash command
    dpp::slashcommand newcommand1("pricegraph", "Get a closing and/or open price graph for a stock", bot.me.id);
    newcommand1.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Stock symbol", true));
    newcommand1.add_option(
        dpp::command_option(dpp::co_string, "period", "Period/time span", true).
        add_choice(dpp::command_option_choice("1 year", std::string("1y"))).
        add_choice(dpp::command_option_choice("6 months", std::string("6mo"))).
        add_choice(dpp::command_option_choice("3 months", std::string("3mo"))).
        add_choice(dpp::command_option_choice("2 months", std::string("2mo"))).
        add_choice(dpp::command_option_choice("1 month", std::string("1mo"))).
        add_choice(dpp::command_option_choice("3 weeks", std::string("3w"))).
        add_choice(dpp::command_option_choice("2 weeks", std::string("2w"))).
        add_choice(dpp::command_option_choice("1 week", std::string("1w"))));
    newcommand1.add_option(
        dpp::command_option(dpp::co_string, "mode", "Price type", true).
        add_choice(dpp::command_option_choice("Only open", std::string("1"))).
        add_choice(dpp::command_option_choice("Only close", std::string("2"))).
        add_choice(dpp::command_option_choice("Both", std::string("3"))));

    // Create slash command
    dpp::slashcommand newcommand2("candlestick", "Get a candlestick for a stock (and optionally with volume graph)", bot.me.id);
    newcommand2.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Stock symbol", true));
    newcommand2.add_option(
        dpp::command_option(dpp::co_string, "period", "Period/time span", true).
        add_choice(dpp::command_option_choice("6 months", std::string("6mo"))).
        add_choice(dpp::command_option_choice("3 months", std::string("3mo"))).
        add_choice(dpp::command_option_choice("2 months", std::string("2mo"))).
        add_choice(dpp::command_option_choice("1 month", std::string("1mo"))).
        add_choice(dpp::command_option_choice("3 weeks", std::string("3w"))).
        add_choice(dpp::command_option_choice("2 weeks", std::string("2w"))).
        add_choice(dpp::command_option_choice("1 week", std::string("1w"))));
    newcommand2.add_option(
        dpp::command_option(dpp::co_string, "volume", "Show volumes (creates additional graph with volume data)", true).
        add_choice(dpp::command_option_choice("Yes", std::string("y"))).
        add_choice(dpp::command_option_choice("No", std::string("n"))));

    // Add commands to vector and register them
    std::vector<dpp::slashcommand> commands;
    commands.push_back(newcommand);
    commands.push_back(newcommand1);
    commands.push_back(newcommand2);
    bot.global_bulk_command_create(commands);
}
