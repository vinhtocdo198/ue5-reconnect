// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Async/Future.h"

#define REDPOINT_AGGREGATE_3(StructName, Type1, Name1)                                                                 \
    struct StructName                                                                                                  \
    {                                                                                                                  \
        Type1 Name1;                                                                                                   \
        StructName(const Type1 &In##Name1)                                                                             \
            : Name1(In##Name1){};                                                                                      \
        [[nodiscard]] static TFuture<StructName> Future(const Type1 &In##Name1)                                        \
        {                                                                                                              \
            return MakeFulfilledPromise<StructName>(In##Name1).GetFuture();                                            \
        }                                                                                                              \
    }
#define REDPOINT_AGGREGATE_5(StructName, Type1, Name1, Type2, Name2)                                                   \
    struct StructName                                                                                                  \
    {                                                                                                                  \
        Type1 Name1;                                                                                                   \
        Type2 Name2;                                                                                                   \
        StructName(const Type1 &In##Name1, const Type2 &In##Name2)                                                     \
            : Name1(In##Name1)                                                                                         \
            , Name2(In##Name2){};                                                                                      \
        [[nodiscard]] static TFuture<StructName> Future(const Type1 &In##Name1, const Type2 &In##Name2)                \
        {                                                                                                              \
            return MakeFulfilledPromise<StructName>(In##Name1, In##Name2).GetFuture();                                 \
        }                                                                                                              \
    }
#define REDPOINT_AGGREGATE_7(StructName, Type1, Name1, Type2, Name2, Type3, Name3)                                     \
    struct StructName                                                                                                  \
    {                                                                                                                  \
        Type1 Name1;                                                                                                   \
        Type2 Name2;                                                                                                   \
        Type3 Name3;                                                                                                   \
        StructName(const Type1 &In##Name1, const Type2 &In##Name2, const Type3 &In##Name3)                             \
            : Name1(In##Name1)                                                                                         \
            , Name2(In##Name2)                                                                                         \
            , Name3(In##Name3){};                                                                                      \
        [[nodiscard]] static TFuture<StructName> Future(                                                               \
            const Type1 &In##Name1,                                                                                    \
            const Type2 &In##Name2,                                                                                    \
            const Type3 &In##Name3)                                                                                    \
        {                                                                                                              \
            return MakeFulfilledPromise<StructName>(In##Name1, In##Name2, In##Name3).GetFuture();                      \
        }                                                                                                              \
    }
#define REDPOINT_AGGREGATE_9(StructName, Type1, Name1, Type2, Name2, Type3, Name3, Type4, Name4)                       \
    struct StructName                                                                                                  \
    {                                                                                                                  \
        Type1 Name1;                                                                                                   \
        Type2 Name2;                                                                                                   \
        Type3 Name3;                                                                                                   \
        Type4 Name4;                                                                                                   \
        StructName(const Type1 &In##Name1, const Type2 &In##Name2, const Type3 &In##Name3, const Type4 &In##Name4)     \
            : Name1(In##Name1)                                                                                         \
            , Name2(In##Name2)                                                                                         \
            , Name3(In##Name3)                                                                                         \
            , Name4(In##Name4){};                                                                                      \
        [[nodiscard]] static TFuture<StructName> Future(                                                               \
            const Type1 &In##Name1,                                                                                    \
            const Type2 &In##Name2,                                                                                    \
            const Type3 &In##Name3,                                                                                    \
            const Type4 &In##Name4)                                                                                    \
        {                                                                                                              \
            return MakeFulfilledPromise<StructName>(In##Name1, In##Name2, In##Name3, In##Name4).GetFuture();           \
        }                                                                                                              \
    }
#define REDPOINT_AGGREGATE_11(StructName, Type1, Name1, Type2, Name2, Type3, Name3, Type4, Name4, Type5, Name5)        \
    struct StructName                                                                                                  \
    {                                                                                                                  \
        Type1 Name1;                                                                                                   \
        Type2 Name2;                                                                                                   \
        Type3 Name3;                                                                                                   \
        Type4 Name4;                                                                                                   \
        Type5 Name5;                                                                                                   \
        StructName(                                                                                                    \
            const Type1 &In##Name1,                                                                                    \
            const Type2 &In##Name2,                                                                                    \
            const Type3 &In##Name3,                                                                                    \
            const Type4 &In##Name4,                                                                                    \
            const Type5 &In##Name5)                                                                                    \
            : Name1(In##Name1)                                                                                         \
            , Name2(In##Name2)                                                                                         \
            , Name3(In##Name3)                                                                                         \
            , Name4(In##Name4)                                                                                         \
            , Name5(In##Name5){};                                                                                      \
        [[nodiscard]] static TFuture<StructName> Future(                                                               \
            const Type1 &In##Name1,                                                                                    \
            const Type2 &In##Name2,                                                                                    \
            const Type3 &In##Name3,                                                                                    \
            const Type4 &In##Name4,                                                                                    \
            const Type5 &In##Name5)                                                                                    \
        {                                                                                                              \
            return MakeFulfilledPromise<StructName>(In##Name1, In##Name2, In##Name3, In##Name4, In##Name5)             \
                .GetFuture();                                                                                          \
        }                                                                                                              \
    }

#define REDPOINT_AGGREGATE_GET(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, NAME, ...) NAME
#define REDPOINT_AGGREGATE_EXPAND(x) x
#define REDPOINT_AGGREGATE(...)                                                                                        \
    REDPOINT_AGGREGATE_EXPAND(REDPOINT_AGGREGATE_GET(                                                                  \
        __VA_ARGS__,                                                                                                   \
        REDPOINT_AGGREGATE_11,                                                                                         \
        INVALID_ARGUMENT_COUNT_10,                                                                                     \
        REDPOINT_AGGREGATE_9,                                                                                          \
        INVALID_ARGUMENT_COUNT_8,                                                                                      \
        REDPOINT_AGGREGATE_7,                                                                                          \
        INVALID_ARGUMENT_COUNT_6,                                                                                      \
        REDPOINT_AGGREGATE_5,                                                                                          \
        INVALID_ARGUMENT_COUNT_4,                                                                                      \
        REDPOINT_AGGREGATE_3,                                                                                          \
        INVALID_ARGUMENT_COUNT_2,                                                                                      \
        INVALID_ARGUMENT_COUNT_1)(__VA_ARGS__))