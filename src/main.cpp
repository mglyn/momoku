#include "interface/GUI/client.h"
#include "interface/pisqpipe/pisqpipe.h"
#include "interface/GUI/selfplay.h"

#include <memory>

/** main function for AI console application  */
int main() {
	//Piskvork::pisqpipe();
	//std::unique_ptr<Client> c(new Client);c->Loop();
	miniGUI m; m.selfPlayParallel();
}
