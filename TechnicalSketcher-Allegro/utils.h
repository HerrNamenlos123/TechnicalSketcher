
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <string>
#include <map>
#include <algorithm>
#include <chrono>
#include <iomanip>

#include "geometry.h"

typedef uint64_t timestamp_t;

timestamp_t millis();
timestamp_t micros();

int mapInt(int x, int in_min, int in_max, int out_min, int out_max);
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
double mapDouble(double x, double in_min, double in_max, double out_min, double out_max);

double random(double min, double max);

//std::wstring stringToWString(std::string str);

std::string loadStringFromFile(const std::string& path);

std::vector<std::string> loadVectorFromString(const std::string& src);

std::vector<std::string> loadVectorFromFile(const std::string& path);

void saveStringToFile(const std::string& path, const std::string& file);


/*
 * name: fileExists
 * desc: checks if a specified file exists and returns true or false
 * @param: std::string relative or absolute path to file
 * @return: bool file existence
*/
bool fileExists(const std::string& str);


/*
 * name: splitString
 * desc: split a string into pieces at every occurence of a specific character
 *       e.g: splitString("Hello world!", ' ')  ->  [0]: "Hello"
 * 													[1]: "world!"
 * @param: std::string to split
 * @param: char delimeter to split at
 * @return: std::vector<std::string> vector containing all split pieces
*/
std::vector<std::string> splitString(std::string str, char delimeter);


/*
 * name: splitStringAtFirstOccurence
 * desc: split a string into two pieces at the first occurence of a specific character
 *       e.g: splitStringAtFirstOccurence("this is a test string", ' ')  ->  [0]: "this"
 * 															                 [1]: "is a test string"
 *       in case the first delimeter is at the beginning or the end, only one string is returned
 * @param: std::string to split
 * @param: char delimeter to split at
 * @return: std::vector<std::string> vector containing up to two of the split pieces
*/
std::vector<std::string> splitStringAtFirstOccurence(std::string str, char delimeter);


/*
 * name: splitStringAtLastOccurence
 * desc: split a string into two pieces at the last occurence of a specific character
 *       e.g: splitStringAtLastOccurence("this/is/a/path/example.txt", '/')  ->  [0]: "this/is/a/path"
 * 															                  	 [1]: "example.txt"
 *       in case the last delimeter is at the beginning or the end, only one string is returned
 * @param: std::string to split
 * @param: char delimeter to split at
 * @return: std::vector<std::string> vector containing up to two of the split pieces
*/
std::vector<std::string> splitStringAtLastOccurence(std::string str, char delimeter);


/*
 * name: getFileExtension
 * desc: get the file type of a specified file name
 *       e.g: getFileExtension("this/is/a/path/example.txt")  ->  "txt"

 * @param: std::string file name
 * @return: std::string file type
*/
std::string getFileExtension(const std::string& str);

template <typename T>
std::string toString(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string toString(float value, size_t digits);

bool isDigit(char c);
bool isPrintable(char c);
bool isNumber(std::string str);
bool isPrintable(std::string str);

void printVector(std::vector<std::string> v);

#endif // UTILS_H
