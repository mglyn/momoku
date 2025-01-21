#include "interface/testGUI.h"
//#include "test/selfplay.h"
#include <memory>

/** main function for AI console application  */
int main() {

	std::unique_ptr<Client> c(new Client(15));
	c->Run();

	//SelfPlay::SelfPlay();
}
