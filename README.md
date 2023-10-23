# equity-bot-discord
[![macOS Build Status](https://github.com/EtoileScintillante/discord-bot-cpp/workflows/Build-macOS/badge.svg)](https://github.com/EtoileScintillante/discord-bot-cpp/actions)  
Discord bot written in C++ with the [D++/DPP library](https://github.com/brainboxdotcc/DPP).  
It can fetch equity (stock/future/index) data and visualize it.  
All data is fetched from [Yahoo Finance](https://finance.yahoo.com).   
Important to note is that data may be delayed.

## Slash Commands (so far)
### /latestprice  
  - Fetches latest price data
  - Input: equity symbol

<img src="docs/latestprice.png" alt="latest price example" width="300" height="auto" />

### /metrics  
  - Fetches equity metrics  
  - Input: equity symbol

<img src="docs/metrics.png" alt="metrics example" width="350" height="auto" />

### /majorindices
  - Fetches latest price data for certain major indices based on the given region  
    and optionally adds a short description (one sentence) about the indices  
  - Input: region (US/EU/Asia), description (yes/no)

<img src="docs/majorindices.png" alt="indices US example" width="330" height="auto" />

### /pricegraph 
  - Fetches price data and plots the open and/or close prices
  - Input: equity symbol, period (e.g. 6 months, 2 weeks, etc.), mode (only open/only close/both)

<img src="docs/pricegraph.png" alt="price graph example" width="500" height="auto" />

### /candlestick
  - Fetches OHLC data and creates a candlestick chart, optionally with volumes
  - Input: equity symbol, period (e.g. 6 months, 2 weeks, etc.), volume (yes/no)

<img src="docs/candlestick.png" alt="candlestick example" width="500" height="auto" />

## Building and Running the Bot (macOS) 
### Prerequisites

- [CMake](https://cmake.org) (version 3.5 or higher)
- C++ Compiler with C++17 support
- [Git](https://git-scm.com)
- Discord Bot Token (add this to a .config file)

### Instructions
1. Clone the repository

    ```bash
    git clone https://github.com/EtoileScintillante/discord-bot-cpp.git
    cd discord-bot-cpp
    ```

2. Build and Run the Bot

    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ./equity-bot
    ```
