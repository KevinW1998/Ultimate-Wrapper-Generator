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

void UWrapperGeneratorVB6IDL::Generate()
{
    // FIXME: DRY Fail
    for (const auto& nextDecl : m_collectedEnums)
        ProcessEnumDecl(nextDecl);
    for (const auto& nextDecl : m_collectedFuncs)
        ProcessFuncDecl(nextDecl);
    for (const auto& nextDecl : m_collectedRecords)
        ProcessRecordDecl(nextDecl);
}

std::string UWrapperGeneratorVB6IDL::ClangBuiltinTypeToVB6Typelib(const clang::BuiltinType* type, bool* success /*= 0*/)
{
    return "";
}

std::string UWrapperGeneratorVB6IDL::ClangTypeToVB6Typelib(const clang::QualType& type, bool* success, bool canHaveRef, bool* isRef /*= 0*/)
{
    return "";
}
