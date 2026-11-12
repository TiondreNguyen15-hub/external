#include "projection.hpp"
#include <iostream>
#include <limits>
using namespace visualizer;
int main() {
    const Matrix identity{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    int failures = 0;
    auto check = [&](bool ok, const char* name) { if (!ok) { std::cerr << name << '\n'; ++failures; } };
    auto center = project({0,0,0},identity,1920,1080);
    check(center && center->x == 960 && center->y == 540,"center");
    auto top = project({-1,1,0},identity,1920,1080);
    check(top && top->x == 0 && top->y == 0,"axis convention");
    check(!project({2,0,0},identity,1920,1080),"offscreen rejection");
    Matrix behind = identity; behind[15] = -1;
    check(!project({0,0,0},behind,1920,1080),"behind camera");
    check(!project({0,0,0},identity,0,1080),"invalid viewport");
    check(!project({std::numeric_limits<float>::quiet_NaN(),0,0},identity,1920,1080),"NaN input");
    Matrix perspective = identity; perspective[15] = 2;
    auto p = project({1,1,0},perspective,100,100);
    check(p && p->x == 75 && p->y == 25,"perspective divide");
    return failures ? 1 : 0;
}
