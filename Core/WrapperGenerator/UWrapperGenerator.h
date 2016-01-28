#ifndef UWrapperGenerator_H
#define UWrapperGenerator_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"

#include <unordered_set>

class UWrapperGenerator {
protected:
    std::string m_libName;
    std::unordered_set<clang::EnumDecl*> m_parsedEnumDecls;
    std::unordered_set<clang::RecordDecl*> m_parsedCXXRecordDecls;


public:
    UWrapperGenerator() {}
    virtual ~UWrapperGenerator() {}

    virtual bool IsReady() { return false; } //< Returns true, if the generator is ready for generating!
    virtual void Start() {} //< Code when the generator is starting!
    virtual void End() {} //< Code when the generator is starting!

    virtual void NextFuncDecl(clang::FunctionDecl* /*func*/) {}
    virtual void NextEnumDecl(clang::EnumDecl* enumDecl) { m_parsedEnumDecls.insert(enumDecl); }
    virtual void NextEnumDecl(clang::EnumDecl* enumDecl, const std::string& /*name*/) { m_parsedEnumDecls.insert(enumDecl); }
    virtual void NextCXXRecordDecl(clang::CXXRecordDecl* cxx) {  m_parsedCXXRecordDecls.insert(cxx); }

    std::string getLibName() const { return m_libName; }
    void setLibName(const std::string& val) { m_libName = val;}
};

#endif
