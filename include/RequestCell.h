#pragma once

#include <iostream>

class RequestCell {
public:
    RequestCell() = default;

    RequestCell(double score_, int did_, size_t pos_) : score(score_), did(did_), pos(pos_) {}

    friend bool operator<(const RequestCell& a, const RequestCell& b) {
        return a.score > b.score;
    }

    double score;
    int did;
    size_t pos;
};
