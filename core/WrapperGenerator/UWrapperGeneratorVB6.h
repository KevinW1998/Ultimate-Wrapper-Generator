#ifndef UWrapperGeneratorVB6_H
#define UWrapperGeneratorVB6_H

#include "UWrapperGenerator.h"
#include <fstream>

class UWrapperGeneratorVB6 : public UWrapperGenerator {
private:
    std::ofstream m_dataStream;
public:
    UWrapperGeneratorVB6(const std::string& basFile)
        : m_dataStream(basFile, std::ios::out | std::ios::binary) {}
    virtual ~UWrapperGeneratorVB6() {}

    virtual bool IsReady() override final { return m_dataStream.is_open(); };
    virtual void Start() override final;
    virtual void End() override final;

    
    virtual void NextFuncDecl(clang::FunctionDecl* func) override final;
    virtual void NextEnumDecl(clang::EnumDecl* enumDecl) override final;
    virtual void NextEnumDecl(clang::EnumDecl* enumDecl, const std::string& name) override final;


};

#endif