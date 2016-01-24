#ifndef UASTConsumer_H
#define UASTConsumer_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "WrapperGenerator/UWrapperGenerator.h"

class UASTConsumer : public clang::ASTConsumer
{
private:
    UWrapperGenerator* m_generator;

public:
    UASTConsumer(UWrapperGenerator* generator) : 
        clang::ASTConsumer(),
        m_generator(generator) 
    { }
    virtual ~UASTConsumer() { }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef d) override final;
};

#endif


