#include "RenderHTML.hpp"

std::string renderHtml(const std::string& query, const std::vector<std::string>& links) {
    std::ostringstream html;
    html << HTML_HEADER_PRE;
    html << query << HTML_MID_2;

    for (const auto& link : links) {
        html << R"(    <div class="result">
      <a href=")" << link << R"(">)" << link << R"(</a>
    </div>
)";
    }

    html << HTML_FOOTER;
    return html.str();
}
