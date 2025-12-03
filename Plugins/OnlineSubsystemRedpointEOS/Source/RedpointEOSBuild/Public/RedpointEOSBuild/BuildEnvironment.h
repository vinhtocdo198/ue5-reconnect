// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// @note: clang-format mangles the _Pragma list, and separating with semicolons induces a warning itself
// clang-format off

// @note: I'd like to turn -Wswitch-enum back on, but it causes issues with switch statements over 
// EOS_EResult since we're never explicitly handling every error code.

#if defined(__clang__)
#define REDPOINT_EOS_CODE_GUARD_BEGIN()                                                                                \
    _Pragma("clang diagnostic push")                                                                                   \
    _Pragma("clang diagnostic error \"-Weverything\"")                                                                 \
    _Pragma("clang diagnostic ignored \"-Wpre-c++17-compat\"")                                                         \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"")                                                             \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat-pedantic\"")                                                    \
    _Pragma("clang diagnostic ignored \"-Wextra-semi\"")                                                               \
    _Pragma("clang diagnostic ignored \"-Wextra-semi-stmt\"")                                                          \
    _Pragma("clang diagnostic ignored \"-Wreserved-identifier\"")                                                      \
    _Pragma("clang diagnostic ignored \"-Wgnu-zero-variadic-macro-arguments\"")                                        \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"")                                                           \
    _Pragma("clang diagnostic ignored \"-Wcast-qual\"")                                                                \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"")                                                         \
    _Pragma("clang diagnostic ignored \"-Wexit-time-destructors\"")                                                    \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")                                                      \
    _Pragma("clang diagnostic ignored \"-Wcovered-switch-default\"")                                                   \
    _Pragma("clang diagnostic ignored \"-Wswitch-enum\"")                                                              \
    _Pragma("clang diagnostic ignored \"-Wweak-vtables\"")                                                             \
    _Pragma("clang diagnostic ignored \"-Wpadded\"")                                                                   \
    _Pragma("clang diagnostic ignored \"-Wunknown-warning-option\"")                                                   \
    _Pragma("clang diagnostic ignored \"-Wunsafe-buffer-usage\"")                                                      \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-experimental-coroutine\"")
#define REDPOINT_EOS_CODE_GUARD_END()                                                                                  \
    _Pragma("clang diagnostic pop")
#define REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()                                                                        \
    REDPOINT_EOS_CODE_GUARD_BEGIN()                                                                                    \
    _Pragma("clang diagnostic ignored \"-Wsuggest-destructor-override\"")                                              \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-copy-with-user-provided-dtor\"")                                  \
    _Pragma("clang diagnostic ignored \"-Winconsistent-missing-destructor-override\"")                                 \
    _Pragma("clang diagnostic ignored \"-Whidden-reinterpret-cast\"")                                                  \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-experimental-coroutine\"")                                        \
    _Pragma("clang diagnostic ignored \"-Wzero-as-null-pointer-constant\"")
#define REDPOINT_EOS_CODE_GUARD_END_UOBJECT()                                                                          \
    REDPOINT_EOS_CODE_GUARD_END()
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()                                                                   \
    _Pragma("clang diagnostic push")                                                                                   \
    _Pragma("clang diagnostic ignored \"-Wimplicit-float-conversion\"")                                                \
    _Pragma("clang diagnostic ignored \"-Wfloat-conversion\"")
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()                                                                     \
    _Pragma("clang diagnostic pop")
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_BEGIN()                                                                 \
    _Pragma("clang diagnostic push")                                                                                   \
    _Pragma("clang diagnostic ignored \"-Wimplicit-int-conversion\"")
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_END()                                                                   \
    _Pragma("clang diagnostic pop")
#define REDPOINT_EOS_CODE_GUARD_END_UOBJECT()                                                                          \
    REDPOINT_EOS_CODE_GUARD_END()
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()                                                                 \
    _Pragma("clang diagnostic push")                                                                                   \
    _Pragma("clang diagnostic ignored \"-Wunknown-warning-option\"")                                                   \
    _Pragma("clang diagnostic ignored \"-Winclude-what-you-use\"")
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END() \
    _Pragma("clang diagnostic pop")
#elif defined(_MSC_VER)
// @note: The __pragma calls must all be on one line, or MSVC gets very confused and can't map the 'pop' to the 'push'.
#define REDPOINT_EOS_CODE_GUARD_BEGIN() __pragma(warning(push,4)) __pragma(warning(error:4244)) __pragma(warning(error:4838))
#define REDPOINT_EOS_CODE_GUARD_END() __pragma(warning(pop))
#define REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT() REDPOINT_EOS_CODE_GUARD_BEGIN()
#define REDPOINT_EOS_CODE_GUARD_END_UOBJECT() REDPOINT_EOS_CODE_GUARD_END()
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN() __pragma(warning(push)) __pragma(warning(disable:4244))
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END() __pragma(warning(pop))
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_BEGIN() __pragma(warning(push)) __pragma(warning(disable:4244))
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_END() __pragma(warning(pop))
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()
#else
#define REDPOINT_EOS_CODE_GUARD_BEGIN()
#define REDPOINT_EOS_CODE_GUARD_END()
#define REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()
#define REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()
#define REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_BEGIN()
#define REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_END()
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_BEGIN()
#define REDPOINT_EOS_IWYU_ALLOW_UNUSED_HEADERS_END()
#endif
// clang-format on

#define REDPOINT_EOS_FILE_NS_ID(FileId, Ns) __File##FileId##_##Ns
#define REDPOINT_EOS_FILE_NS_EXPORT(FileId, Ns, Export) using ::__File##FileId##_##Ns::Export;
#define REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(FileId, Ns, Export)                                                 \
    namespace __File##FileId##_##Ns                                                                                    \
    {                                                                                                                  \
        class Export;                                                                                                  \
    }                                                                                                                  \
    namespace Ns                                                                                                       \
    {                                                                                                                  \
    using ::__File##FileId##_##Ns::Export;                                                                             \
    }
#define REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_STRUCT(FileId, Ns, Export)                                                \
    namespace __File##FileId##_##Ns                                                                                    \
    {                                                                                                                  \
        struct Export;                                                                                                 \
    }                                                                                                                  \
    namespace Ns                                                                                                       \
    {                                                                                                                  \
    using ::__File##FileId##_##Ns::Export;                                                                             \
    }
#define REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_TYPEDEF(FileId, Ns, Export, Alias)                                        \
    namespace __File##FileId##_##Ns                                                                                    \
    {                                                                                                                  \
        using namespace ::Ns;                                                                                          \
        typedef Alias Export;                                                                                          \
    }                                                                                                                  \
    namespace Ns                                                                                                       \
    {                                                                                                                  \
    using ::__File##FileId##_##Ns::Export;                                                                             \
    }
#define REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_ENUM(FileId, Ns, Export)                                                  \
    namespace __File##FileId##_##Ns                                                                                    \
    {                                                                                                                  \
        enum class Export : uint8;                                                                                     \
    }                                                                                                                  \
    namespace Ns                                                                                                       \
    {                                                                                                                  \
    using ::__File##FileId##_##Ns::Export;                                                                             \
    }

// Evaluates to 1 if the parameters passed to UE_LOG calls are required for compilation, and 0 if they are not. This
// allows lambda captures to be excluded in Shipping builds for platforms that use the Clang compiler, which would
// otherwise emit an error about unused lambda captures.
#if !NO_LOGGING || !defined(__clang__)
#define REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES 1
#else
#define REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES 0
#endif