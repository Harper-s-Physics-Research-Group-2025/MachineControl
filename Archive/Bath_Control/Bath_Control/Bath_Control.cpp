#include "RTE7.h"
#include <iostream>
using namespace std;


enum Command {
    TURN_ON,
    TURN_OFF,
    SET_TEMP,
    GET_TEMP
};

Command static parse_command(const string& cmd) {
    if (cmd == "turn_on") return TURN_ON;
    else if (cmd == "turn_off") return TURN_OFF;
    else if (cmd == "set_temp") return SET_TEMP;
    else if (cmd == "get_temp") return GET_TEMP;
    else {
        cerr << "Invalid command: " << cmd << endl;
        exit(1); // Exit with error
    }
}

int main(int argc, char** argv) {
    float data;
    if(argc < 3) {
        cerr << "Usage: " << " <COMMAND> <COM_PORT> || <COMMAND> <COM_PORT> <VALUE> " << endl;
        return 1; // Error: not enough arguments
	}

    //RTE7 bath = RTE7("COM14");
    RTE7 bath = RTE7(argv[2]);
    //Command command = TURN_ON; // Default command
    Command command = parse_command(argv[1]);
	

    switch (command) {
    case TURN_ON:
		cout << "Turning on the bath..." << endl;
        bath.turn_on();
        break;
    case TURN_OFF:
        cout << "Turning off the bath..." << endl;
        bath.turn_off();
        break;
    case SET_TEMP:
        cout << "Setting the bath's temperature..." << endl;
        if (argc < 4) {
            cerr << "Error: Temperature value required for SET_TEMP command." << endl;
            return 1; // Error: not enough arguments
        }
        data = stof(argv[3]);
        bath.set_setpoint(data);
        break;
    case GET_TEMP:
        if (!bath.get_temp(data)) {
            cerr << "Unable to get the bath's temperature" << endl; //Error: maybe bath is turned off
            return 1;
        }
        else {
            cout << "The Bath's temperature is: " << data << "C \n";
            break;
        }
    };

	cout << "Operation completed successfully." << endl;
	return 0; // Success

    

   

}