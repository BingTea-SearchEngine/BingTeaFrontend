#include "SearchEngine.hpp"

#define safe_xgboost(call) {  \
    int err = (call); \
    if (err != 0) { \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + \
                          ": error in " + #call + ":" + XGBGetLastError());  \
    } \
}

SearchEngine::SearchEngine(std::string ipPath, std::string modelPath) : modelPath(modelPath) {
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

std::vector<doc_t> SearchEngine::rank(std::vector<doc_t> input) {
    std::vector<float> data;
    unsigned num_rows = 0;
    unsigned num_cols = 8;
    for (const doc_t& page: input) {
        data.push_back(page.numWords);
        data.push_back(page.numTitleWords);
        data.push_back(page.numOutLinks);
        data.push_back(page.numTitleMatch);
        data.push_back(page.numBodyMatch);
        data.push_back(page.pageRank);
        data.push_back(page.cheiRank);
        data.push_back(page.rankingScore);
        num_rows++;
    }

    DMatrixHandle dmat;
    // Valgrind says there a memory leak here. IDFK how, I free at the end.
    safe_xgboost(XGDMatrixCreateFromMat(data.data(), num_rows, num_cols, -1, &dmat));

    BoosterHandle booster;
    const char *model_path = modelPath.c_str();

    // create booster handle first
    safe_xgboost(XGBoosterCreate(NULL, 0, &booster));

    // set the model parameters here

    // load model
    // Valgrind says there a memory leak here. IDFK how, I free at the end.
    safe_xgboost(XGBoosterLoadModel(booster, model_path));

    char const config[] =
        "{\"training\": false, \"type\": 0, "
        "\"iteration_begin\": 0, \"iteration_end\": 0, \"strict_shape\": false}";
    /* Shape of output prediction */
    uint64_t const* out_shape;
    /* Dimension of output prediction */
    uint64_t out_dim;
    /* Pointer to a thread local contiguous array, assigned in prediction function. */
    float const* predictions = NULL;
    safe_xgboost(XGBoosterPredictFromDMatrix(booster, dmat, config, &out_shape, &out_dim, &predictions));

    // DO SOMETHING WITH predicitions
    std::vector<unsigned> indices;
    for (unsigned i = 0; i < num_rows; ++i) {
        indices.push_back(i);
    }
    indices.shrink_to_fit();
    auto comp = [predictions](size_t first, size_t second) {
        return predictions[first] > predictions[second];
    };
    std::partial_sort(indices.begin(), indices.begin() + 10,
                      indices.end(), comp);
    std::sort(indices.begin(), indices.begin() + 10, comp);
    indices.resize(10);
    std::vector<doc_t> top10;
    for (unsigned i = 0; i < 10; ++i) {
        top10.push_back(input[indices[i]]);
    }

    safe_xgboost(XGDMatrixFree(dmat));
    safe_xgboost(XGBoosterFree(booster));
    return top10;
}

