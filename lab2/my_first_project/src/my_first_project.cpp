#include <iostream>
#include <sys/process.h>
using namespace std;

int main()
{
	cout << "Hello World from QNX Neutrino RTOS!!!" << endl;

	cout << "@author Neil Kingdom (king0482@algonquinlive.com)" << endl;
	cout << "Something unique about myself: I am an avid coffee fanatic" << endl;
	cout << endl;

	cout << "Current process ID: " << getpid() << endl;
	cout << "Parent process ID: " << getppid() << endl;
	cout << endl;

	cout << "Enter 'q' to quit: ";
	getchar();

	return EXIT_SUCCESS;
}
