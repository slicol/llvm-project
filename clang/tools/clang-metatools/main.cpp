#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
//////////////////////////////////////////////////////////////////////////
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"
#include "clang/AST/Type.h"

using namespace clang::tooling;
using namespace llvm;

//////////////////////////////////////////////////////////////////////////
using namespace clang;



class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
	explicit FindNamedClassVisitor(ASTContext* Context) : Context(Context) {}

	bool VisitCXXRecordDecl(CXXRecordDecl* Declaration)
	{
		llvm::outs() << *Declaration << "\n";
		llvm::outs() << Declaration->getName() << "\n";
		llvm::outs() << Declaration->getQualifiedNameAsString() << "\n";

		std::string ScalarTypeNames[10];
		ScalarTypeNames[clang::Type::STK_CPointer] = "STK_CPointer";
		ScalarTypeNames[clang::Type::STK_BlockPointer] = "STK_BlockPointer";
		ScalarTypeNames[clang::Type::STK_ObjCObjectPointer] = "STK_ObjCObjectPointer";
		ScalarTypeNames[clang::Type::STK_MemberPointer] = "STK_MemberPointer";
		ScalarTypeNames[clang::Type::STK_Bool] = "STK_Bool";
		ScalarTypeNames[clang::Type::STK_Integral] = "STK_Integral";
		ScalarTypeNames[clang::Type::STK_Floating] = "STK_Floating";
		ScalarTypeNames[clang::Type::STK_IntegralComplex] = "STK_IntegralComplex";
		ScalarTypeNames[clang::Type::STK_FloatingComplex] = "STK_FloatingComplex";
		ScalarTypeNames[clang::Type::STK_FixedPoint] = "STK_FixedPoint";

		for(const FieldDecl* field: Declaration->fields())
		{
			QualType fieldType = field->getType();
			
			
			fieldType->dump();

			llvm::outs() << "\t" << *field << ":" << fieldType->getUnqualifiedDesugaredType()->getTypeClassName() << ":";
			if (fieldType->isScalarType())
			{	
				fieldType->getTypeClass();
				llvm::outs() << ScalarTypeNames[(int)fieldType->getScalarTypeKind()];
			}
			else if (fieldType->isStructureOrClassType() || fieldType->isUnionType())
			{
				const CXXRecordDecl* field_decl = fieldType->getAsCXXRecordDecl();
				llvm::outs() << field_decl->getQualifiedNameAsString();
			}
			
			llvm::outs() << "\n";
		}

		llvm::outs() << "--------------------------------------\n";

		if (Declaration->getQualifiedNameAsString() == "n::m::C")
		{
			FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
			if (FullLocation.isValid())
				llvm::outs() << "Found declaration at "
				<< FullLocation.getSpellingLineNumber() << ":"
				<< FullLocation.getSpellingColumnNumber() << "\n";
		}
		return true;
	}

private:
	ASTContext* Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer
{
public:
	explicit FindNamedClassConsumer(ASTContext* Context) : Visitor(Context) {}

	virtual void HandleTranslationUnit(clang::ASTContext& Context)
	{
		Visitor.TraverseDecl(Context.getTranslationUnitDecl());
	}
private:
	FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
public:
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile)
	{
		return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext());
	}
};

//////////////////////////////////////////////////////////////////////////

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");


//////////////////////////////////////////////////////////////////////////


int main(int argc, const char** argv)
{
	CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
	ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

	return Tool.run(newFrontendActionFactory<FindNamedClassAction>().get());

	if (false)
	{
		if (argc > 1)
		{
			clang::tooling::runToolOnCode(std::make_unique<FindNamedClassAction>(), argv[1]);
		}
	}
}