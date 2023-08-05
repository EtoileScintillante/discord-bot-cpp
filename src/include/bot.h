#ifndef BOT_H
#define BOT_H

#include <dpp/dpp.h>
#include <chrono> 
#include <thread> 
#include <string>
#include <fstream>
#include <vector>
#include "data.h"
#include "visualize.h"

class Bot {
public:
    Bot(const std::string& token);

    void run();

private:
    void setupBot();
    void commandHandler(const dpp::slashcommand_t& event);
    void onReady(const dpp::ready_t& event);
    void registerCommands();

    // Add any other helper functions or member variables you may need here.

    dpp::cluster bot;
};

#endif // BOT_H
