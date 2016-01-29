#ifndef UWrapperGeneratorVB6_H
#define UWrapperGeneratorVB6_H

#include "UWrapperGenerator.h"
#include "../UStartupArgs.h"
#include <fstream>

class UWrapperGeneratorVB6 : public UWrapperGenerator {
private:
    std::ofstream m_dataStream;
    std::string m_enumStrDataBuf;
    std::string m_funcStrDataBuf;
    std::string m_typeStrDataBuf;

    bool m_ignoreUnsigned;
    
public:
    UWrapperGeneratorVB6(const std::string& basFile, bool ignoreUnsigned) : 
        // Replace * with bas at the end, if it is given (i.e. test.* --> test.bas)
        m_dataStream((basFile.find("*") == basFile.length() - 1 ? basFile.substr(0, basFile.length() - 1) + "bas" : basFile), std::ios::out | std::ios::binary),
        m_enumStrDataBuf(""),
        m_funcStrDataBuf(""),
        m_typeStrDataBuf(""),
        m_ignoreUnsigned(ignoreUnsigned)
    {
    
    }
    virtual ~UWrapperGeneratorVB6() {}

    virtual bool IsReady() override final { return m_dataStream.is_open(); };
    virtual void Start() override final;
    virtual void End() override final;

    
    void ProcessFuncDecl(clang::FunctionDecl* func);
    void ProcessEnumDecl(clang::EnumDecl* enumDecl);
    void ProcessEnumDecl(clang::EnumDecl* enumDecl, const std::string& name);
    void ProcessRecordDecl(clang::RecordDecl* record);

    virtual void Generate();

private:
    std::string ClangBuiltinTypeToVB6(const clang::BuiltinType* type);
    std::string ClangTypeToVB6(const clang::QualType& type, bool canHaveRef, bool* isRef = 0);
};

#endif