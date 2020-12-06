
#include "pch.h"

#include "utils.h"

timestamp_t millis() {
    return micros() / 1000;
}

timestamp_t micros() {
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    //struct timeval now;
    //gettimeofday(&now, NULL);
    //return now.tv_sec * 1000000 + now.tv_usec;
}

int mapInt(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double mapDouble(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

double random(double min, double max) {
    return mapDouble(static_cast<double>(rand()), 0.0, static_cast<double>(RAND_MAX), min, max);
}

/*std::wstring stringToWString(std::string str) {
    wchar_t* wcs = new wchar_t[str.length() + 1];
    std::mbstowcs_s(wcs, str.c_str(), str.length() + 1);

    std::wstring wideString(wcs);

    delete[] wcs;

    return wideString;
}*/

std::string loadStringFromFile(const std::string& path) {
	std::ifstream file;
	file.open(path);

	if(file.fail())
        throw std::runtime_error("Failed to open file '" + path + "' for reading!");

	std::stringstream st;
	std::string out;
	st << file.rdbuf();
	out = st.str();

	file.close();

	return out;
}

std::vector<std::string> loadVectorFromString(const std::string& src) {
    std::vector<std::string> vector;
    std::stringstream ss;

    ss << src;

    std::string line;
    while (std::getline(ss, line)) {
        vector.push_back(line);
    }

    return vector;
}

std::vector<std::string> loadVectorFromFile(const std::string& path) {

    std::string stringFile = loadStringFromFile(path);  // Can throw std::runtime_error

    return loadVectorFromString(stringFile);
}

void saveStringToFile(const std::string& path, const std::string& file) {

    //if(!fileExists(path))
    //    throw std::runtime_error("Failed to open file '" + path + "' for writing!");

    std::ofstream outFile;
    outFile.open(path);

    if(outFile.fail())
        throw std::runtime_error("Failed to open file '" + path + "' for writing!");

    outFile << file;

    outFile.close();
}

bool fileExists(const std::string& str) {
    std::ifstream file(str);

    if(!file.is_open())
        return false;

    file.close();
    return true;
}

std::vector<std::string> splitString(std::string str, char delimeter) {
    std::string::size_type b = 0;
    std::vector<std::string> result;

    while ((b = str.find_first_not_of(delimeter, b)) != std::string::npos) {
        auto e = str.find_first_of(delimeter, b);
        result.push_back(str.substr(b, e-b));
        b = e;
    }
    return result;
}

std::vector<std::string> splitStringAtFirstOccurence(std::string str, char delimeter) {
    std::string first;
    std::string last;
    std::vector<std::string> result;
    bool foundDelimeter = false;

    for(size_t i = 0; i < str.length(); i++) {

        if(!foundDelimeter) {

            if(str[i] == delimeter)
                foundDelimeter = true;
            else
                first += str[i];
        } else {
            last += str[i];
        }
    }

    if(foundDelimeter) { 

        if(first.length() > 0)
            result.push_back(first);

        if(last.length() > 0)
            result.push_back(last);

    } else {
        result.push_back(str);
    }

    return result;
}

std::vector<std::string> splitStringAtLastOccurence(std::string str, char delimeter) {
    std::string first;
    std::string last;
    std::vector<std::string> result;
    bool foundDelimeter = false;

    for(int i = (int)str.length() - 1; i >= 0; i--) {

        if(!foundDelimeter) {

            if(str[i] == delimeter)
                foundDelimeter = true;
            else
                last = str[i] + last;
        } else {
            first = str[i] + first;
        }
    }

    if(foundDelimeter) { 

        if(first.length() > 0)
            result.push_back(first);

        if(last.length() > 0)
            result.push_back(last);
    } else {
        result.push_back(str);
    }

    return result;
}

std::string getFileExtension(const std::string& str) {
    std::string filetype;

    for(int i = (int)str.length() - 1; i >= 0; i--) {
        if(str[i] != '.') {
            if(str[i] == '/')
                return "";

            filetype = str[i] + filetype;

        } else {
            break;
        }
    }

    return filetype;
}

std::string toString(float value, size_t digits) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(digits) << value;
    return ss.str();
}

bool isDigit(char c) {
    if(c >= '0' && c <= '9')
        return true;

    return false;
}

bool isPrintable(char c) {
    if((c >= 32 && c < 127) || c == 10 || c == 13)
        return true;

    return false;
}

bool isNumber(std::string str) {
    bool is = true;

    if(!isDigit(str.at(0)) && str.at(0) != '-')
        is = false;

    for(unsigned int i = 1; i < str.length(); i++) {
        if(!isDigit(str.at(i)))
            is = false;
    }
    return is;
}

bool isPrintable(std::string str) {
    bool is = true;
    for(unsigned int i = 0; i < str.length(); i++) {
        if(!isPrintable(str.at(i)))
            is = false;
    }
    return is;
}

void printVector(std::vector<std::string> v) {
    for(size_t i = 0; i < v.size(); i++) {
        std::cout << "[" << i << "]: " << v.at(i) << std::endl;
    }
}
