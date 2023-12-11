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
        std::string priceStr = getFormattedPrice(symbol, true);
        event.reply(priceStr);
    }
    else if (event.command.get_command_name() == "pricegraph")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string period = std::get<std::string>(event.get_parameter("period"));
        std::string mode = std::get<std::string>(event.get_parameter("mode"));

        // Get name of symbol (this will be added to the message instead of just adding the symbol)
        Metrics metrics = fetchMetrics(symbol);
        std::string name = metrics.name;

        // Check if duration is valid and not too short, and add note if it is too short
        std::time_t duration = getDurationInSeconds(period);
        std::string note = "";
        if (duration == 0)
        {
            dpp::message errorMsg{"Invalid period format. Examples of supported formats: 7mo, 1w, 3y, 6d, "
                                  "where mo = month, w = week, y = year, and d = day."};
            event.reply(errorMsg);
        }
        if (duration < 259200)
        {
            period = "3 days";
            note = "Note: period has been set to 3 days, because periods shorter than 3 days may result in an empty graph";
        }
        // Create graph
        priceGraph(symbol, period, std::stoi(mode));

        // Additional delay to make sure the file is fully written to disk
        // Without this delay the bot sends an empty image file
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        // If the file exists, add it to the message
        const std::string imagePath = "../images/price_graph.png";
        if (std::filesystem::exists(imagePath))
        {
            // Add note if the duration has been adjusted
            dpp::message msg{"### Price Graph for " + name + "\n" + note};
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

        // Get name of symbol (this will be added to the message instead of just adding the symbol)
        Metrics metrics = fetchMetrics(symbol);
        std::string name = metrics.name;

        // Check if duration is valid and in case it is too long, add note
        std::time_t duration = getDurationInSeconds(period);
        std::string note = "";
        if (duration == 0)
        {
            dpp::message errorMsg{"Invalid period format. Examples of supported formats: 7mo, 1w, 3y, 6d, "
                                  "where mo = month, w = week, y = year, and d = day."};
            event.reply(errorMsg);
        }
        if (duration > 31536000)
        {
            note = "To ensure readability of the chart, a period of longer than 1 year is not recommended.";
        }
        
        // Create candlestick chart
        (showV == "n") ? createCandle(symbol, period) : createCandleAndVolume(symbol, period);
        
        // Additional delay to make sure the file is fully written to disk
        // Without this delay the bot sends an empty image file
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});

        // If the file exists, add it to the message
        std::string imagePath = (showV == "n") ? "../images/candle_chart.png" : "../images/candle_volume.png";
        if (std::filesystem::exists(imagePath))
        {
            dpp::message msg{"### Candlestick chart for " + name + "\n" + note};
            msg.add_file(showV == "n" ? "candle_chart.png" : "candle_volume.png", dpp::utility::read_file(imagePath));
            event.reply(msg);
            
            // Delete the file after sending the message
            std::filesystem::remove(imagePath);
        }
        else
        {
            // If the file doesn't exist, reply with an error message
            dpp::message errorMsg{"Oops! Something went wrong while creating the candlestick chart."};
            event.reply(errorMsg);
        }
    }
    else if (event.command.get_command_name() == "metrics")
    {
        std::string symbol = std::get<std::string>(event.get_parameter("symbol"));
        std::string metrics = getFormattedMetrics(symbol, true);
        event.reply(metrics);
    }
    else if (event.command.get_command_name() == "majorindices")
    {
        std::string region = std::get<std::string>(event.get_parameter("region"));
        std::string description = std::get<std::string>(event.get_parameter("description"));
        bool showDesc = (description == "n") ? false : true;
        event.reply(getFormattedJSON("../data/indices.json", region, true, showDesc));
    }
    else if (event.command.get_command_name() == "commodities")
    {
        event.reply(getFormattedJSON("../data/commodities.json", "commodities", true));
    }
    else if (event.command.get_command_name() == "currencies")
    {
        event.reply(getFormattedJSON("../data/currencies.json", "currencies", true));
    }
    else if (event.command.get_command_name() == "industries")
    {
        std::string industry = std::get<std::string>(event.get_parameter("industry"));
        event.reply(getFormattedJSON("../data/industries.json", industry, true));
    }
    else if (event.command.get_command_name() == "crypto")
    {
        event.reply(getFormattedJSON("../data/currencies.json", "cryptocurrencies", true));
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
    // Create slash command for latestprice
    dpp::slashcommand latestprice("latestprice", "Get the latest price of a stock, future, index or crypto", bot.me.id);
    latestprice.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));

    // Create slash command for pricegraoh
    dpp::slashcommand pricegraph("pricegraph", "Get a graph of the closing and/or open price of a stock, future, index or crypto", bot.me.id);
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "period", "Period (e.g. 10d, 2w, 3mo, 1y)", true));
    pricegraph.add_option(
        dpp::command_option(dpp::co_string, "mode", "Price type", true).
        add_choice(dpp::command_option_choice("Only open", std::string("1"))).
        add_choice(dpp::command_option_choice("Only close", std::string("2"))).
        add_choice(dpp::command_option_choice("Both", std::string("3"))));

    // Create slash command for candlestick
    dpp::slashcommand candlestick("candlestick", "Get a candlestick chart for a stock, future, index or crypto (optionally with volumes)", bot.me.id);
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "period", "Period (e.g. 10d, 2w, 3mo)", true));
    candlestick.add_option(
        dpp::command_option(dpp::co_string, "volume", "Show volumes", true).
        add_choice(dpp::command_option_choice("Yes", std::string("y"))).
        add_choice(dpp::command_option_choice("No", std::string("n"))));
    
    // Create slash command for majorindices
    dpp::slashcommand majorindices("majorindices", "Get the latest price info for major indices of a certain region", bot.me.id);
    majorindices.add_option(
        dpp::command_option(dpp::co_string, "region", "Region", true).
        add_choice(dpp::command_option_choice("Asia", std::string("Asia"))).
        add_choice(dpp::command_option_choice("Europe", std::string("EU"))).
        add_choice(dpp::command_option_choice("US", std::string("US"))));
    majorindices.add_option(
        dpp::command_option(dpp::co_string, "description", "Short description about the indices", true).
        add_choice(dpp::command_option_choice("Yes", std::string("y"))).
        add_choice(dpp::command_option_choice("No", std::string("n"))));

    // Create slash command for metrics
    dpp::slashcommand metrics("metrics", "Get metrics of a stock, future or index", bot.me.id);
    metrics.add_option(
        dpp::command_option(dpp::co_string, "symbol", "Symbol", true));

    // Create slash command for commodities
    dpp::slashcommand commodities("commodities", "Get the latest price info for different commodities", bot.me.id);

    // Create slash command for currencies
    dpp::slashcommand currencies("currencies", "Get the latest currency rates and USD index", bot.me.id);

    // Create slash command for industries
    dpp::slashcommand industries("industries", "Get the latest price info for major companies of a certain industry", bot.me.id);
    industries.add_option(
        dpp::command_option(dpp::co_string, "industry", "Industry", true).
        add_choice(dpp::command_option_choice("Technology", std::string("Technology"))).
        add_choice(dpp::command_option_choice("Automotive", std::string("Automotive"))).
        add_choice(dpp::command_option_choice("Oil and Gas", std::string("Oil and Gas"))).
        add_choice(dpp::command_option_choice("Chip Companies", std::string("Chip Companies"))).
        add_choice(dpp::command_option_choice("Financial Services", std::string("Financial Services"))).
        add_choice(dpp::command_option_choice("Consumer Goods (Retail)", std::string("Consumer Goods"))).
        add_choice(dpp::command_option_choice("Entertainment and Media", std::string("Entertainment and Media"))).
        add_choice(dpp::command_option_choice("Pharmaceuticals and Healthcare", std::string("Pharmaceuticals and Healthcare"))));

    // Create slash command for crypto
    dpp::slashcommand crypto("crypto", "Get the latest price info for the 5 biggest cryptocurrencies (by market cap.)", bot.me.id);
    
    // Add commands to vector and register them
    std::vector<dpp::slashcommand> commands;
    commands.push_back(latestprice);
    commands.push_back(pricegraph);
    commands.push_back(candlestick);
    commands.push_back(majorindices);
    commands.push_back(metrics);
    commands.push_back(commodities);
    commands.push_back(currencies);
    commands.push_back(industries);
    commands.push_back(crypto);
    bot.global_bulk_command_create(commands);
}
