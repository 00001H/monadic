#pragma once
#include"commons.hpp"
#include<cppp/variant.hpp>
#include<cppp/string.hpp>
#include<functional>
#include<cstdint>
#include<vector>
namespace mnd::impl{
    class Value;
    struct function{
        std::move_only_function<Value(Value&&)> fn;
    };
    struct function_ref{
        std::move_only_function<Value(Value&&)> fn;
    };
    class Value{
        using vv_t = cppp::heap_variant<std::uint64_t,float,cppp::str,std::vector<Value>,function,function_ref>;
        vv_t v;
        public:
            template<typename ...A>
            Value(A&& ...a) : v(std::forward<A>(a)...){}
            template<typename T>
            bool has() const{
                return v.index() == v.index_of<T>;
            }
            std::size_t index() const{
                return v.tell();
            }
            template<typename T>
            constexpr static std::size_t index_of = vv_t::index_of<T>;
            template<typename T>
            T& get(){
                return v.get<T>();
            }
            template<typename T>
            const T& get() const{
                return v.get<T>();
            }
            explicit operator bool() const{
                return bool(v);
            }
    };
}
namespace mnd{
    MND_EXPORT function;
    MND_EXPORT function_ref;
    MND_EXPORT Value;
}
