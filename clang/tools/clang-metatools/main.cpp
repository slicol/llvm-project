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
#include "MetaDefines.h"

using namespace clang::tooling;
using namespace llvm;

//////////////////////////////////////////////////////////////////////////
using namespace clang;


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



static void MTNormalizeQualifiedName(mtstr& name)
{
	MTStringReplaceAll(name, "::", ".");
}

static bool MTHasVirtualTable(const clang::Type* Ty)
{
	if (!Ty) return false;
	CXXRecordDecl* Decl = Ty->getAsCXXRecordDecl();
	if (!Decl) return false;

	for (const CXXMethodDecl* method : Decl->methods())
	{
		if (method->isVirtual()) return true;
	}

	for (CXXBaseSpecifier& base : Decl->bases())
	{
		if (MTHasVirtualTable(base.getType().getTypePtr())) return true;
	}

	return false;
}

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
	bResult = bResult && !MTStringStartWith(qname, "std::");

	if (bResult)
	{
		if (Decl->field_empty())
		{
			if (!MTHasVirtualTable(Decl->getTypeForDecl()))
			{
				bResult = false;
			}
		}
	}

	return bResult;
}

class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
	explicit FindNamedClassVisitor(ASTContext* Context) : Context(Context) {}

	mtstr MTGetTypeName(const clang::Type* Ty, bool& bHasUnkownType)
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
				bHasUnkownType = true;
				return mtstr("(unkown)") + BT->getNameAsCString(Context->getPrintingPolicy());
			}
		}
		else if (DesugaredType->isUnionType())
		{
			const RecordType* RT = DesugaredType->getAs<RecordType>();
			const RecordDecl* RD = RT->getDecl();
			mtstr name = RD->getQualifiedNameAsString();
			MTNormalizeQualifiedName(name);
			return mtstr("%union.") + name;
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
						TmplArgList = MTGetTypeName(TAT.getTypePtr(), bHasUnkownType);
					}
					else
					{
						TmplArgList = TmplArgList + "_" + MTGetTypeName(TAT.getTypePtr(),bHasUnkownType);
					}
				}

				mtstr name = RD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);

				return mtstr("%template.") + name + "." + TmplArgList;
			}
			else if(DesugaredType->isClassType())
			{
				mtstr name = RD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);
				return mtstr("%class.") + name;
			}
			else
			{
				mtstr name = RD->getQualifiedNameAsString();
				MTNormalizeQualifiedName(name);
				return mtstr("%struct.") + name;
			}
		}
		else if (DesugaredType->isEnumeralType())
		{
			const EnumType* ET = DesugaredType->getAs<EnumType>();
			const EnumDecl* ED = ET->getDecl();
			
			const clang::Type* Tmp = ED->getIntegerType().getTypePtr();
			mtstr basename = MTGetTypeName(Tmp, bHasUnkownType);

			//mtstr name = ED->getQualifiedNameAsString();
			//MTNormalizeQualifiedName(name);
			return basename;
		}
		else if (DesugaredType->isConstantArrayType())
		{
			const ConstantArrayType* CAT = dyn_cast<ConstantArrayType>(DesugaredType->getAsArrayTypeUnsafe());
			
			mtstr elt = MTGetTypeName(CAT->getElementType().getTypePtr(), bHasUnkownType);
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

			mtstr Pte = MTGetTypeName(PTE.getTypePtr(), bHasUnkownType);
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

				bHasUnkownType = true;
				return mtstr("(unkown)") + name;
			}
		}
		
		bHasUnkownType = true;
		return "(unkown)none";
	}



	mtstr MTGetTypeLayout(CXXRecordDecl* Declaration, bool& bHasUnkownType)
	{
		mtstr Result;
		mtstr TypeName = Declaration->getQualifiedNameAsString();
		MTNormalizeQualifiedName(TypeName);

		if (Declaration->isUnion())
		{
			Result = mtstr("%union.") + TypeName;
		}
		else if(Declaration->isTemplated())
		{
			Result = mtstr("%template.") + TypeName;
		}
		else if (Declaration->isClass())
		{
			Result = mtstr("%class.") + TypeName;
		}
		else
		{
			Result = mtstr("%struct.") + TypeName;
		}

		
		Optional<CharUnits> TypeSize = Context->getTypeSizeInCharsIfKnown(Declaration->getTypeForDecl());
		if (TypeSize.hasValue())
		{
			Result = Result + ":" + mtToString(TypeSize->getQuantity());
		}
		
		
		Result += " = \n{\n";

		bool bThisVirtual = false;
		for (const CXXMethodDecl* method : Declaration->methods())
		{
			if(method->isVirtual()) bThisVirtual = true;
		}

		bool bBaseVirtual = false;
		for (CXXBaseSpecifier& base : Declaration->bases())
		{
			if(MTHasVirtualTable(base.getType().getTypePtr())) bBaseVirtual = true;
		}

		if (bThisVirtual && !bBaseVirtual)
		{
			mtstr fielditem = "vftable:void**";
			Result += "    " + fielditem + ",\n";
		}

		int i = 0;
		for (CXXBaseSpecifier& base : Declaration->bases())
		{
			mtstr basename = MTGetTypeName(base.getType().getTypePtr(), bHasUnkownType);
			mtstr fielditem = "super." + std::to_string(i) + ":" + basename;
			i++;

			Result += "    " + fielditem + ",\n";
		}

		for (const FieldDecl* field : Declaration->fields())
		{
			QualType fieldType = field->getType();
			const clang::Type* DesugaredType = fieldType->getUnqualifiedDesugaredType();

			mtstr fname = field->getNameAsString();
			mtstr ftype = MTGetTypeName(DesugaredType, bHasUnkownType);
			mtstr fielditem = fname + ":" + ftype;

			if (field->isBitField())
			{
				int bitwidth = field->getBitWidthValue(*Context);
				fielditem = fielditem + ":" + mtToString(bitwidth);
			}


			Result += "    " + fielditem + ",\n";
		}

		Result += "};\n";

		return Result;
	}

	bool VisitCXXRecordDecl(CXXRecordDecl* Declaration)
	{
		if (!MTCheckFilter(Declaration))
		{
			return true;
		}

		bool bHasUnkownType = false;
		logs_line('-');
		mtstr layout = MTGetTypeLayout(Declaration, bHasUnkownType);
		if (bHasUnkownType)
		{
			MTAppendFile(CmdLineOption.WorkDir, "MetaTypeDecl.unkown.mtd", layout);
		}
		else
		{
			MTAppendFile(CmdLineOption.WorkDir, "MetaTypeDecl.mtd", layout);
		}
		logs_line('.');
		llvm::outs() << layout;
		logs_line('.');
		
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

static llvm::cl::OptionCategory MyToolCategory("metatools options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nMore help text...\n");
static cl::opt<mtstr> Opt_MTWorkDir("mt-work-dir", cl::desc("MetaTools Work Directory"), cl::value_desc("WorkDir"), cl::cat(MyToolCategory));

//////////////////////////////////////////////////////////////////////////


int main(int argc, const char** argv)
{
	CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
	CmdLineOption.WorkDir = Opt_MTWorkDir;

	logs_bar("MetaTools Begin");
	logs() << "MetaTools<CmdLineOption.WorkDir> = " << CmdLineOption.WorkDir << "\n";
	logs_line('-');
	
	
	
	
	ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

	bool bResult = Tool.run(newFrontendActionFactory<FindNamedClassAction>().get());

	logs_bar("MetaTools End");
	return bResult;
}