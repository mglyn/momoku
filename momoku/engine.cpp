#include "engine.h"

Option::Option():
    type("button"),
    min(0),
    max(0) {}

Option::Option(const char* v) :
    type("string"),
    min(0),
    max(0) {
    defaultValue = currentValue = v;
}

Option::Option(bool v) :
    type("check"),
    min(0),
    max(0) {
    defaultValue = currentValue = (v ? "true" : "false");
}

Option::Option(double v, int minv, int maxv) :
    type("spin"),
    min(minv),
    max(maxv) {
    defaultValue = currentValue = std::to_string(v);
}
