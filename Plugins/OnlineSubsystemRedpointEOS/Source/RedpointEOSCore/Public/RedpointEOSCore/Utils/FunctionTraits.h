// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2472674857, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

#define REDPOINT_EOS_EXPANSIVE_FUNCTION_TRAIT_TEMPLATE(TemplateName)                                                   \
    template <typename ReturnType, typename... Args>                                                                   \
    struct TemplateName<ReturnType (*)(Args...)> : public TemplateName<ReturnType(Args...)>                            \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template <typename ClassType, typename ReturnType, typename... Args>                                               \
    struct TemplateName<ReturnType (ClassType::*)(Args...)> : public TemplateName<ReturnType(Args...)>                 \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template <typename ClassType, typename ReturnType, typename... Args>                                               \
    struct TemplateName<ReturnType (ClassType::*)(Args...) const> : public TemplateName<ReturnType(Args...)>           \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template <typename ClassType, typename ReturnType, typename... Args>                                               \
    struct TemplateName<ReturnType (ClassType::*)(Args...) volatile> : public TemplateName<ReturnType(Args...)>        \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template <typename ClassType, typename ReturnType, typename... Args>                                               \
    struct TemplateName<ReturnType (ClassType::*)(Args...) const volatile> : public TemplateName<ReturnType(Args...)>  \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template <typename FunctionType> struct TemplateName<TFunction<FunctionType>> : public TemplateName<FunctionType>  \
    {                                                                                                                  \
    }

}

REDPOINT_EOS_CODE_GUARD_END()