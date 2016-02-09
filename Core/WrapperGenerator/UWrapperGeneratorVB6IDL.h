#ifndef UWrapperGeneratorVB6IDL_H
#define UWrapperGeneratorVB6IDL_H

#include "UWrapperGenerator.h"
#include <fstream>

class UWrapperGeneratorVB6IDL : public UWrapperGenerator {
private:
    std::ofstream m_dataIDLStream;
    std::ofstream m_dataDefStream;

    std::string m_enumStrDataBuf;
    std::string m_funcStrDataBuf;
    std::string m_typeStrDataBuf;

    bool m_ignoreUnsigned;
    bool m_ptrToLong;
public:
    UWrapperGeneratorVB6IDL(const std::string& genPath, bool ignoreUnsigned, bool ptrToLong);
    virtual ~UWrapperGeneratorVB6IDL() {}

    virtual bool IsReady() override final { return m_dataIDLStream.is_open(); };
    virtual void Start() override final;
    virtual void End() override final;

    
    virtual void ProcessFuncDecl(clang::FunctionDecl* func) override final; //< Process and validate (remove from list if needed) functions
    virtual void ProcessEnumDecl(clang::EnumDecl* enumDecl) override final; //< Process and validate (remove from list if needed) functions
    virtual void ProcessRecordDecl(clang::RecordDecl* record) override final; //< Process and validate (remove from list if needed) functions

    std::string FormatCommentIDL(const char* rawText);
    std::string FormatCommentDEF(const char* rawText);

private:
    std::string ClangBuiltinTypeToVB6Typelib(const clang::BuiltinType* type, bool* success = 0);
    std::string ClangTypeToVB6Typelib(const clang::QualType& type, bool* success = 0);
};

#endif