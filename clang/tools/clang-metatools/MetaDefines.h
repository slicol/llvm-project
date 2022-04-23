#pragma once
#include <string>
#include "llvm/Support/raw_ostream.h"

#define mtMap  std::unordered_map
#define mtOMap  std::map
#define mtVector std::vector
#define mtPair std::pair
#define mtSet std::set
#define mtHash std::hash
#define mtstr std::string
#define mtsize std::size_t

#define mtToString(v) std::to_string(v)

using namespace llvm;

enum class MTLogLevel
{
	Error = 0,
	Warning = 1,
	Log = 2,
	Verbose = 3
};

struct MTCmdLineOption
{
	mtstr WorkDir;
	MTLogLevel LogLevel = MTLogLevel::Verbose;
};

extern MTLogLevel MTEnableLogLevel;
raw_ostream& logs(MTLogLevel Lv = MTLogLevel::Log);
inline raw_ostream& verboses(){return logs(MTLogLevel::Verbose);}
extern MTCmdLineOption CmdLineOption;

void logs_bar(const mtstr& title);
void logs_line(char type = 0);

mtstr& MTStringFormat(mtstr& buff, const char* fmt_str, ...);
bool MTSaveFile(const mtstr& dir, const mtstr& filename, const mtstr& data);
bool MTAppendFile(const mtstr& dir, const mtstr& filename, const mtstr& data);
void MTStringSplit(const mtstr& s, const mtstr& delim, mtVector<mtstr>& output);
bool MTStringStartWith(const mtstr& s, const mtstr& t);
bool MTStringStartWith(const mtstr& s, const char* t);
bool MTStringEndWith(const mtstr& s, const mtstr& t);
bool MTStringEndWith(const mtstr& s, const char* t);
void MTStringReplaceAll(mtstr& str, const mtstr& from, const mtstr& to);
