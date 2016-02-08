#include "UWrapperGeneratorVB6IDL.h"

UWrapperGeneratorVB6IDL::UWrapperGeneratorVB6IDL(const std::string& genPath) : UWrapperGenerator(genPath)
{
    m_dataStream.open(genPath + "/out.idl", std::ios::out | std::ios::binary);
}

void UWrapperGeneratorVB6IDL::Start()
{
    assert(m_dataStream.is_open() && "Data stream must be open!");
    m_dataStream << "[uuid(F1B9E420-F306-11d1-996A-92FF02C40D32), helpstring(\"FIXME: Helpstr\"), version(1.0)]\n"
        "library FIXME_LIB\n"
        "{\n";
}

void UWrapperGeneratorVB6IDL::End()
{
    m_dataStream << "}";
    m_dataStream.close();
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

}

std::string UWrapperGeneratorVB6IDL::ClangTypeToVB6Typelib(const clang::QualType& type, bool* success, bool canHaveRef, bool* isRef /*= 0*/)
{

}
