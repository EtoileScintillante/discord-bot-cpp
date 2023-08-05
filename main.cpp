#include "bot.h"

int main()
{
    // Read bot token from .config file
    std::ifstream configFile("../.config");
    std::string BOT_TOKEN;
    if (configFile.is_open())
    {
        std::getline(configFile, BOT_TOKEN);
        configFile.close();
    }
    else
    {
        std::cout << "Error: Could not open .config file.\n";
        return 1;
    }

    // Create bot and run it
    Bot bot(BOT_TOKEN);
    bot.run();

    return 0;
}
