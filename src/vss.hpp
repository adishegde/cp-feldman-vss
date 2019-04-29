#ifndef _VSS_HPP
#define _VSS_HPP

#include <vector>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>

#include "constants.hpp"

using namespace NTL;
using namespace std;

class FeldmanVSS {
    static ZZ q;
    static ZZ g;
public:
    vector<ZZ_p> shares;
    vector<ZZ> commits;
    vector<uchar> cipher;

    FeldmanVSS();

    void print();

    static void init();
    static void load(const ZZ &q, const ZZ &g);

    static ZZ get_q();
    static ZZ get_g();

    static ZZ commit(const ZZ_p &a);
    static bool verify(const vector<ZZ> commits, int xval, const ZZ_p &share);

    // Currently relies on the fact that data is at least 16 bytes
    static FeldmanVSS split(const vector<uchar> &data);
    static vector<uchar> reconstruct(const vector<int> &xvec, const vector<ZZ_p> &shares, const vector<uchar> &cipher);
};

#endif
