#pragma once

#include <iostream>

class IndexCell {
public:
    IndexCell() = default;

    IndexCell(int did_, size_t pos_, int tf_) : did(did_), pos(pos_), tf(tf_) {}

    int did, tf;
    size_t pos;
};
