#include "UWrapperGeneratorVB6IDL.h"
#include "../Resources/UStrResources.h"
#include <boost/algorithm/string/replace.hpp>

UWrapperGeneratorVB6IDL::UWrapperGeneratorVB6IDL(const std::string& genPath, bool ignoreUnsigned, bool ptrToLong) : 
    UWrapperGenerator(genPath),
    m_ignoreUnsigned(ignoreUnsigned),
    m_ptrToLong(ptrToLong)
{
    m_dataIDLStream.open(genPath + "/out.idl", std::ios::out | std::ios::binary);
    m_dataDefStream.open(genPath + "/out.def", std::ios::out | std::ios::binary);
}

void UWrapperGeneratorVB6IDL::Start()
{
    assert(m_dataIDLStream.is_open() && "Data stream must be open!");
    m_dataIDLStream << FormatCommentIDL(GEN_HEADER)
        << 
        "[uuid(F1B9E420-F306-11d1-996A-92FF02C40D32), helpstring(\"FIXME: Helpstr\"), version(1.0)]\n"
        "library FIXME_LIB\n"
        "{\n";

    m_dataDefStream << FormatCommentDEF(GEN_HEADER)
        <<
        "\n"
        "; This def file is needed, if you compile with MSVC.\n"
        "; If you use MinGW, then use the command line arguments: \n"
        "; -Wl,--kill-at\n"
        "\n"
        "LIBRARY\n"
        "EXPORTS\n";
}

void UWrapperGeneratorVB6IDL::End()
{
    m_dataIDLStream << "}";
    m_dataIDLStream.close();
    m_dataDefStream.close();
}

void UWrapperGeneratorVB6IDL::ProcessFuncDecl(clang::FunctionDecl* func)
{
    std::string genDefStr = "\t" + (std::string)func->getName() + "\n";
    m_dataDefStream << genDefStr;

    std::string typelibWrapperLine = "";

    m_funcStrDataBuf += typelibWrapperLine + "\n";
}

void UWrapperGeneratorVB6IDL::ProcessEnumDecl(clang::EnumDecl* enumDecl)
{

}

void UWrapperGeneratorVB6IDL::ProcessRecordDecl(clang::RecordDecl* record)
{

}

std::string UWrapperGeneratorVB6IDL::FormatCommentIDL(const char* rawText)
{
    return std::string("/* \n * ") + boost::replace_all_copy<std::string>(rawText, "\n", "\n * ") + "\n */\n\n";
}

std::string UWrapperGeneratorVB6IDL::FormatCommentDEF(const char* rawText)
{
    return std::string("; ") + boost::replace_all_copy<std::string>(rawText, "\n", "\n; ") + "\n\n";
}

std::string UWrapperGeneratorVB6IDL::ClangBuiltinTypeToVB6Typelib(const clang::BuiltinType* type, bool* success /*= 0*/)
{
    if (success)
        *success = true;
    switch (type->getKind()) {
        case clang::BuiltinType::Bool: return "Boolean";
        case clang::BuiltinType::SChar:
        case clang::BuiltinType::Char_S:
        case clang::BuiltinType::UChar:
        case clang::BuiltinType::Char_U: return "char";
        case clang::BuiltinType::Char16: return "short";
        case clang::BuiltinType::Char32: return "long";
        case clang::BuiltinType::Short: return "short";
        case clang::BuiltinType::Int: return "long";
        case clang::BuiltinType::Long: return "long";
        case clang::BuiltinType::Float: return "float";
        case clang::BuiltinType::Double: return "double";
        default:
        {
            if (m_ignoreUnsigned) {
                switch (type->getKind()) {
                case clang::BuiltinType::UShort: return "short";
                case clang::BuiltinType::UInt: return "long";
                case clang::BuiltinType::ULong: return "long";
                default:
                    break;
                }
            }
            if (success)
                *success = false;
            return "<Unsupported>";
        }
    }
}

std::string UWrapperGeneratorVB6IDL::ClangTypeToVB6Typelib(const clang::QualType& type, bool* success)
{
    if (type->isFunctionPointerType()) {
        return "Long";
    }
    else if (type->getTypeClass() == clang::Type::Builtin) {
        const clang::BuiltinType* builtInType = type->getAs<clang::BuiltinType>();
        return ClangBuiltinTypeToVB6Typelib(builtInType, success);
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
            case clang::BuiltinType::Char_U: return "String"; //char* --> String
            case clang::BuiltinType::Void: return "Long"; // void* --> Long
            default:
                
                break;
            }
        }
        return "Long";
    }
    else if (type->getTypeClass() == clang::Type::Typedef) {
        return ClangTypeToVB6Typelib(type->getAs<clang::TypedefType>()->desugar(), success);
    }
    else if (type->getTypeClass() == clang::Type::Elaborated) {
        return ClangTypeToVB6Typelib(type->getAs<clang::ElaboratedType>()->desugar(), success);
    }
    else if (type->getTypeClass() == clang::Type::Paren) {
        return ClangTypeToVB6Typelib(type->getAs<clang::ParenType>()->desugar(), success);
    }
    else if (type->getTypeClass() == clang::Type::Enum) {
        const clang::EnumType* enumType = type->getAs<clang::EnumType>();
        if (m_collectedEnums.find(enumType->getDecl()) != m_collectedEnums.end()) {
            return clang::QualType(enumType, 0).getAsString();
        }
    }
    else if (type->getTypeClass() == clang::Type::Record) {
        const clang::RecordType* recType = type->getAs<clang::RecordType>();
        if (m_collectedRecords.find(recType->getDecl()) != m_collectedRecords.end())
            return recType->getDecl()->getNameAsString();
    }
    if (success)
        *success = false;




    return "<Unsupported>";
}
