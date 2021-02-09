#pragma once

#include <memory>
#include <assert.h>
#include <stdexcept>

#include "fwds.hpp"
#include "math.hpp"

#include <DiligentCore/Common/interface/BasicMath.hpp>

#define DG_ASSERT(x) assert(x)

#define DG_THROW(x) throw std::runtime_error(x)

#define DG_DECL_PTR(T)                         \
    using Ptr = std::shared_ptr<T>;            \
    using ConstPtr = std::shared_ptr<const T>; \
    using WeakPtr = std::weak_ptr<T>;          \
    using UniquePtr = std::unique_ptr<T>;

#define DG_PTR(T)                     \
    DG_DECL_PTR(T)                    \
    template<typename... Args>        \
    static Ptr make(Args&&... args) { \
        return std::make_shared<T>(std::forward<Args>(args)...); \
    }                                 \
    template<typename... Args>        \
    static UniquePtr make_unique(Args&&... args) {            \
        return UniquePtr(new T(std::forward<Args>(args)...)); \
    }


#define DG_DECL_PTR_FWD(T)             \
    using T##Ptr = std::shared_ptr<T>; \
    using T##UniquePtr = std::unique_ptr<T>;


#define DG_STRINGIFY_(x) #x
#define DG_STRINGIFY(x) DG_STRINGIFY_(x)


#define DG_ENUM_FLAGS(EnumType)                                     \
    inline EnumType                                                 \
    operator&(EnumType a, EnumType b)                               \
    { return EnumType(static_cast<int>(a) & static_cast<int>(b)); } \
                                                                    \
    inline EnumType                                                 \
    operator|(EnumType a, EnumType b)                               \
    { return EnumType(static_cast<int>(a) | static_cast<int>(b)); } \
                                                                    \
    inline EnumType                                                 \
    operator^(EnumType a, EnumType b)                               \
    { return EnumType(static_cast<int>(a) ^ static_cast<int>(b)); } \
                                                                    \
    inline EnumType&                                                \
    operator|=(EnumType& a, EnumType b)                             \
    { return a = a | b; }                                           \
                                                                    \
    inline EnumType&                                                \
    operator&=(EnumType& a, EnumType b)                             \
    { return a = a & b; }                                           \
                                                                    \
    inline EnumType&                                                \
    operator^=(EnumType& a, EnumType b)                             \
    { return a = a ^ b; }                                           \
                                                                    \
    inline EnumType                                                 \
    operator~(EnumType a)                                           \
    { return EnumType(~static_cast<int>(a)); }


namespace dg {
using namespace Diligent;
}
