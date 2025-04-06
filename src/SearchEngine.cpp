#include "SearchEngine.hpp"

SearchEngine::SearchEngine() {}

std::string SearchEngine::Search(std::string query) {
    std::string result = "<h1>Results for: " + query + "</h1>";
    return result;
}
