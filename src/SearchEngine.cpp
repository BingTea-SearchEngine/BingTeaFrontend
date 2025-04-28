#include "SearchEngine.hpp"

SearchEngine::SearchEngine(std::string ipPath) {
    std::ifstream ipFile(ipPath);
    std::string line;
    while (std::getline(ipFile, line)) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string ip = line.substr(0, colonPos);
            int port = std::stoi(line.substr(colonPos + 1));
            _clients.push_back(std::move(std::make_unique<Client>(ip, port)));
        } else {
            spdlog::error("Error parsing IP and port {}", line);
        }
    }
}

std::string SearchEngine::Search(std::string query) {
    std::lock_guard<std::mutex> lock(_m);

    if (_clients.size() == 0) {
        spdlog::error("Lost connection to all index servers, exiting");
        exit(EXIT_FAILURE);
    }

    spdlog::info("Search: {}", query);
    IndexMessage queryMsg{IndexMessageType::QUERY, query, {}};
    std::string encoded = IndexInterface::Encode(queryMsg);

    for (size_t i = 0; i < _clients.size(); ++i) {
        _clients[i]->SendMessage(encoded);
    }

    spdlog::info("Sent messages");

    std::vector<doc_t> results;
    std::vector<size_t> deadServers;
    for (size_t i = 0; i < _clients.size(); ++i) {
        std::optional<Message> msg = _clients[i]->GetMessageBlocking();
        if (msg) {
            spdlog::info("Reply from {}:{}", msg->senderIp, msg->senderPort);
            IndexMessage reply = IndexInterface::Decode(msg->msg);
            if (reply.type != IndexMessageType::DOCUMENTS) {
                spdlog::error("Wrong message reply type");
                continue;
            }
            results.insert(results.end(), reply.documents.begin(), reply.documents.end());
        } else {
            spdlog::error("Error getting from index server {}", i);
            deadServers.push_back(i);
        }
    }

    for (size_t deadServer : deadServers) {
        spdlog::info("Removing {} from clients list", deadServer);
        _clients.erase(_clients.begin() + deadServer);
    }

    if (_clients.size() == 0) {
        spdlog::error("Lost connection to all index servers, exiting");
        exit(EXIT_FAILURE);
    }

    // RANK
    std::sort(results.begin(), results.end(), [](const doc_t& a, const doc_t& b) {
        return a.rankingScore > b.rankingScore;
    });

    return renderHtml(query, results);
}
