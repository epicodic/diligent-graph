#pragma once

#include <vector>
#include <map>
#include <sstream>
#include <functional>
#include <type_traits>

#include "common.hpp"


namespace dg {


inline std::vector<std::string> split(std::istream& stream, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(stream, token, delimiter))
        tokens.push_back(token);
    return tokens;
}

template <std::size_t... Is>
struct indices {}; // NOLINT

template <std::size_t N, std::size_t... Is>
struct build_indices // NOLINT
  : build_indices<N-1, N-1, Is...> {};

template <std::size_t... Is>
struct build_indices<0, Is...> : indices<Is...> {}; // NOLINT




namespace detail
{
    template <typename F>
    struct function_traits : public function_traits<decltype(&F::operator())> {}; // NOLINT

    template <typename R, typename C, typename... Args>
    struct function_traits<R (C::*)(Args...) const>
    {
        using function_type = std::function<R (Args...)>;
    };
}


template <typename T>
inline T parseFromString(const std::string& s)
{
    std::stringstream is(s);
    T value; is >> value;
    return value;
}

template <>
inline std::string parseFromString<std::string>(const std::string& s)
{
    return s;
}


struct IMethodInvoker
{
    typedef std::vector<std::string> Parameters;

    virtual ~IMethodInvoker() = default;
    virtual std::string call(const Parameters& params) = 0;

    DG_DECL_PTR(IMethodInvoker);

    template <typename T>
    using ValueType = typename std::remove_const<typename std::remove_reference<T>::type>::type;


    template <typename T>
    ValueType<T> parse(const std::string& s)
    {
        return parseFromString<ValueType<T>>(s);
    }
};

template<typename R, typename... Args>
struct MethodInvoker : public IMethodInvoker
{
    std::function<R(Args...)> fn;

    template<std::size_t... Is>
    void call_(const Parameters& params, indices<Is...>) // NOLINT
    {
        fn(parse<Args>(params[Is])...);
    }

    virtual std::string call(const Parameters& params)
    {
        call_(params,build_indices<sizeof...(Args)>{}); // NOLINT
        return std::string(); // TODO: return value
    }
};


class MethodStringInterface
{
public:

    typedef std::pair<std::string, int> MethodKey; // method name + args count

    template<typename R, typename... Args,  typename Class>
    void registerMethod(const char* name, R (Class::*fn)(Args ...), Class* This)
    {
        MethodInvoker<R,Args...>* invoker = new MethodInvoker<R,Args...>();
        invoker->fn = [This,fn](Args... args)->R { return (This->*fn)(args...); };
        methods_[MethodKey(name, sizeof...(Args))] = IMethodInvoker::Ptr(invoker); 

    }

    template<typename R, typename... Args>
    void registerMethod(const char* name, std::function<R (Args ...)> fn)
    {
        MethodInvoker<R,Args...>* invoker = new MethodInvoker<R,Args...>();
        invoker->fn = fn;
        methods_[MethodKey(name, sizeof...(Args))] = IMethodInvoker::Ptr(invoker);
    }

    template<typename Lambda>
    void registerMethod(const char* name, const Lambda& lambda)
    {
        using function_type = typename detail::function_traits<Lambda>::function_type;
        registerMethod(name, static_cast<function_type>(lambda));
    }

    std::string callMethod(const std::string& call_string)
    {
        std::istringstream stream (call_string);

        std::string method_name;
        stream >> method_name;

        std::vector<std::string> params;
        params = split(stream, ',');

        auto it = methods_.find(MethodKey(method_name, params.size()));
        if(it==methods_.end())
            DG_THROW("So such method found: " + method_name + " (" + std::to_string(params.size()) + " parameters)");

        return it->second->call(params);
    }


    bool empty() const { return methods_.empty(); }

private:

    std::map<MethodKey, IMethodInvoker::Ptr> methods_;

};

}
