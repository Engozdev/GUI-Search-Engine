#include "Engine.h"

const std::string kAnyExtension = "-";

void Engine::setHomeDir(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Wrong amount of command line arguments\n";
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "-d") == 0) {
        home_dir = argv[2];
    } else {
        std::cerr << "Error in command line arguments\n";
        exit(EXIT_FAILURE);
    }
}

void Engine::parseDirectory(fs::path dir) {
    std::vector<fs::path> new_dirs;
    for (const auto& el: fs::directory_iterator(dir)) {
        if (fs::is_regular_file(el, ec)) {
            paths_to_files.push_back(el);
            files_extensions.insert(el.path().extension());
        }
        if (ec) std::cerr << "Error in file: " << ec.message() << std::endl;

        if (fs::is_directory(el, ec)) {
            parseDirectory(el);
        }
    }
}

void Engine::collectFiles() {
    for (const auto& file: fs::directory_iterator(home_dir)) {
        if (fs::is_regular_file(file, ec)) {
            paths_to_files.push_back(file.path());
        }
        if (ec) std::cerr << "Error in file: " << ec.message() << std::endl;

        if (fs::is_directory(file, ec)) {
            parseDirectory(file);
        }
        if (ec) std::cerr << "Error in directory: " << ec.message() << std::endl;
    }
}

void Engine::buildIndex() {
    collectFiles();
    for (size_t i = 0; i < paths_to_files.size(); ++i) {
        std::ifstream file(paths_to_files[i], std::ifstream::binary);
        size_t words_amount = 0;
        if (file.is_open()) {
            std::string line;
            std::string word;
            size_t file_pos = 0;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                while (std::getline(iss, word, ' ')) {
                    if (word.empty()) continue;
                    // if (ban_words.find(word) != ban_words.end()) continue;
                    words_amount++;
                    if (!index[word].empty()) {
                        if (i != index[word].back().did) {
                            index[word].emplace_back(i, file_pos, 1);
                        } else {
                            index[word].back().tf++;
                        }
                    } else {
                        index[word].emplace_back(i, file_pos, 1); // did, pos, term_freq
                    }
                }
                file_pos = file.tellg();
            }
            files_size.push_back(words_amount);
            avg_doc_len += words_amount;
            file.close();
        } else {
            std::cerr << "Unable to open the file: " << paths_to_files[i] << "\n";
            exit(EXIT_FAILURE);
        }
    }
    avg_doc_len /= paths_to_files.size();
}

double Engine::bm_25(int did, double doc_frequency, double term_frequency) {
    double N = paths_to_files.size();
    double k = 1; // power of influence of repeating
    double b = 0.5; // power of influence of doc length
    double doc_len = files_size[did];
    double score = term_frequency * (k + 1);
    score /= (term_frequency + k * (1 - b + b * (doc_len / avg_doc_len)));
    score *= std::log((N - doc_frequency + 0.5) / (doc_frequency + 0.5));
    return score;
}


std::set<RequestCell> Engine::parseRequest(const std::string& request, const std::string& extension) {
    top_docs.clear();
    for (const auto& el: index[request]) {
        if (extension == kAnyExtension || paths_to_files[el.did].extension().string() == extension) {
            double score = bm_25(el.did, index[request].size(), el.tf);
            if (top_docs.size() < top_k) {
                top_docs.insert(RequestCell(score, el.did, el.pos));
            } else if (top_docs.begin()->score < score) {
                top_docs.erase(*top_docs.rbegin());
                top_docs.insert(RequestCell(score, el.did, el.pos));
            }
        }
    }
    return top_docs;
}

std::set<RequestCell> Engine::unite(const std::set<RequestCell>& a, const std::set<RequestCell>& b) {
    std::set<RequestCell> res;
    if (a.size() < b.size()) {
        res = b;
        for (auto& el: a) {
            if (el.score > res.begin()->score) {
                res.erase(*res.rbegin());
                res.insert(el);
            }
        }
    } else {
        res = a;
        for (auto& el: b) {
            if (el.score > res.begin()->score) {
                res.erase(*res.rbegin());
                res.insert(el);
            }
        }
    }
    return res;
}

std::set<RequestCell> Engine::intersection(const std::set<RequestCell>& a, const std::set<RequestCell>& b) {
    std::set<RequestCell> res;
    std::set<RequestCell>::iterator beg1, end1, beg2, end2;
    std::set<int> doc_ids;
    if (a.size() < b.size()) {
        for (auto& el: b) {
            doc_ids.insert(el.did);
        }
        for (auto& el: a) {
            if (doc_ids.find(el.did) != doc_ids.end()) {
                res.insert(el);
            }
        }
    } else {
        for (auto& el: a) {
            doc_ids.insert(el.did);
        }
        for (auto& el: b) {
            if (doc_ids.find(el.did) != doc_ids.end()) {
                res.insert(el);
            }
        }
    }
    return res;
}

std::set<RequestCell> Engine::OR(const std::string& extension) {
    if (request_error) return {};
    std::set<RequestCell> value = AND(extension);
    while (true) {
        cur_lexeme++;
        if (*cur_lexeme == "OR") {
            value = unite(value, AND(extension));
        } else {
            cur_lexeme--;
            return value;
        }
    }
}

std::set<RequestCell> Engine::AND(const std::string& extension) {
    if (request_error) return {};
    std::set<RequestCell> value = FACTOR(extension);
    while (true) {
        cur_lexeme++;
        if (*cur_lexeme == "AND") {
            value = intersection(value, FACTOR(extension));
        } else {
            cur_lexeme--;
            return value;
        }
    }
}

std::set<RequestCell> Engine::FACTOR(const std::string& extension) {
    if (request_error) return {};
    cur_lexeme++;
    if (*cur_lexeme == "(") {
        std::set<RequestCell> value = OR(extension);
        cur_lexeme++;
        return value;
    } else {
        if (cur_lexeme == lexemes.end() || *cur_lexeme == "AND" || *cur_lexeme == "OR") {
            request_error = true;
            return {};
        }
        return parseRequest(*cur_lexeme, extension);
    }
}

std::string Engine::to_low(const std::string& s) {
    std::string res;
    for (auto c: s) res += std::tolower(c);
    return res;
}

void Engine::checkQuery() {
    if (lexemes.size() > 1) {
        if ((lexemes[lexemes.size() - 1] == "AND" && lexemes[lexemes.size() - 2] == "OR") ||
            (lexemes[lexemes.size() - 1] == "OR" && lexemes[lexemes.size() - 2] == "AND")) {
            request_error = true;
        }
        if (lexemes.size() > 0) {
            if ((lexemes[lexemes.size() - 1] != "OR" && to_low(lexemes[lexemes.size() - 1]) == "or") ||
                lexemes[lexemes.size() - 1] != "AND" && to_low(lexemes[lexemes.size() - 1]) == "and") {
                request_error = true;
            }
        }
    }
}

std::vector<std::string> Engine::getRequest(const std::string& query, const std::string& extension) {
    std::vector<std::string> res;
    std::string s;
    std::istringstream iss(query);
    std::vector<std::string> v;
    while (getline(iss, s, ' ')) {
        v.emplace_back(s.c_str());
    }
    lexemes.clear();
    for (auto& el: v) {
        if (el[0] == '(') {
            if (el.back() == ')') {
                lexemes.emplace_back(el.substr(1, el.size() - 2));
                checkQuery();
                lexemes.emplace_back(")");
            } else {
                lexemes.emplace_back(el.substr(1));
                checkQuery();
            }
        } else if (el.back() == ')') {
            lexemes.emplace_back(el.substr(0, el.size() - 1));
            checkQuery();
            lexemes.emplace_back(")");
        } else {
            lexemes.emplace_back(el);
            checkQuery();
        }
        if (request_error) break;
    }
    cur_lexeme = lexemes.begin() - 1;
    request_error = false;
    std::set<RequestCell> response = OR(extension);
    if (request_error) {
        res = {"Invalid request\n"};
    } else if (response.empty()) {
        res = {"Nothing found"};
    } else {
        for (auto& doc: top_docs) {
            res.push_back(paths_to_files[doc.did].string());
            std::ifstream in(paths_to_files[doc.did], std::ifstream::binary);
            in.seekg(doc.pos, std::ios::beg);
            std::string line;
            char c;
            while (in.get(c)) {
                if (c == '\n') break;
                line += c;
            }
            res.push_back(line);
            in.close();
        }
    }
    return res;
}

std::unordered_set<fs::path> Engine::getExtensions() {
    return files_extensions;
}

void Engine::setTopK(int k) {
    top_k = k;
}

void Engine::run() {
    buildIndex();
}
