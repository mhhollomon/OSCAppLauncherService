// OALMidi2osc.cpp 
//

#include <iostream>

#include <platform.hpp>
#include <config.hpp>

// into the wild, baby!
#include <thread>
#include <condition_variable>


#include <deque>
#include <map>

#include <mmeapi.h>

#pragma comment(lib,"Winmm.lib")

namespace Pl = Platform;

struct midi {
	int command;
	int data1;
	int data2;

	midi() = default;

	midi(int word) : command(word & 0xff),
		data1((word >> 8) & 0xff), data2((word >> 16) & 0xff) {}
};

std::deque<midi> midi_queue;

// need to hold this to play with the queue
std::mutex midi_queue_mtx;

std::condition_variable midi_queue_cv;

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
			std::cout << "cmd=" << std::hex << (midi_message & 0xff) << " d1=" << ((midi_message >> 8) & 0xff) << " d2=" << ((midi_message >> 16) & 0xff) << "\n";

			std::lock_guard<std::mutex> guard(midi_queue_mtx);
			midi_queue.emplace_back(int(midi_message));
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
				exit(EXIT_FAILURE);
			}
			dstatus = midiInAddBuffer(hMidiIn, header, sizeof(MIDIHDR));
			if (dstatus != MMSYSERR_NOERROR) {
				std::cout << "Error when calling midiInAddBuffer: " << dstatus << "\n";
				exit(1);
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

void midi_reader(int port) {

	std::unique_lock<std::mutex> lck(shutdown_mtx);

	HMIDIIN handle;

	if (midiInOpen(&handle, port, (DWORD_PTR)MidiInProc, port, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		std::cerr << "Failed to open the port\n";
		return;
	}

	midiInStart(handle);

	shutdown_cv.wait(lck, []{ return ready_to_shutdown; });
	lck.unlock();
	shutdown_cv.notify_all();
	midi_queue_cv.notify_all();

	std::cout << "midi reader shutting down\n";
}

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

		midi m = midi_queue.front();
		midi_queue.pop_front();

		std::cout << "proc cmd=" << std::hex << m.command << " d1=" << m.data1 << " d2=" << m.data2 << "\n";
	}


}

std::map<std::string, int> os_midi_ports_in;
std::map<std::string, int> os_midi_ports_out;

struct osc_config {
	enum TRANSPORT { UDP, TCP } transport_ = UDP;
	std::string address_;

	osc_config() = default;
	osc_config(std::string a, enum TRANSPORT t = UDP) : transport_(t), address_(a) {}
};

std::map<std::string, osc_config> osc_ports;

std::string default_osc_port;

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

				// "osc"
				if (setting.exists("osc")) {
					std::string osc_port;
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

				// Parse "map"
				// A list of groups that define a mapping.

				if (setting.exists("map")) {
					auto& map_setting = setting.lookup("map");
					if (map_setting.isList()) {
						for (auto& iter : map_setting) {
							if (iter.isGroup()) {
								//auto event_map = parse_event_map(iter);
							}
							else {
								std::cerr << "entries in map for '" << name << "' must be groups (have brances)\n";
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
		}
	}
	else {
		std::cerr << "'midi' setting is not a grop (must have braces)\n";
		error_found = true;
	}


	if (error_found) throw std::runtime_error("config error");

	return cfg;

}

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

	std::string port_name = cfg->lookup("port");

	int the_port;

	auto iter = os_midi_ports_in.find(port_name);
	if (iter != os_midi_ports_in.end()) {
		the_port = iter->second;
	}
	else {
		std::cerr << "Could not find input midi port\n";
		return(EXIT_FAILURE);
	}


	std::thread reader = std::thread(midi_reader, the_port);
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
