#pragma once

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <memory>
#include <vector>
#include <mutex>

#include "GatewayClient.hpp"
#include "IndexInterface.hpp"
#include "RenderHTML.hpp"

/*
 * @brief: Parses query, communicates with index servers, ranks documents and returns html to
 * respond to client
 * */
class SearchEngine {
   public:
    SearchEngine(std::string ipPath);

    std::string Search(std::string query);

   private:
    std::vector<std::unique_ptr<Client>> _clients;

    std::mutex _m;
};
