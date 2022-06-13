#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
//////////////////////////////////////////////////////////////////////////
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "clang/AST/Type.h"
#include "MetaParser/MetaDefines.h"
#include "MetaParser/MetaParser.h"
#include "MetaParser/MetaParserOptions.h"


using namespace clang::tooling;
using namespace llvm;
using namespace clang;


//////////////////////////////////////////////////////////////////////////

static llvm::cl::OptionCategory MyToolCategory("MetaParser options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");
static cl::opt<mtstr> Opt_MTOutFile("o", cl::desc("MetaParser Output "), cl::value_desc("OutputFilePath"), cl::cat(MyToolCategory));
static cl::opt<bool> Opt_MTLogToFile("mt-log-to-file", cl::desc("MetaParser Log To File "), cl::value_desc("ToggleLogToFile"), cl::cat(MyToolCategory));
//////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
	mtVector<mtstr> args;
	mtVector<mtstr> srcfiles;
	mtVector<mtstr> incfiles;
	mtVector<mtstr> defines;
	mtstr outfile;

	args.push_back(argv[0]);

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		mtstr line = arg;
		MTParserArgLine(args, srcfiles, incfiles, defines, outfile, line);
	}


	mtstr mtdsrc = MTGetMTDSrcFilePath(outfile);
	MTCreateMTDSrcFile(mtdsrc, srcfiles, incfiles, defines);
	args.push_back(mtdsrc);


	int newargc = args.size();
	const char** newargv = new const char* [newargc + 1];
	for (int i = 0; i < newargc; ++i)
	{
		newargv[i] = args[i].c_str();
	}
	newargv[newargc] = nullptr;


	CommonOptionsParser OptionsParser(newargc, newargv, MyToolCategory);
	CmdLineOption.OutFileNormal = Opt_MTOutFile;
	CmdLineOption.OutFileUnkown = MTGetUnkownOutFilePath(Opt_MTOutFile);
	CmdLineOption.LogFilePath = MTGetLogFilePath(Opt_MTOutFile);
	CmdLineOption.bLogToFile = Opt_MTLogToFile;
	MTMakeSureDirExist(Opt_MTOutFile);
	
	std::error_code EC;
	MTLogFileStream = new raw_fd_ostream(CmdLineOption.LogFilePath, EC);


	logs_bar("MetaParser Begin");
	logs() << "MetaParser<CmdLineOption.OutFileNormal> = " << CmdLineOption.OutFileNormal << "\n";
	logs() << "MetaParser<CmdLineOption.OutFileUnkown> = " << CmdLineOption.OutFileUnkown << "\n";
	logs() << "MetaParser<mtdsrc> = " << mtdsrc << "\n";
	logs_line('-');
	
	
	ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());


	bool bResult = Tool.run(newFrontendActionFactory<FindNamedClassAction>().get());


	MTSaveFile(CmdLineOption.OutFileNormal, GMTContext.NormalDecl);
	MTSaveFile(CmdLineOption.OutFileUnkown, GMTContext.UnkownDecl);

	logs_bar("MetaParser End");
	return bResult;
}