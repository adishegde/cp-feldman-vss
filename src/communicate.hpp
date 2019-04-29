#ifndef _COMMUNICATE_H
#define _COMMUNICATE_H

#include <vector>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>

#include "constants.hpp"

using namespace NTL;
using namespace std;

// Data related to a party's share
class Share {
public:
    uchar share[STORAGE_LEN];
    uchar commits[2][STORAGE_LEN];
    uchar cipher[SECRET_LEN];

    ZZ_p get_share();
    vector<ZZ> get_commits();
    vector<uchar> get_cipher();

    void set_share(const ZZ_p &share);
    void set_commits(const vector<ZZ> &commits);
    void set_cipher(const vector<uchar> &cipher);
};

// Data common to everyone - output of Gen(1^n)
class Init {
public:
    uchar q[STORAGE_LEN];
    uchar g[STORAGE_LEN];

    ZZ get_q();
    ZZ get_p();

    void set_q(const ZZ &zq);
    void set_g(const ZZ &zg);
};

#endif
