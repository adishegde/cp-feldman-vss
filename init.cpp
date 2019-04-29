#include "src/constants.hpp"
#include "src/communicate.hpp"
#include "src/vss.hpp"

int main() {
	FeldmanVSS::init();

	cout<<"Prime: "<<FeldmanVSS::get_q()<<endl;
	cout<<"Generator: "<<FeldmanVSS::get_g()<<endl;

	Init save;
	save.set_q(FeldmanVSS::get_q());
	save.set_g(FeldmanVSS::get_g());

	save.save(INIT_FILE);

	return 0;
}
