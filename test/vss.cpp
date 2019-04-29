#include <vector>
#include <string>
#include <iostream>
#include <NTL/ZZ_p.h>

#include "../src/vss.hpp"

using namespace std;

int main() {
	FeldmanVSS::init();

	cout<<"Prime (q): "<<FeldmanVSS::get_q()<<endl;
	cout<<"Generator: "<<FeldmanVSS::get_g()<<endl<<endl;

	string m = "Cryptography VSS";

	vector<uchar> secret(SECRET_LEN);
	for(size_t i = 0; i < m.length(); ++i) {
		secret[i] = m[i];
	}

	cout<<"Original Secret: "<<m<<endl<<endl;

	FeldmanVSS v = FeldmanVSS::split(secret);

	for(int i = 0; i < 4; ++i) {
		if (FeldmanVSS::verify(v.commits, i+1, v.shares[i])) {
			cout<<"Share "<<(i+1)<<": verified successfully.\n";
		} else {
			cout<<"Share "<<(i+1)<<": verification failed.\n";
		}
	}
	cout<<endl;

	vector<int> xvec{2, 3};
	vector<ZZ_p> shares;

	for(auto i : xvec) {
		shares.push_back(v.shares[i-1]);
	}

	vector<uchar> ans = FeldmanVSS::reconstruct(xvec, shares, v.cipher);

	cout<<"Reconstructed Secret: ";
	bool same = true;
	for(size_t i = 0; i < secret.size(); ++i) {
		cout<<ans[i];
		if (ans[i] != secret[i]) {
			same = false;
		}
	}
	cout<<endl<<endl;

	if (!same) {
		cout<<"Reconstruction Failed!"<<endl;
	} else {
		cout<<"Reconstruction Successful!"<<endl;
	}

	return 0;
}
