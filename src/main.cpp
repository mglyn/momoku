#include "interface/client.h"
#include "interface/pisqpipe.h"
#include <memory>

/** main function for AI console application  */
int main() {

	Piskvork::pisqpipe();

	//std::unique_ptr<Client> c(new Client);c->Loop();

	/*std::unique_ptr<ProtocalEngine> e(new ProtocalEngine);
	e->loop();*/
}
