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
#include <stdarg.h>

using namespace clang::tooling;
using namespace llvm;

//////////////////////////////////////////////////////////////////////////
using namespace clang;

#define mtstr std::string
#define mtsize std::size_t

bool MTStringStartWith(const mtstr& s, const mtstr& t)
{
	return s.substr(0, t.size()).compare(t) == 0;
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

static std::vector<mtstr>* MTBuiltinTypeTable;
static void MTCheckBuiltinTypeTable()
{
	if (!MTBuiltinTypeTable)
	{
		MTBuiltinTypeTable = new std::vector<mtstr>((mtsize)BuiltinType::LastKind + 1);

		(*MTBuiltinTypeTable)[BuiltinType::Void] = "void";
		(*MTBuiltinTypeTable)[BuiltinType::NullPtr] = "void*";
		(*MTBuiltinTypeTable)[BuiltinType::Bool] = "bool";
		(*MTBuiltinTypeTable)[BuiltinType::Char_U] = "i8";
		(*MTBuiltinTypeTable)[BuiltinType::Char_S] = "i8";
		(*MTBuiltinTypeTable)[BuiltinType::UChar] = "i8";
		(*MTBuiltinTypeTable)[BuiltinType::SChar] = "i8";
		(*MTBuiltinTypeTable)[BuiltinType::Char8] = "i8";
		(*MTBuiltinTypeTable)[BuiltinType::Short] = "i16";
		(*MTBuiltinTypeTable)[BuiltinType::UShort] = "i16";
		(*MTBuiltinTypeTable)[BuiltinType::Char16] = "i16";
		(*MTBuiltinTypeTable)[BuiltinType::Char32] = "i32";
		(*MTBuiltinTypeTable)[BuiltinType::Int] = "i32";
		(*MTBuiltinTypeTable)[BuiltinType::UInt] = "i32";
		(*MTBuiltinTypeTable)[BuiltinType::Long] = "i32";
		(*MTBuiltinTypeTable)[BuiltinType::ULong] = "i32";
		(*MTBuiltinTypeTable)[BuiltinType::LongLong] = "i64";
		(*MTBuiltinTypeTable)[BuiltinType::ULongLong] = "i64";
		(*MTBuiltinTypeTable)[BuiltinType::Float] = "float";
		(*MTBuiltinTypeTable)[BuiltinType::Double] = "double";
	}
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


void MTNormalizeQualifiedName(mtstr& name)
{
	MTStringReplaceAll(name, "::", ".");
}


class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
	explicit FindNamedClassVisitor(ASTContext* Context) : Context(Context) {}

	static bool MTCheckFilter(CXXRecordDecl* Decl)
	{
		bool bResult = true;
		StringRef name = Decl->getName();
		bResult = bResult && !name.empty();
		bResult = bResult && !name.startswith("__vcrt");
		bResult = bResult && !name.startswith("__crt");
		bResult = bResult && !name.startswith("_Crt");
		bResult = bResult && !name.startswith("_");
		
		std::string qname = Decl->getQualifiedNameAsString();
		bResult = bResult && !MTStringStartWith(qname,"std::");
		
		bResult = bResult && !Decl->field_empty();

		return bResult;
	}


	mtstr MTGetTypeName(const clang::Type* Ty)
	{
		MTCheckBuiltinTypeTable();

		const clang::Type* DesugaredType = Ty->getUnqualifiedDesugaredType();

		if (DesugaredType->isBuiltinType())
		{
			const BuiltinType* BT = DesugaredType->getAs<BuiltinType>();

			int k = (int)BT->getKind();
			mtstr name = (*MTBuiltinTypeTable)[k];
			if (!name.empty())
			{
				return name;
			}
			else
			{
				return mtstr("(unkown)") + BT->getNameAsCString(Context->getPrintingPolicy());
			}
		}
		else if (DesugaredType->isUnionType())
		{
			const RecordType* RT = DesugaredType->getAs<RecordType>();
			const RecordDecl* RD = RT->getDecl();
			mtstr name = RD->getQualifiedNameAsString();
			MTNormalizeQualifiedName(name);
			return mtstr("%") + name;
		}
		else if (DesugaredType->isStructureOrClassType())
		{
			const RecordType* RT = DesugaredType->getAs<RecordType>();
			const RecordDecl* RD = RT->getDecl();
			const ClassTemplateSpecializationDecl* CTSD = dyn_cast<ClassTemplateSpecializationDecl>(RD);
			if (CTSD)
			{
				mtstr TmplArgList;
				ArrayRef<TemplateArgument>  ArgList = CTSD->getTemplateInstantiationArgs().asArray();
				for (int i = 0; i < ArgList.size(); ++i)
				{
					const TemplateArgument& Arg = ArgList[i];
					QualType TAT = Arg.getAsType();
					if (i == 0)
					{
						TmplArgList = MTGetTypeName(TAT.getTypePtr());
					}
					else
					{
						TmplArgList = TmplArgList + "," + MTGetTypeName(TAT.getTypePtr());
					}
				}

				mtstr name = RD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);
				return mtstr("%") + name + "<" + TmplArgList + ">";
			}
			else
			{
				mtstr name = RD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);
				return mtstr("%") + name;
			}
		}
		else if (DesugaredType->isEnumeralType())
		{
			const EnumType* ET = DesugaredType->getAs<EnumType>();
			const EnumDecl* ED = ET->getDecl();
			mtstr name = ED->getQualifiedNameAsString();
			MTNormalizeQualifiedName(name);
			return mtstr("%") + name;
		}
		else if (DesugaredType->isConstantArrayType())
		{
			const ConstantArrayType* CAT = dyn_cast<ConstantArrayType>(DesugaredType->getAsArrayTypeUnsafe());
			
			mtstr elt = MTGetTypeName(CAT->getElementType().getTypePtr());
			mtstr n =  CAT->getSize().toString(10, false);
			return mtstr("[") + n + " x " + elt + "]";
		}
		else if (DesugaredType->isVoidType())
		{
			return "void";
		}
		else  if (DesugaredType->isVoidPointerType())
		{
			return "void*";
		}
		else if(!DesugaredType->getPointeeType().isNull())
		{
			mtstr PtrTokens = "*";

			QualType PTE = DesugaredType->getPointeeType();
			while (!PTE->getPointeeType().isNull())
			{
				PtrTokens.append("*");
				PTE = PTE->getPointeeType();
			}

			mtstr Pte = MTGetTypeName(PTE.getTypePtr());
			mtstr name = Pte + PtrTokens;
			return name;
		}
		else
		{
			
			const TagDecl* TD = DesugaredType->getAsTagDecl();
			if (TD)
			{
				mtstr name = TD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);
				return mtstr("(unkown)") + name;
			}
		}
		
		return "(unkown)";
	}


	bool VisitCXXRecordDecl(CXXRecordDecl* Declaration)
	{
		if (!MTCheckFilter(Declaration))
		{
			return true;
		}

		mtstr structName = Declaration->getQualifiedNameAsString();
		MTNormalizeQualifiedName(structName);
		llvm::outs() << structName;
		if (Declaration->getDescribedTemplate())
		{
			llvm::outs() << "<getDescribedTemplate>";
		}
		if (Declaration->getDescribedClassTemplate())
		{
			llvm::outs() << "<getDescribedClassTemplate>";
		}
		if (Declaration->getDescribedTemplateParams())
		{
			llvm::outs() << "<getDescribedTemplateParams>";
		}
		if (Declaration->getTemplateInstantiationPattern())
		{
			llvm::outs() << "<getTemplateInstantiationPattern>";
		}
		if (Declaration->isTemplated())
		{
			llvm::outs() << "<isTemplated>";
		}
		if (Declaration->isTemplateParameter())
		{
			llvm::outs() << "<isTemplateParameter>";
		}
		llvm::outs() << " = \n{\n";

		for(const FieldDecl* field: Declaration->fields())
		{
			if (field->isTemplated())
			{
				llvm::outs() << "<isTemplated>";
			}
			if (field->isTemplateDecl())
			{
				llvm::outs() << "<isTemplateDecl>";
			}

			QualType fieldType = field->getType();
			const clang::Type* DesugaredType = fieldType->getUnqualifiedDesugaredType();
			
			llvm::outs() << "    " << *field << ":";

			mtstr fieldTypeName = MTGetTypeName(DesugaredType);

			llvm::outs() << fieldTypeName;

			if (DesugaredType->isStructureOrClassType())
			{
				const RecordType* RT = DesugaredType->getAs<RecordType>();
				const RecordDecl* RD = RT->getDecl();
				
			
				if (RD->isTemplateDecl())
				{
					llvm::outs() << "<isTemplateDecl>";
				}
			}

			//fieldType->getUnqualifiedDesugaredType()->dump(llvm::outs(), *Context);
			
			llvm::outs() << ",\n";
		}
		llvm::outs() << "};\n";
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