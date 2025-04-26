#include "RenderHTML.hpp"

std::string clean_html_unsafe(const std::string& input) {
    std::string cleaned = input;

    // Remove everything between < and > including the brackets
    cleaned = std::regex_replace(cleaned, std::regex("<[^>]*>"), "");

    // Remove stray < and > characters
    cleaned = std::regex_replace(cleaned, std::regex("[<>]"), "");

    // Remove all non-alphanumeric and non-whitespace characters
    cleaned = std::regex_replace(cleaned, std::regex("[^\\w\\s]"), "");

    return cleaned;
}

std::string html_escape(const std::string& text) {
    std::ostringstream escaped;
    for (char c : text) {
        switch (c) {
            case '&':  escaped << "&amp;";  break;
            case '\"': escaped << "&quot;"; break;
            case '\'': escaped << "&#39;";  break;
            case '<':  escaped << "&lt;";   break;
            case '>':  escaped << "&gt;";   break;
            default:   escaped << c;        break;
        }
    }
    return escaped.str();
}

std::string renderHtml(const std::string& query, const std::vector<doc_t>& documents) {
    std::ostringstream html;
    html << HTML_HEADER_PRE;
    html << html_escape(query) << HTML_MID_2;

    for (const auto& document : documents) {
        std::string cleanedTitle = clean_html_unsafe(document.title);
        std::string cleanedSnippet = clean_html_unsafe(document.snippet);

        html << R"(    <div class="result">
          <a href=")" << document.url << R"(" class="title">)" << cleanedTitle << R"(</a><br>
          <div class="url">)" << document.url << R"(</div>
          <div class="description">)" << cleanedSnippet << R"(</div>
        </div>
    )";
    }

    html << HTML_FOOTER;
    return html.str();
}
