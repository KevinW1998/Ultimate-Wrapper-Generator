#ifndef UWrapperGenerator_H
#define UWrapperGenerator_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"

#include "../UASTUtils.h"

#include <unordered_set>
#include <vector>
#include <set>

class UWrapperGenerator {
protected:
    std::string m_libName;
    std::string m_genPath;
    std::unordered_set<clang::EnumDecl*> m_parsedEnumDecls;
    std::unordered_set<clang::RecordDecl*> m_parsedCXXRecordDecls;

    std::set<clang::FunctionDecl*> m_collectedFuncs;
    std::set<clang::EnumDecl*> m_collectedEnums;
    std::set<clang::RecordDecl*> m_collectedRecords;
public:
    UWrapperGenerator(std::string genPath) : m_genPath(genPath)
    {}
    virtual ~UWrapperGenerator() {}

    // General
    virtual bool IsReady() { return false; } //< Returns true, if the generator is ready for generating!
    virtual void Start() {} //< Code when the generator is starting!
    virtual void End() {} //< Code when the generator is starting!

    // Gathering
    virtual void NextFuncDecl(clang::FunctionDecl* func);
    
    // Generation - Function
    virtual void ProcessFuncDecl(clang::FunctionDecl* func) {}
    
    // Generation - Enum
    virtual void ProcessEnumDecl(clang::EnumDecl* enumDecl) {} //< Process and validate (remove from list if needed) functions
    
    // Generation - Record
    virtual void ProcessRecordDecl(clang::RecordDecl* record) {} //< Process and validate (remove from list if needed) functions
    
    void Generate();

    std::string getLibName() const { return m_libName; }
    void setLibName(const std::string& val) { m_libName = val;}

private:
    void ProcessTypeAdd(const clang::QualType& typeToProcess);
};

#endif
