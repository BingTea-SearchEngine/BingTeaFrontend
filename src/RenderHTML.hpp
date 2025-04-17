#pragma once

#include <string>
#include <vector>
#include <sstream>

std::string renderHtml(const std::string& query, const std::vector<std::string>& links);

const std::string HTML_HEADER_PRE = R"(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Bing Tea Results</title>
  <style>
    body {
      background-color: #202124;
      color: #e8eaed;
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
    }

    .logo-bar {
      display: flex;
      align-items: center;
      padding: 24px;
    }

    .logo-bar .logo {
      height: 100px;
      margin-right: 16px;
    }

    .main {
      max-width: 700px;
      margin: 0 auto;
    }

    .search-bar {
      margin-top: 12px;
      margin-bottom: 32px;
    }

    .search-bar input {
      width: 100%;
      padding: 12px 20px;
      font-size: 16px;
      border-radius: 24px;
      border: none;
      background-color: #303134;
      color: white;
    }

    .search-results {
      margin-bottom: 40px;
    }

    .result {
      margin-bottom: 32px;
    }

    .result a {
      color: #8ab4f8;
      font-size: 18px;
      text-decoration: none;
    }

    .result a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="main">
    <div class="logo-bar">
      <a href="/"><img src="/bingtealogo.png" alt="Bing Tea Logo" class="logo"></a>
    </div>
    <div class="search-bar">
      <form action="/search" method="GET">
        <input type="text" name="query" value=")";

const std::string HTML_MID_2 = R"(" placeholder="Search Bing Tea..." />
      </form>
    </div>

    <div class="search-results">
)";

const std::string HTML_FOOTER = R"(
    </div>
  </div>
</body>
</html>
)";
