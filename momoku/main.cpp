#include "piskvork/pisqpipe.h"
#include "test/testGUI.h"

/** main function for AI console application  */
int main(){
	if (0) {
		Piskvork::pisqpipe();
	}
	else {
		GUI* gui = new GUI;
		gui->Run();
		delete gui;
	}
}
