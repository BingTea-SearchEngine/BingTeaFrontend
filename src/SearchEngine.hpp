#pragma once

#include <memory>
#include <vector>
#include "GatewayClient.hpp"

/*
 * @brief: Parses query, communicates with index servers, ranks documents and returns html to
 * respond to client
 * */
class SearchEngine {
   public:
    SearchEngine();

    std::string Search(std::string query);

   private:
    std::vector<std::unique_ptr<Client>> clients;
};
