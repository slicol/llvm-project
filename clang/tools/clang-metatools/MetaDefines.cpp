#include "MetaDefines.h"
#include <stdarg.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include "llvm/Support/FileSystem.h"


using namespace llvm;
using namespace std;
using namespace llvm::sys;


MTLogLevel MTEnableLogLevel = MTLogLevel::Verbose;
MTCmdLineOption CmdLineOption;

llvm::raw_ostream& logs(MTLogLevel Lv /*= MTLogLevel::Log*/)
{
	if (Lv <= MTEnableLogLevel)
	{
		if (Lv >= MTLogLevel::Log)
		{
			return errs();
		}
		else
		{
			return errs();
		}
	}
	return nulls();
}

mtstr& MTStringFormat(mtstr& buff, const char* fmt_str, ...)
{
	mtsize n = 256;

	if (buff.size() < n)
	{
		buff.resize(n);
	}
	else
	{
		n = buff.size();
	}

	while (1)
	{
		va_list ap;
		va_start(ap, fmt_str);
		const int final_n = vsnprintf(&buff[0], n, fmt_str, ap);
		va_end(ap);
		if (final_n < 0)
		{
			n += static_cast<size_t>(-final_n);
			buff = "encoding error";
			break;
		}

		if (static_cast<size_t>(final_n) >= n)
		{
			n += static_cast<size_t>(final_n) - n + 1;
			buff.resize(n);
		}
		else
		{
			buff[final_n] = '\0';
			buff.resize(final_n);
			break;
		}
	}
	return buff;
}


void MTStringSplit(const mtstr& s, const mtstr& delim, mtVector<mtstr>& output)
{
	mtsize pos = 0;
	mtsize len = s.length();
	mtsize delim_len = delim.length();
	if (delim_len == 0) 
	{
		output.push_back(s); // 如果分隔符为空， 将原始串放进去
		return;
	}
	while (pos < len)
	{
		int find_pos = (int)s.find(delim, pos);
		if (find_pos < 0) {
			if (len - pos > 0)
				output.push_back(s.substr(pos, len - pos));
			break;
		}
		if (find_pos - pos > 0)
			output.push_back(s.substr(pos, find_pos - pos));

		pos = find_pos + delim_len;
	}
	if (output.empty())
	{
		output.push_back(s);
	}
	return;
}


bool MTSaveFile(const mtstr& dir, const mtstr& filename, const mtstr& data)
{
	if (!fs::exists(dir))
	{
		errs() << "MTSaveFile() Create Dir: " << dir << "\n";
		fs::create_directories(dir);
	}

	mtstr fullpath = dir + "/" + filename;
	logs() << "MTSaveFile() " << fullpath << "\n";

	ofstream ofs;
	ofs.open(fullpath.c_str(), ios_base::binary | ios_base::out | ios_base::trunc);
	if (!ofs.good())
	{
		logs() << "MTSaveFile() \t file open failed!\n";
		return false;
	}

	ofs.write(data.c_str(), data.size());
	if (ofs.bad())
	{
		logs() << "MTSaveFile() \t file write failed, inner error !\n";
		return false;
	}

	ofs.close();
	return true;
}

bool MTAppendFile(const mtstr& dir, const mtstr& filename, const mtstr& data)
{
	if (!dir.empty())
	{
		if (!fs::exists(dir))
		{
			fs::create_directory(dir);
		}
	}
	

	mtstr fullpath = dir + "/" + filename;
	logs() << "MTAppendFile() " << fullpath << "\n";

	ofstream ofs;
	ofs.open(fullpath.c_str(), ios_base::binary | ios_base::out | ios_base::app);
	if (!ofs.good())
	{
		logs() << "MTAppendFile() \t file open failed!\n";
		return false;
	}

	ofs.write(data.c_str(), data.size());
	if (ofs.bad())
	{
		logs() << "MTAppendFile() \t file write failed, inner error !\n";
		return false;
	}

	ofs.close();
	return true;
}

void logs_bar(const mtstr& title)
{
	logs() << "\n";
	logs() << "\n";
	logs() << "===-------------------------------------------------------------------------===" << "\n";
	logs() << "   " << title << "\n";
	logs() << "===-------------------------------------------------------------------------===" << "\n";
}

void logs_line(char type)
{
	switch (type)
	{
	case '.': logs() << "..............................................................................." << "\n"; break;
	case '-': logs() << "-------------------------------------------------------------------------------" << "\n"; break;
	case '*': logs() << "*******************************************************************************" << "\n"; break;
	case '=': logs() << "===============================================================================" << "\n"; break;
	case '+': logs() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << "\n"; break;
	case '#': logs() << "###############################################################################" << "\n"; break;
	default:logs() << "===-------------------------------------------------------------------------===" << "\n"; break;
	}
}

bool MTStringStartWith(const mtstr& s, const mtstr& t)
{
	return s.substr(0, t.size()).compare(t) == 0;
}

bool MTStringStartWith(const mtstr& s, const char* t)
{
	return s.substr(0, strlen(t)).compare(t) == 0;
}

bool MTStringEndWith(const mtstr& s, const mtstr& t)
{
	int len = (int)t.size();
	return s.substr(s.size() - len, len).compare(t) == 0;
}

bool MTStringEndWith(const mtstr& s, const char* t)
{
	mtsize len = strlen(t);
	return s.substr(s.size() - len, len).compare(t) == 0;
}

void MTStringReplaceAll(mtstr& str, const mtstr& from, const mtstr& to)
{
	if (from.empty())
		return;
	mtsize start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != mtstr::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
