#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"

using namespace std;

class ProcessParser{
private:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:

string ProcessParser::getVmSize(string pid) {
	string line;
	string name = "VmData";
	string value;
	float result;

	ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
	while(std::getline(stream, line)) {
		if (line.compare(0, name.size(), name) == 0){
			istringstream buf(line);
			istream_iterator<string> beg(buf), end;
			vector<string> values(beg, end);
			result = (stof(values[1])/float(1024*1024));
			break;
		}
	}
	return to_string(result);
}

string ProcessParser::getCpuPercent(string pid) {
	string line;
	string value;
	float result;
	ifstream stream = Util::getStream((Path::basePath()+ pid + "/" + Path::statPath()));
	
	while(getline(stream, line)){
		istringstream buf(line);
		istream_iterator<string> beg(buf), end;
		vector<string> values(beg, end);

		float utime = stof(ProcessParser::getProcUpTime(pid));
		float stime = stof(values[14]);
		float cutime = stof(values[15]);
		float cstime = stof(values[16]);
		float starttime = stof(values[21]);
		float uptime = ProcessParser::getSysUpTime();
		float freq = sysconf(_SC_CLK_TCK);
		float total_time = utime + stime + cutime + cstime;
		float seconds = uptime - (starttime/freq);
		result = 100.0*((total_time/freq)/seconds);
		stream.close();
		return to_string(result*100.0);
	}
	stream.close();
	return to_string(-1);
}

string ProcessParser::getProcUpTime(string pid) {
	string line;
	string value;
	float result;

    ifstream stream = Util::getStream((Path::basePath() + pid + "/" +  Path::statPath()));
    getline(stream, line);
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); // done!
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

long int ProcessParser::getSysUpTime() {
    string line;
    ifstream stream = Util::getStream((Path::basePath() + Path::upTimePath()));
    getline(stream,line);
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    return stoi(values[0]);
}

string ProcessParser::getProcUser(string pid) {
	string line;
    string name = "Uid:";
    string result ="";
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
			istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result =  values[1];
            break;
		}
	}
	stream = Util::getStream("/etc/passwd");
    name =("x:" + result);
    while (std::getline(stream, line)) {
        if (line.find(name) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
		}
	}
	return "";
}

vector<string> ProcessParser::getPidList() {
    DIR* dir;
    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));
	while (dirent* dirp = readdir(dir)) {
        if(dirp->d_type != DT_DIR)
            continue;
        if (all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c){ return std::isdigit(c); })) {
            container.push_back(dirp->d_name);
		}
	}
	if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
	return container;
}

bool ProcessParser::isPidExisting(string pid) {
	vector<string> pidList = getPidList();
	return std::find(pidList.begin(), pidList.end(), pid) != pidList.end();
}

string ProcessParser::getCmd(string pid) {
    string line;
    ifstream stream = Util::getStream((Path::basePath() + pid + Path::cmdPath()));
    std::getline(stream, line);
    return line;
}

int ProcessParser::getNumberOfCores() {
    string line;
    string name = "cpu cores";
    ifstream stream = Util::getStream((Path::basePath() + "cpuinfo"));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
		}
	}
	return 0;
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber) {
    string line;
    string name = "cpu" + coreNumber;
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values;
		}
	}
    return (vector<string>());
}

float getSysActiveCpuTime(vector<string> values)
{
    return (stof(values[S_USER]) + stof(values[S_NICE]) + stof(values[S_SYSTEM]) + stof(values[S_IRQ]) + stof(values[S_SOFTIRQ]) + stof(values[S_STEAL]) + stof(values[S_GUEST]) + stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string>values)
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2)
{
	float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0*(activeTime / totalTime);
    return to_string(result);
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree:";
    string name3 = "Buffers:";
    string value;
    int result;
    ifstream stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (std::getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0)
			break;
        if (line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
		}
        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
		}
        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
		}
	}
	return float(100.0*(1-(free_mem/(total_mem-buffers))));
}

string ProcessParser::getSysKernelVersion()
{
    string line;
    string name = "Linux version ";
    ifstream stream = Util::getStream((Path::basePath() + Path::versionPath()));
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
		}
	}
	return "";
}

string ProcessParser::getOSName()
{
    string line;
    string name = "PRETTY_NAME=";
    ifstream stream = Util::getStream(("/etc/os-release"));

    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              std::size_t found = line.find("=");
              found++;
              string result = line.substr(found);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
		}
	}
	return "";
}

int ProcessParser::getTotalThreads()
{
    string line;
    int result = 0;
    string name = "Threads:";
    vector<string>_list = ProcessParser::getPidList();
    for (int i=0 ; i<_list.size();i++) {
    	string pid = _list[i];
    	ifstream stream = Util::getStream((Path::basePath() + pid + Path::statusPath()));
	    while (std::getline(stream, line)) {
	        if (line.compare(0, name.size(), name) == 0) {
				istringstream buf(line);
            	istream_iterator<string> beg(buf), end;
            	vector<string> values(beg, end);
            	result += stoi(values[1]);
				break;
			}
		}
	}
	return result;
}

int ProcessParser::getTotalNumberOfProcesses()
{
    string line;
    int result = 0;
    string name = "processes";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
	while (std::getline(stream, line)) {
		if (line.compare(0, name.size(), name) == 0) {
			istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
			break;
		}
	}
    return result;
}

int ProcessParser::getNumberOfRunningProcesses()
{
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream stream = Util::getStream((Path::basePath() + Path::statPath()));
	while (std::getline(stream, line)) {
		if (line.compare(0, name.size(), name) == 0) {
			istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
			break;
		}
	}
	return result;
}


