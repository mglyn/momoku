#include "interface/client.h"
#include "gomocup.h"
#include <memory>

/** main function for AI console application  */
int main() {

	std::unique_ptr<Client> c(new Client);
	c->Loop();

	//std::unique_ptr<ProtocalEngine> e(new ProtocalEngine);
	//e->loop();
}
