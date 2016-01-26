#ifndef UWrapperGenerator_H
#define UWrapperGenerator_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"

class UWrapperGenerator {
protected:
    std::string m_libName;
public:
    UWrapperGenerator() {}
    virtual ~UWrapperGenerator() {}

    virtual bool IsReady() { return false; } //< Returns true, if the generator is ready for generating!
    virtual void Start() {} //< Code when the generator is starting!
    virtual void End() {} //< Code when the generator is starting!

    virtual void NextFuncDecl(clang::FunctionDecl* /*func*/) {}
    virtual void NextEnumDecl(clang::EnumDecl* /*enumDecl*/) {}
    virtual void NextEnumDecl(clang::EnumDecl* /*enumDecl*/, const std::string& /*name*/) {}

    std::string getLibName() const { return m_libName; }
    void setLibName(const std::string& val) { m_libName = val;}
};

#endif
