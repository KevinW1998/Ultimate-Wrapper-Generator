#include "UWrapperGeneratorVB6.h"

#include "clang/AST/DeclCXX.h"


namespace UVB6Helper {
    const char* ClangBuiltinTypeToVB6(const clang::BuiltinType* type) {
        switch (type->getKind()) {
        case clang::BuiltinType::UChar: return "Byte";
        case clang::BuiltinType::Char16: return "Integer";
        case clang::BuiltinType::Char32: return "Long";
        case clang::BuiltinType::Short: return "Integer";
        case clang::BuiltinType::Int: return "Long";
        case clang::BuiltinType::Long: return "Long";
        case clang::BuiltinType::Float: return "Single";
        case clang::BuiltinType::Double: return "Double";
        default:
            return "<Unsupported>";
        }
    }

    const char* ClangTypeToVB6(const clang::QualType& type, bool canHaveRef, bool* isRef = 0) {
        if(isRef)
            *isRef = false;
        if (type->getTypeClass() == clang::Type::Builtin) {
            const clang::BuiltinType* builtInType = type->getAs<clang::BuiltinType>();
            return ClangBuiltinTypeToVB6(builtInType);
        }
        else if (type->getTypeClass() == clang::Type::Pointer) {
            const clang::PointerType* pointerType = type->getAs<clang::PointerType>();
            const clang::QualType& pointeeType = pointerType->getPointeeType();
            if (pointeeType->getTypeClass() == clang::Type::Builtin) {
                const clang::BuiltinType* pointeeTypeBuiltin = pointeeType->getAs<clang::BuiltinType>();
                switch (pointeeTypeBuiltin->getKind()) {
                case clang::BuiltinType::SChar:
                case clang::BuiltinType::Char_S:
                case clang::BuiltinType::UChar:
                case clang::BuiltinType::Char_U: return "String";
                default:
                    if (canHaveRef) {
                        if (isRef)
                            *isRef = true;
                        return ClangBuiltinTypeToVB6(pointeeTypeBuiltin);
                    }                        
                }
            }
            return "Long";
        }
        return "<Unsupported>";
    }
}

void UWrapperGeneratorVB6::Start()
{
    assert(m_dataStream.is_open(), "Data stream must be open!");
    m_dataStream << "Attribute VB_Name = \"fixme_sample\"" << std::endl
        << "Option Explicit" << std::endl 
        << std::endl;
}

void UWrapperGeneratorVB6::End()
{
    m_dataStream << m_enumStrDataBuf << m_funcStrDataBuf << std::endl;
    m_dataStream.close();
}

void UWrapperGeneratorVB6::NextFuncDecl(clang::FunctionDecl* func)
{
    clang::QualType funcReturnType = func->getReturnType();
    bool isVoidReturn = funcReturnType->isVoidType();
    std::string vb6WrapperLine = "Public Declare ";
    vb6WrapperLine += (isVoidReturn ? "Sub " : "Function ");
    vb6WrapperLine += func->getName();
    vb6WrapperLine += " Lib \"" + m_libName + "\" (";

    
    for (clang::ParmVarDecl* nextParameter : func->parameters()) {
        bool isRef = false;
        const char* vb6Type = UVB6Helper::ClangTypeToVB6(nextParameter->getType(), true, &isRef);
        vb6WrapperLine += (isRef ? "ByRef " : "ByVal ");
        vb6WrapperLine += nextParameter->getName().data();
        vb6WrapperLine += " As ";
        vb6WrapperLine += vb6Type;
        vb6WrapperLine += ", ";
    }
    // Remove additional ", "
    if (func->getNumParams() != 0)
        vb6WrapperLine = vb6WrapperLine.substr(0, vb6WrapperLine.length() - 2);

    if (!isVoidReturn) {
        vb6WrapperLine += ") As ";
        vb6WrapperLine += UVB6Helper::ClangTypeToVB6(funcReturnType, false);
    }
    else 
    {
        vb6WrapperLine += ")";
    }
    
    m_funcStrDataBuf += vb6WrapperLine + "\n";
}

void UWrapperGeneratorVB6::NextEnumDecl(clang::EnumDecl* enumDecl)
{
    NextEnumDecl(enumDecl, enumDecl->getName());
}

void UWrapperGeneratorVB6::NextEnumDecl(clang::EnumDecl* enumDecl, const std::string& name)
{
    std::string vb6WrapperLine = "Public Enum ";
    vb6WrapperLine += name;
    vb6WrapperLine += "\n";
    for (clang::EnumConstantDecl* nextConst : enumDecl->enumerators()) {
        vb6WrapperLine += "\t";
        vb6WrapperLine += nextConst->getName();
        vb6WrapperLine += " = ";
        vb6WrapperLine += std::to_string(nextConst->getInitVal().getExtValue());
        vb6WrapperLine += "\n";
    }
    vb6WrapperLine += "End Enum\n";

    m_enumStrDataBuf += vb6WrapperLine + "\n";
}
