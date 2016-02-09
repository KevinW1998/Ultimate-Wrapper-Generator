#include "UWrapperGeneratorVB6Declare.h"

#include "clang/AST/DeclCXX.h"
#include <boost/filesystem.hpp>

UWrapperGeneratorVB6Declare::UWrapperGeneratorVB6Declare(const std::string& genPath, bool ignoreUnsigned, bool ptrToLong) : UWrapperGenerator(genPath),
    m_enumStrDataBuf(""),
    m_funcStrDataBuf(""),
    m_typeStrDataBuf(""),
    m_ignoreUnsigned(ignoreUnsigned),
    m_ptrToLong(ptrToLong)
{
    m_dataStream.open(genPath + "/out.bas", std::ios::out | std::ios::binary);
}

void UWrapperGeneratorVB6Declare::Start()
{
    assert(m_dataStream.is_open() && "Data stream must be open!");
    m_dataStream << "Option Explicit" << std::endl 
        << std::endl;
}

void UWrapperGeneratorVB6Declare::End()
{
    m_dataStream << m_enumStrDataBuf 
        << m_typeStrDataBuf
        << m_funcStrDataBuf << std::endl;
    m_dataStream.close();
}

void UWrapperGeneratorVB6Declare::ProcessFuncDecl(clang::FunctionDecl* func)
{
    clang::QualType funcReturnType = func->getReturnType();
    bool isVoidReturn = funcReturnType->isVoidType();
    std::string vb6WrapperLine = "Public Declare ";
    vb6WrapperLine += (isVoidReturn ? "Sub " : "Function ");
    vb6WrapperLine += UASTUtils::FindName(func);
    vb6WrapperLine += " Lib \"" + m_libName + "\" (";
    for (clang::ParmVarDecl* nextParameter : func->parameters()) {
        bool isRef = false;
        std::string vb6Type = ClangTypeToVB6(nextParameter->getType(), nullptr, true, &isRef);
        vb6WrapperLine += (isRef ? "ByRef " : "ByVal ");
        vb6WrapperLine += nextParameter->getName().data();
        vb6WrapperLine += " As ";
        vb6WrapperLine += vb6Type;
        vb6WrapperLine += ", ";
    }
    // Remove additional ", "
    if (func->getNumParams() != 0)
        vb6WrapperLine = vb6WrapperLine.substr(0, vb6WrapperLine.length() - 2);

    if (!isVoidReturn) {
        vb6WrapperLine += ") As ";
        vb6WrapperLine += ClangTypeToVB6(funcReturnType, nullptr, false);
    }
    else 
    {
        vb6WrapperLine += ")";
    }
    
    m_funcStrDataBuf += vb6WrapperLine + "\n";
}

void UWrapperGeneratorVB6Declare::ProcessEnumDecl(clang::EnumDecl* enumDecl)
{
    std::string vb6WrapperLine = "Public Enum ";
    vb6WrapperLine += UASTUtils::FindName(enumDecl);
    vb6WrapperLine += "\n";
    for (clang::EnumConstantDecl* nextConst : enumDecl->enumerators()) {
        vb6WrapperLine += "\t";
        vb6WrapperLine += nextConst->getName();
        vb6WrapperLine += " = ";
        vb6WrapperLine += std::to_string(nextConst->getInitVal().getExtValue());
        vb6WrapperLine += "\n";
    }
    vb6WrapperLine += "End Enum\n";

    m_enumStrDataBuf += vb6WrapperLine + "\n";
}

void UWrapperGeneratorVB6Declare::ProcessRecordDecl(clang::RecordDecl* record)
{
    if (record->isUnion()) {
        m_collectedRecords.erase(record);
        return;
    }
    
    
    std::string vb6WrapperLine = "";
    if (record->getDefinition()) {
        clang::TypeInfo recordTypeInfo = record->getASTContext().getTypeInfo(record->getTypeForDecl());
        vb6WrapperLine += "' Size = " + std::to_string(recordTypeInfo.Width / 8) + "\n";
    }
    vb6WrapperLine += "Public Type ";
    vb6WrapperLine += UASTUtils::FindName(record);
    vb6WrapperLine += "\n";
    for (clang::FieldDecl* nextField : record->fields()) {
        if(nextField->isFunctionOrFunctionTemplate())
            continue;
        
        bool convSuccess = false;
        std::string possibleType = ClangTypeToVB6(nextField->getType(), &convSuccess, false);

        vb6WrapperLine += "\t";
        vb6WrapperLine += nextField->getName();
        vb6WrapperLine += " As ";
        if (convSuccess)
        {
            vb6WrapperLine += possibleType;
        }
        else
        {
            clang::TypeInfo typeInfoOfUnsupported = nextField->getASTContext().getTypeInfo(nextField->getType());
            int byteWidthOfUnsupported = typeInfoOfUnsupported.Width / 8;
            int byteAlignOfUnsupported = typeInfoOfUnsupported.Align / 8;
            vb6WrapperLine += "Byte(" + std::to_string(byteWidthOfUnsupported) + ") 'Unsupported, filling with Byte-Array (size="
                + std::to_string(byteWidthOfUnsupported) + ", align=" + std::to_string(byteAlignOfUnsupported) + ")";
        }
        vb6WrapperLine += "\n";
    }
    vb6WrapperLine += "End Type\n";

    m_typeStrDataBuf += vb6WrapperLine + "\n";
}

void UWrapperGeneratorVB6Declare::Generate()
{
    // FIXME: DRY Fail
    for (const auto& nextDecl : m_collectedEnums) 
        ProcessEnumDecl(nextDecl);
    for (const auto& nextDecl : m_collectedFuncs)
        ProcessFuncDecl(nextDecl);
    for (const auto& nextDecl : m_collectedRecords)
        ProcessRecordDecl(nextDecl);
}

std::string UWrapperGeneratorVB6Declare::ClangBuiltinTypeToVB6(const clang::BuiltinType* type, bool* success /*= 0*/)
{
    if (success)
        *success = true;
    switch (type->getKind()) {
    case clang::BuiltinType::SChar:
    case clang::BuiltinType::Char_S:
    case clang::BuiltinType::UChar:
    case clang::BuiltinType::Char_U: return "Byte";
    case clang::BuiltinType::Char16: return "Integer";
    case clang::BuiltinType::Char32: return "Long";
    case clang::BuiltinType::Short: return "Integer";
    case clang::BuiltinType::Int: return "Long";
    case clang::BuiltinType::Long: return "Long";
    case clang::BuiltinType::Float: return "Single";
    case clang::BuiltinType::Double: return "Double";
    default:
        {
            if (m_ignoreUnsigned) {
                switch (type->getKind()) {
                case clang::BuiltinType::UShort: return "Integer";
                case clang::BuiltinType::UInt: return "Long";
                case clang::BuiltinType::ULong: return "Long";
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

#include <iostream>
std::string UWrapperGeneratorVB6Declare::ClangTypeToVB6(const clang::QualType& type, bool* success, bool canHaveRef, bool* isRef /*= 0*/)
{
    if (isRef)
        *isRef = false;

    if (type->isFunctionPointerType()) {
        return "Long";
    }
    else if (type->getTypeClass() == clang::Type::Builtin) {
        const clang::BuiltinType* builtInType = type->getAs<clang::BuiltinType>();
        return ClangBuiltinTypeToVB6(builtInType, success);
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
                if (m_ptrToLong)
                    return "Long";

                if (canHaveRef) {
                    if (isRef)
                        *isRef = true;
                    return ClangBuiltinTypeToVB6(pointeeTypeBuiltin, success);
                }
            }
        }
        if (m_ptrToLong)
            return "Long";

        std::string conv = ClangTypeToVB6(pointeeType, success, false);
        if (canHaveRef) {
            if (isRef)
                *isRef = true;
            return conv;
        }
        return "Long";
    }
    else if (type->getTypeClass() == clang::Type::Typedef) {
        return ClangTypeToVB6(type->getAs<clang::TypedefType>()->desugar(), success, canHaveRef, isRef);
    }
    else if (type->getTypeClass() == clang::Type::Elaborated) {
        return ClangTypeToVB6(type->getAs<clang::ElaboratedType>()->desugar(), success, canHaveRef, isRef);
    }
    else if (type->getTypeClass() == clang::Type::Paren) {
        return ClangTypeToVB6(type->getAs<clang::ParenType>()->desugar(), success, canHaveRef, isRef);
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

    
    std::cout << type->getTypeClassName() << std::endl;
    type->dump();
    

    return "<Unsupported>";
}
