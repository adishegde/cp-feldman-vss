#include <vector>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <openssl/sha.h>

#include "vss.hpp"

using namespace NTL;
using namespace std;

// Utility function to compute SHA256
void sha256(uchar *digest, uchar *data, long len) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(digest, &sha256);
}

// Static member definitions
ZZ FeldmanVSS::q;
ZZ FeldmanVSS::g;

FeldmanVSS::FeldmanVSS() : shares(4), commits(2), cipher(SECRET_LEN) {
}

void FeldmanVSS::init() {
    // Generate prime q such that 2q+1 is also a prime
    // In all code below, p := 2q + 1
    q = GenGermainPrime_ZZ(P_LEN);
    ZZ_p::init(q);

    // We find b in Z_p^*
    ZZ p = 2 * q + 1;
    ZZ b = RandomBnd(p);
    while (b == 0) {
        b = RandomBnd(p);
    }
    // We square b so that it is a quadratic residue and belongs to
    // prime order group with order q. This becomes g, our generator.
    g = SqrMod(b, p);
}

void FeldmanVSS::load(const ZZ &q, const ZZ &g) {
    FeldmanVSS::q = q;
    ZZ_p::init(q);

    FeldmanVSS::g = g;
}

ZZ FeldmanVSS::get_q() {
    return q;
}

ZZ FeldmanVSS::get_g() {
    return g;
}

ZZ FeldmanVSS::commit(const ZZ_p &a) {
    return PowerMod(g, conv<ZZ>(a), 2 * q + 1);
}

bool FeldmanVSS::verify(const vector<ZZ> commits, int xval, const ZZ_p &share) {
    if (commits.size() < 2) {
        throw "verify: Expected 2 commits";
    }

    ZZ p = 2 * q + 1;
    // Commits are multiplied in modulus p while exponents are
    // calculated in modulus q
    ZZ acc = MulMod(
        commits[0],
        power(commits[1], xval),
        p
    );
    ZZ share_commit = commit(share);

    return (acc == share_commit);
}

FeldmanVSS FeldmanVSS::split(const vector<uchar> &secret) {
    // Throw error if secret is not long enough
    if (secret.size() < SECRET_LEN) {
        throw "split: Secret not long enough";
    }

    // Generate random linear polynomial 'f' for Shamir SS
    // f = (c[0] + c[1]x) mod q
    ZZ_pX f = random_ZZ_pX(2);

    FeldmanVSS out;

    // Computing shares for each party
    // shares[i] := f(i) mod q, for i in [1, 4]
    for(int i = 1; i <= 4; ++i) {
        out.shares[i - 1] = eval(f, ZZ_p(i));
    }

    // Computing the commitments for the coefficients
    // commits[i] := g^(c[i]) mod p
    for(int i = 0; i < 2; ++i) {
        out.commits[i] = commit(coeff(f, i));
    }

    // Getting 'data stream' from ZZ_p type for constant term (c[0])
    long len = NumBytes(rep(coeff(f, 0)));
    uchar c[len];
    BytesFromZZ(c, rep(coeff(f, 0)), len);

    // Computing key
    // key := KDF(c[0])
    uchar key[SHA256_DIGEST_LENGTH];
    sha256(key, c, len);

    // Encrypting secret
    // cipher := secret + key
    for(int i = 0; i < SECRET_LEN; ++i) {
        out.cipher[i] = secret[i] ^ key[i];
    }

    return out;
}

vector<uchar> FeldmanVSS::reconstruct(
    const vector<int> &xvec,
    const vector<ZZ_p> &shares,
    const vector<uchar> &cipher
) {
    // Throw error if secret cannot be reconstructed
    if (xvec.size() < 2 || shares.size() < 2 || cipher.size() != SECRET_LEN) {
        throw "reconstruct: Input size shorter than expeced";
    }

    // Lagrange interpolation - simplified for linear polynomial
    ZZ_p c = ((xvec[1] * shares[0]) - (xvec[0] * shares[1])) / (xvec[1] - xvec[0]);

    // Get 'data stream' of constant term
    long len = NumBytes(rep(c));
    uchar c_bytes[len];
    BytesFromZZ(c_bytes, rep(c), len);

    // Compute key from constant term
    // key := KDF(c)
    uchar key[SHA256_DIGEST_LENGTH];
    sha256(key, c_bytes, len);

    // Decrypt cipher
    // secret := cipher + key
    vector<uchar> secret(SECRET_LEN);
    for(int i = 0; i < SECRET_LEN; ++i) {
        secret[i] = cipher[i] ^ key[i];
    }

    return secret;
}
