// OALMidi2osc.cpp 
//

#include <iostream>
#include <sstream>

#include <platform.hpp>
#include <config.hpp>

// into the wild, baby!
#include <thread>
#include <condition_variable>


#include <deque>
#include <map>
#include <string>
#include <array>
#include <charconv>
#include <string_view>
#include <iomanip>

#include <mmeapi.h>

#pragma comment(lib,"Winmm.lib")

namespace Pl = Platform;
using namespace std::string_literals;


std::map<std::string, byte> midi_commands = { {"noteon", 0x90}, {"noteoff", 0x80}, {"cc", 0xb0}, {"prog", 0xc0} };
std::map<byte, std::string> midi_to_string = { {0x90, "noteon"}, {0x80, "noteoff"}, {0xb0, "cc"}, {0xc0, "prog"} };

std::array<std::string, 12> midi_note_string = { "C", "C#", "D", "D#",
	"E", "F", "F#", std::string("G"), std::string("G#"), std::string("A"),
	std::string("A#"), std::string("B")};

std::map<std::string, int> midi_name_to_offset = {
	{ "C", 0 },	
	{ "C#", 1 }, {"Db", 1},	
	{ "D", 2 },
	{ "D#", 3 }, { "Eb" , 3 },
	{ "E", 4},
	{ "F", 5},
	{ "F#", 6}, { "Gb", 6},
	{ "G", 7},
	{ "G#", 8}, {"Ab", 8 },
	{ "A", 9 },
	{ "A#", 10 }, {"Bb", 10 },
	{ "B", 11 },

};

std::string note_num_to_name(byte note_num) {

	auto divres = div(int(note_num), 12);

	int octave = divres.quot - 2;
	auto pitch = midi_note_string[divres.rem];

	std::ostringstream ss;

	ss << pitch << octave;

	return ss.str();
}

// midi notes are 0 -127 - return 255 of error.
constexpr byte NOTE_ERROR = 255;

byte note_name_to_num(std::string_view name) {
	size_t pos = 0;
	std::string pitch_name;
	byte retval = NOTE_ERROR;

	if (name[1] == 'b' || name[1] == '#') {
		pitch_name = std::string(name.substr(0, 2));
		pos = 2;
	}
	else {
		pitch_name = std::string(name.substr(0, 1));
		pos = 1;
	}
	auto& iter = midi_name_to_offset.find(pitch_name);
	if (iter == midi_name_to_offset.end()) {
		std::cerr << "Invalid pitch name " << std::quoted(pitch_name) << "\n";
		return NOTE_ERROR;
	} 
	else
		retval = iter->second;

	std::string octave_name = std::string(name.substr(pos));
	int octave_number = -100;
	auto res = std::from_chars(octave_name.c_str(), octave_name.c_str() + octave_name.size(), octave_number);
	if (octave_number > 8 || octave_number < -2) {
		std::cerr << "Octave number in note name " << std::quoted(name) << " is invalid (-2 to 8)\n";
		return NOTE_ERROR;
	}

	retval += (octave_number + 2) * 12;

	return retval;
}

struct midi_event {
	byte command;
	byte channel;
	byte data1;
	byte data2;

	midi_event() = default;

	midi_event(int word) : command(word & 0xf0), channel(word & 0x0f),
		data1((word >> 8) & 0xff), data2((word >> 16) & 0xff) {
		channel += 1;
	}

	midi_event(const midi_event& x) = default;

	midi_event & operator=(const midi_event& o) = default;

	friend std::ostream& operator << (std::ostream& strm, const midi_event& me);
};

std::ostream& operator << (std::ostream& strm, const midi_event& me) {

	auto& command_name = midi_to_string[me.command];
	strm << "cmd=" << command_name  << "(" << std::hex << int(me.command) <<
		") chan=" << std::dec << int(me.channel+1);

	if (command_name == "noteon" || command_name == "noteoff") {
		strm << " note=" << note_num_to_name(me.data1) << std::dec << "(" << int(me.data1) <<
			") vel=" << std::hex << int(me.data2) << "\n";
	}
	else {
		strm << std::hex <<
			" d1=" << int(me.data1) << " d2=" << int(me.data2) << "\n";
	}

	return strm;

}
struct action {
	enum action_type { OSC, MIDI } type = OSC;
	std::string osc;
	std::string path;
	std::string arg;
};


struct map_rule {
	midi_event event{};
	bool use_d1 = false;
	bool use_d2 = false;
	bool valid = false;

	action act;

	friend std::ostream& operator <<(std::ostream& strm, const map_rule & mr);
};

std::ostream& operator <<(std::ostream& strm, const map_rule& mr) {

	strm << mr.event;
	strm << "use_d1=" << std::boolalpha << mr.use_d1 << " use_d2=" << mr.use_d2 << " valid =" << mr.valid<<"\n";

	return strm;
}

struct midi_port {
	std::string name;
	int port_num;
	std::string osc_port;
	std::vector<map_rule> rule_list;
};

struct midi_queue_event {
	midi_event me;
	const midi_port* port;
};

struct osc_config {
	enum TRANSPORT { UDP, TCP } transport_ = UDP;
	std::string host_;
	int port_ = -1;

	osc_config() = default;
	osc_config(std::string a, enum TRANSPORT t = UDP) : transport_(t), port_(-1) {
		int pos = a.find(':');
		if (pos == std::string::npos) {
			throw std::runtime_error("Badly formed osc address - can't find colon : '"s + a + "'");
		}

		host_ = a.substr(0, pos);

		std::from_chars(a.c_str() + pos + 1, a.c_str() + a.size(), port_);
		if (port_ < 0) {
			throw std::runtime_error("Badly formed osc address - bad port number: '"s + a + "'");
		}

		std::cout << "port_ =" << port_ << " host_=" << host_ << "\n";
	}
};

std::map<std::string, osc_config> osc_ports;

std::string default_osc_port;


/*======================================================================
* PARSE_RULE()
* Parse config structure for a mapping rule.
========================================================================*/
map_rule parse_rule(libconfig::Setting& rule, std::string osc) {
	map_rule retval;

	bool error = false;

	std::string cmd_name;
	if (rule.exists("cmd")) {
		cmd_name = rule.lookup("cmd").c_str();
	}
	else {
		std::cerr << "Rule must contain 'cmd' parameter.\n";
		error = true;
	}

	auto& cmd_iter = midi_commands.find(cmd_name);
	if (cmd_iter == midi_commands.end()) {
		std::cerr << "Unknown command in rule '" << cmd_name << "'\n";
	}
	else {
		retval.event.command = cmd_iter->second;
	}

	if  (cmd_name == "noteon" || cmd_name == "noteoff") {
		if (rule.exists("note")) {
			auto& note_setting = rule.lookup("note");
			byte note_num;
			if (note_setting.isString()) {
				std::string note_name = note_setting.c_str();
				note_num = note_name_to_num(note_name);
				if (note_num != NOTE_ERROR) {
					retval.event.data1 = note_num;
					retval.use_d1 = true;
				}
				else {
					error = true;
				}
			}
			else if (note_setting.isNumber()) {
				int note_num = note_setting;
				if (note_num > 127 || note_num < 0) {
					std::cerr << "Note number is out of rage (0-127)\n";
					error = true;
				}
				else {
					retval.event.data1 = byte(note_num);
					retval.use_d1 = true;
				}
			}
		}

		if (rule.exists("vel")) {
			auto& vel_setting = rule.lookup("vel");
			int vel;
			if (vel_setting.isNumber()) {
				vel = vel_setting;
				if (vel > 127 || vel < 0) {
					std::cerr << "Invalid velocity " << vel << " must be a whole number 0-127\n";
					error = true;
				}
				else {
					retval.event.data2 = byte(vel);
					retval.use_d2 = true;
				}
			}
		}
	}

	if (rule.exists("action")) {
		auto const& action_setting = rule.lookup("action");

		action new_act;
		if (action_setting.exists("type")) {
			auto const& type_setting = action_setting.lookup("type");
			if (type_setting.isString()) {
				std::string new_type = type_setting.c_str();
				if (new_type == "osc"s) {
					new_act.type = action::OSC;
					new_act.osc = osc;

					if (action_setting.exists("addr")) {
						auto const& address_setting = action_setting.lookup("addr");
						if (address_setting.isString()) {
							new_act.osc = address_setting.c_str();
						}
						else {
							std::cerr << "address for osc action must be a string\n";
							error = true;
						}
					}
				}
				else {
					std::cerr << "Invalid action type " << new_type << " (only \"osc\" currently supported)\n";
					error = true;
				}
			}
			else {
				std::cerr << "'type' setting for action must be a string\n";
				error = true;
			}
		}
		else {
			std::cerr << "Missing 'type' setting for action\n";
			error = true;
		}
		if (action_setting.exists("path")) {
			auto const& path_setting = action_setting.lookup("path");
			if (path_setting.isString()) {
				new_act.path = path_setting.c_str();
			}
			else {
				std::cerr << "'path' setting for action must be a string\n";
				error = true;
			}
		}
		else {
			std::cerr << "Missing 'path' setting for action\n";
			error = true;
		}
		if (action_setting.exists("arg")) {
			auto const& arg_setting = action_setting.lookup("arg");
			if (arg_setting.isString()) {
				new_act.arg = arg_setting.c_str();
			}
			else {
				std::cerr << "'arg' setting for action must be a string\n";
				error = true;
			}
		}
		retval.act = new_act;
	}
	else {
		std::cerr << "No action exists for the rule\n";
		error = true;
	}


	if (!error) retval.valid = true;

	std::cout << "parsed rule = " << retval << "\n";

	return retval;

}


std::map<std::string, midi_port> midi_ports;

std::deque<midi_queue_event> midi_queue;

// need to hold this to play with the queue
std::mutex midi_queue_mtx;

std::condition_variable midi_queue_cv;

/*======================================================================
* MIDIINPROC()
* Callback for midi reader. Used by midi_Reader() thread.
========================================================================*/

void CALLBACK MidiInProc(
	HMIDIIN   hMidiIn,
	UINT      wMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR midi_message,
	DWORD_PTR dwParam2
) {

	std::cout << "msg = " << wMsg << "\n";
	switch (wMsg) {

	case MIM_DATA:
		{
		midi_queue_event mqe{ midi_event{(int)midi_message}, (const midi_port*)dwInstance };
			std::cout << "Message : " << mqe.me << "\n";

			std::lock_guard<std::mutex> guard(midi_queue_mtx);
			midi_queue.emplace_back(mqe);
			midi_queue_cv.notify_all();
		}

		break;

	case MIM_OPEN:
	case MIM_CLOSE:
	case MIM_ERROR:
		break;

	// These have to do with sysex messages. Just throw them away for now.
	case MIM_LONGERROR:
	case MIM_LONGDATA: 
		{

			MIDIHDR* header = (LPMIDIHDR)midi_message;

			int dstatus = midiInPrepareHeader(hMidiIn, header, sizeof(MIDIHDR));
			if (dstatus != MMSYSERR_NOERROR) {
				std::cerr << "error in prepare: " << dstatus << "\n";
				return;
			}
			dstatus = midiInAddBuffer(hMidiIn, header, sizeof(MIDIHDR));
			if (dstatus != MMSYSERR_NOERROR) {
				std::cout << "Error when calling midiInAddBuffer: " << dstatus << "\n";
				return;
			}
		}
		break;
	}
}

// The thread needs to loaf around after getting the midi set up.
// But don't want a busy wait. So, set up to wait on a condition variable
// That will only be true when it is time for the application to end.

bool ready_to_shutdown = false;
std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;

/*======================================================================
* MIDI_READER()
* REad and queue midi inputs. Entry for thread.
*
* Not sure I really need this because of the call back struture ??
========================================================================*/

void midi_reader() {

	std::unique_lock<std::mutex> lck(shutdown_mtx);

	std::cout << "starting midi_reader()\n";

	HMIDIIN handle;

	for (auto& port : midi_ports) {
		std::cout << "opening " << port.first << "\n";
		if (midiInOpen(&handle, port.second.port_num, (DWORD_PTR)MidiInProc, (DWORD_PTR) & (port.second), CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
			std::cerr << "Failed to open the port " << port.first << "\n";
			return;
		}
		midiInStart(handle);
	}


	shutdown_cv.wait(lck, []{ return ready_to_shutdown; });
	lck.unlock();
	shutdown_cv.notify_all();
	midi_queue_cv.notify_all();

	std::cout << "midi reader shutting down\n";
}

/*======================================================================
* TAKE_ACTION()
========================================================================*/
void take_action(map_rule const& rule, midi_event const& me) {
	if (rule.act.type == action::OSC) {
		std::cout << "osc port =" << rule.act.osc << "\n";
		int port = osc_ports.find(rule.act.osc)->second.port_;
		std::cout << "Talking OSC to port " << std::dec << port << "\n";
		auto socket = Pl::create_socket(port, SocketDirection::WRITE);
		OSCMessage msg{rule.act.path};
		if (! rule.act.arg.empty()) 
			msg.add_arg(rule.act.arg);

		socket->send_osc(msg);

	}
}
/*======================================================================
* MIDI_PROCESSOR()
* Process the midi inputs. ENtry for processing thread.
========================================================================*/
void midi_processor() {

	std::unique_lock<std::mutex> lck(midi_queue_mtx);

	while (1) {

		midi_queue_cv.wait(lck, [] {return (!midi_queue.empty() || ready_to_shutdown); });

		if (ready_to_shutdown) {
			lck.unlock();
			shutdown_cv.notify_all();
			std::cout << "midi processor shutting down\n";
			return;
		}

		midi_queue_event mqe = midi_queue.front();
		midi_queue.pop_front();

		std::cout << "proc (" << mqe.port->name << ") :" << mqe.me << "\n";

		bool found_rule = false;
		for (auto& rule : mqe.port->rule_list) {

			std::cout << "Looking at rule : " << rule << "\n";
			if (rule.event.command == mqe.me.command &&
				(!rule.use_d1 || rule.event.data1 == mqe.me.data1) &&
				(!rule.use_d2 || rule.event.data2 == mqe.me.data2)) {
				std::cout << "Matched rule! :" << rule << "\n";
				found_rule = true;
				take_action(rule, mqe.me);

				break;
			}
		}
		if (!found_rule) {
			std::cout << "No matching rule - ignoring\n";
		}
	}


}


std::map<std::string, int> os_midi_ports_in;
std::map<std::string, int> os_midi_ports_out;



/*======================================================================
* ENUMERATE_MINI_PORTS()
* Find all the midi ports the OS knows about and get port numbers, etc
========================================================================*/
void enumerate_midi_ports() {
	auto port_count = midiInGetNumDevs();


	for (decltype(port_count) i = 0; i < port_count; ++i) {
		MIDIINCAPS mic;

		if (midiInGetDevCaps(i, &mic, sizeof(mic)) == MMSYSERR_NOERROR) {
			std::cout << "midi in port " << i << ": " << mic.szPname << "\n";
			os_midi_ports_in.emplace(mic.szPname, i);
		}
		else {
			std::cout << "had error for in port " << i << "\n";
		}
	}

	port_count = midiOutGetNumDevs();


	for (decltype(port_count) i = 0; i < port_count; ++i) {
		MIDIOUTCAPS mic;

		if (midiOutGetDevCaps(i, &mic, sizeof(mic)) == MMSYSERR_NOERROR) {
			std::cout << "midi out port " << i << ": " << mic.szPname << "\n";
			os_midi_ports_out.emplace(mic.szPname, i);
		}
		else {
			std::cout << "had error for out port " << i << "\n";
		}
	}
}

/*======================================================================
* COMPILE_CONFIG()
* Run through the configuration and check for validity. 
* Set up in-memory structures.
========================================================================*/
config_ptr compile_config(std::string cfg_file) {

	config_ptr cfg;

	//delay exiting after some errors, so we can find others.
	bool error_found = false;

	try {
		cfg = read_config_file(cfg_file, true);
	}
	catch (const libconfig::FileIOException& fioex) {
		std::cerr << "I/O error while reading file: " << fioex.what() << std::endl;
		throw std::runtime_error("io error");
	}
	catch (const libconfig::ParseException& pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError() << std::endl;
		throw std::runtime_error("Parse error");
	}

	// Parse the 'osc' group. 
	// This is map with the key being an alias for the end point.
	// Values are a group with the ipaddr:port and transport (udp/tcp). Note that currently
	// only udp is supported. Transport is optional and defaults to udp.

	auto &osc_group = cfg->lookup("osc");

	if (osc_group.isGroup()) {
		for (auto iter = osc_group.begin(); iter != osc_group.end(); ++iter) {
			std::string name = iter->getName();
			if (iter->isGroup()) {
				std::string address;
				std::string transport = "udp";
				if (!iter->lookupValue("address", address)) {
					std::cerr << "Osc port '" << name << "' does not have an address specified\n";
					error_found = true;
				}
				iter->lookupValue("transport", transport);
				if (transport.compare("udp") != 0) {
					std::cerr << "Invalid value for transport in osc." << name << " Only 'udp' currently supported)\n";
					transport = "udp";
					error_found = true;
				}
				osc_ports.emplace(name, address );

			}
			else if (iter->isString()) {
				std::string address = *iter;
				osc_ports.emplace(name, osc_config{ address });
			}
			else {
				std::cerr << "invalid configuration for osc." << name << " - must be a string (host:port) or a group\n";
				error_found = true;
			}
		}
	} else {
		std::cerr << "'osc' setting must be a group (have braces)\n";
		error_found = true;
	}

	// Parse "default-osc"

	if (cfg->exists("default-osc")) {
		default_osc_port = cfg->lookup("default-osc").c_str();

		if (osc_ports.find(default_osc_port) == osc_ports.end()) {
			std::cerr << "Configured default osc port '" << default_osc_port << "' does not exist in the osc section.\n";
			error_found = true;
		}

	}

	// Parse the midi group
	// This is a map with the keys being an alias for the midi port.
	// Require value keys are "in", "map" - optional are "osc"
	//
	auto& midi_group = cfg->lookup("midi");

	if (midi_group.isGroup()) {
		for (auto& setting : midi_group) {
			midi_port new_port{};
			
			std::string name = setting.getName();
			if (setting.isGroup()) {
				std::string midi_in;
				int midi_port = -1;
				osc_config osc;

				// "in"
				if (setting.lookupValue("in", midi_in)) {
					auto& iter = os_midi_ports_in.find(midi_in);
					if (iter == os_midi_ports_in.end()) {
						std::cerr << "midi input port for '" << name << "' (" << midi_in << ") is invalid\n";
						error_found = true;
					}
					else {
						midi_port = iter->second;
					}
				}
				else {
					std::cerr << "No midi input port specified for '" << name << "'\n";
					error_found = true;
				}

				new_port.name = name;
				new_port.port_num = midi_port;

				// "osc"
				std::string osc_port;
				if (setting.exists("osc")) {
					auto& osc_setting = setting.lookup("osc");
					if (osc_setting.isString()) {
						osc_port = osc_setting.c_str();
						auto& iter = osc_ports.find(osc_port);
						if ( iter != osc_ports.end()) {
							osc = iter->second;
						} else {
							std::cerr << "OSC port (" << osc_port << ") defined for " << name << " is not configured.\n";
							error_found = true;
						}
					}
				}
				else if (!default_osc_port.empty()) {
					osc = osc_ports.find(default_osc_port)->second;
				}
				else {
					std::cerr << "No OSC port defined for '" << name << "' but no default port defined either.\n";
					error_found = true;
				}

				new_port.osc_port = osc_port;

				// Parse "map"
				// A list of groups that define a mapping.

				if (setting.exists("map")) {
					auto& map_setting = setting.lookup("map");
					if (map_setting.isList()) {
						for (auto& iter : map_setting) {
							if (iter.isGroup()) {
								auto rule = parse_rule(iter, new_port.osc_port);

								if (rule.valid) {
									if (!rule.act.osc.empty()) {

										if (osc_ports.find(rule.act.osc) != osc_ports.end()) {
											new_port.rule_list.push_back(rule);
										}
										else {
											std::cerr << "Invalid OSC address " << rule.act.osc << " (not configured)\n";
											error_found = true;
										}
									}
									else {
										std::cerr << "No OSC address was supplied (and no default) for entry in " << name << "\n";
										error_found = true;
									}
								} else {
									// parse_rule() output an error message, we just need to bubble up the indicator
									error_found = true;
								}
							}
							else {
								std::cerr << "entries in map for '" << name << "' must be groups (have braces)\n";
								error_found = true;
							}
						}
					}
					else {
						std::cerr << "midi." << name << ".map must be a list (have parentheses)\n";
						error_found = true;
					}
				}
				else {
					std::cerr << "No mappings exist for " << name << "\n";
					error_found = true;
				}

			}
			else {
				std::cerr << "Invalid configuration for midi." << name << " - it must be a group (have braces)\n";
				error_found = true;
			}

			midi_ports.emplace(new_port.name, new_port);
		}
	}
	else {
		std::cerr << "'midi' setting is not a grop (must have braces)\n";
		error_found = true;
	}


	if (error_found) throw std::runtime_error("config error");

	return cfg;

}

/*======================================================================
* MAIN()
========================================================================*/
int main(int argc, char** argv) {

	std::string cfg_file = Pl::get_cfg_directory() + "OALMidi2osc.cfg";

	if (argc >= 2) {
		cfg_file = std::string(argv[1]);
	}

	std::cout << "using config file = " << cfg_file << "\n";

	enumerate_midi_ports();

	config_ptr cfg;
	try {
		cfg = compile_config(cfg_file);
	}
	catch (const std::runtime_error& ex) {
		std::cerr << "Problem with config file\n";
		return EXIT_FAILURE;
	}

	std::thread reader = std::thread(midi_reader);
	std::thread processor = std::thread(midi_processor);

	char c;
	while (1) {
		std::cin >> c;
		if (c == 'q')
			break;
	}

	ready_to_shutdown = true;
	shutdown_cv.notify_all();

	reader.join();
	processor.join();

	return(EXIT_SUCCESS);
}
