#pragma once

#include "IndexCell.h"
#include "RequestCell.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class Engine {
public:
    Engine() = default;
    ~Engine() = default;

    void run();
    std::vector<std::string> getRequest(const std::string& query, const std::string& extension);
    std::unordered_set<fs::path> getExtensions();
    void setHomeDir(int argc, char* argv[]);
    void setTopK(int k);


private:
    // index functions
    std::vector<fs::path> paths_to_files;
    std::vector<size_t> files_size;
    std::unordered_map<std::string, std::vector<IndexCell>> index;
    std::unordered_set<fs::path> files_extensions;
    std::error_code ec;
    std::string home_dir;
    double avg_doc_len = 0.0;
    void collectFiles();
    void buildIndex();
    void parseDirectory(fs::path dir);

    // search functions
    int top_k = 3;
    std::string query;
    std::set<RequestCell> top_docs;
    double bm_25(int did, double doc_frequency, double term_frequency);
    std::set<RequestCell> parseRequest(const std::string& request, const std::string& extension);
    std::vector<std::string> lexemes;
    std::vector<std::string>::iterator cur_lexeme;
    std::set<RequestCell> unite(const std::set<RequestCell>& a, const std::set<RequestCell>& b);
    std::set<RequestCell> intersection(const std::set<RequestCell>& a, const std::set<RequestCell>& b);
    std::set<RequestCell> OR(const std::string& extension);
    std::set<RequestCell> AND(const std::string& extension);
    std::set<RequestCell> FACTOR(const std::string& extension);
    std::string to_low(const std::string& s);
    void checkQuery();
    bool request_error = false;
};
