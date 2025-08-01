#include<mnd/mnd.hpp>
#include<cppp/polyfill/pack-indexing.hpp>
#include<cppp/string.hpp>
#include<cppp/strmap.hpp>
#include<filesystem>
#include<charconv>
#include<iostream>
#include<fstream>
#include<stack>
using namespace std::literals;
std::uint8_t htoi(char8_t c){
    switch(c){
        case u8'0': return 0;
        case u8'1': return 1;
        case u8'2': return 2;
        case u8'3': return 3;
        case u8'4': return 4;
        case u8'5': return 5;
        case u8'6': return 6;
        case u8'7': return 7;
        case u8'8': return 8;
        case u8'9': return 9;
        case u8'a': return 10;
        case u8'b': return 11;
        case u8'c': return 12;
        case u8'd': return 13;
        case u8'e': return 14;
        case u8'f': return 15;
        default: {
            std::cerr << "Error: Invalid hex"sv << std::endl;
            std::exit(1);
        }
    }
}
class Runtime{
    std::stack<mnd::Value> v;
    public:
        void put(mnd::Value val){
            if(v.empty()||val.has<mnd::function>()){
                v.emplace(std::move(val));
            }else if(!v.top().has<mnd::function>()){
                throw std::logic_error("Cannot run non-function");
            }else{
                mnd::Value res = v.top().get<mnd::function>().fn(std::move(val));
                v.pop();
                put(std::move(res));
            }
        }
        void pop(){
            v.pop();
        }
        const mnd::Value& top() const{
            return v.top();
        }
        mnd::Value& top(){
            return v.top();
        }
        std::size_t ss() const{
            return v.size();
        }
};
template<bool endl>
mnd::Value printv(const mnd::Value& v){
    switch(v.index()){
        case mnd::Value::index_of<std::uint64_t>:
            std::cout << v.get<std::uint64_t>();
            break;
        case mnd::Value::index_of<float>:
            std::cout << v.get<float>();
            break;
        case mnd::Value::index_of<cppp::str>:
            std::cout << cppp::cview(v.get<cppp::str>());
            break;
        case mnd::Value::index_of<mnd::function_ref>:
            std::cout << "<function>"sv;
            break;
        case mnd::Value::index_of<mnd::function>:
            std::cout << "<function call>"sv;
            break;
        default:
            std::cerr << "Error: Unprintable value"sv << std::endl;
            std::exit(1);
    }
    if constexpr(endl){
        std::endl(std::cout);
    }else{
        std::flush(std::cout);
    }
    return mnd::Value();
}
namespace detail{
    template<typename ...A>
    struct with_args{
        template<typename F>
        class curry{
            using ret = std::invoke_result_t<F,A...>;
            using arg = cppp::compat::index_pack<0uz,A...>&&;
            template<typename _,typename ...Rest>
            auto _invoke(arg value) &&{
                if constexpr(sizeof...(Rest)==0){
                    return inst(std::forward<arg>(value));
                }else{
                    typename with_args<Rest...>::curry c{[c=std::move(*this),p=std::forward<arg>(value)] <typename ...T>(T&& ...v) mutable -> ret {
                        return c.inst(std::forward<arg>(p),std::forward<T>(v)...);
                    }};
                    return mnd::Value(cppp::emplace_tag<mnd::function>,std::move(c));
                }
            }
            std::conditional_t<std::is_function_v<F>,F&,F> inst;
            public:
                curry(F&& f) : inst(std::forward<F>(f)){}
                auto operator()(arg value){
                    return std::move(*this).template _invoke<A...>(std::forward<arg>(value));
                }
        };
    };
}
template<typename ...A>
struct curry{
    template<typename T>
    static auto of(T&& t){
        return typename detail::with_args<A...>::curry(std::forward<T>(t));
    }
};
template<typename T>
mnd::Value addv(mnd::Value&& lhs,mnd::Value&& rhs){
    if(!lhs.has<T>()||!rhs.has<T>()){
        std::cerr << "Error: Bad arguments to addv"sv << std::endl;
        std::exit(1);
    }
    return mnd::Value(lhs.get<T>()+rhs.get<T>());
}
mnd::Value mcall(mnd::Value&& lhs,mnd::Value&& rhs){
    if(!lhs.has<mnd::function_ref>()){
        std::cerr << "Error: Trying to mcall non-function"sv << std::endl;
        std::exit(1);
    }
    return lhs.get<mnd::function_ref>().fn(std::move(rhs));
}
mnd::Value dcall(mnd::Value&& lhs,mnd::Value&& mhs,mnd::Value&& rhs){
    if(!lhs.has<mnd::function_ref>()){
        std::cerr << "Error: Trying to dcall non-function"sv << std::endl;
        std::exit(1);
    }
    return mcall(lhs.get<mnd::function_ref>().fn(std::move(mhs)),std::move(rhs));
}
int main(){
    std::fstream ifs{std::filesystem::path(u8"test/main.mnd"sv),std::ios_base::in};
    std::string buf;
    cppp::strmap<std::function<mnd::Value(mnd::Value&&)>> ftab{
        {u8"print"s,printv<false>},
        {u8"println"s,printv<true>},
        {u8"addi"s,curry<mnd::Value&&,mnd::Value&&>::of(addv<std::uint64_t>)},
        {u8"addf"s,curry<mnd::Value&&,mnd::Value&&>::of(addv<float>)},
        {u8"mcall"s,curry<mnd::Value&&,mnd::Value&&>::of(mcall)},
        {u8"dcall"s,curry<mnd::Value&&,mnd::Value&&,mnd::Value&&>::of(dcall)},
    };
    Runtime rt;
    do{
        std::getline(ifs,buf,' ');
        // std::cout << "Processing "sv << buf << std::endl;
        cppp::str text{cppp::tou8(buf)};
        if(text.empty()) continue;
        switch(text.front()){
            case u8'#': {
                std::uint64_t v;
                if(!std::from_chars(buf.data()+1,buf.data()+buf.size(),v)){
                    std::cerr << "Error: Invalid number"sv << std::endl;
                    return 1;
                }
                rt.put(v);
                break;
            }
            case u8'~': {
                float v;
                if(!std::from_chars(buf.data()+1,buf.data()+buf.size(),v)){
                    std::cerr << "Error: Invalid number"sv << std::endl;
                    return 1;
                }
                rt.put(v);
                break;
            }
            case u8'\'': {
                cppp::str s;
                std::uint8_t escape = 0;
                std::uint8_t escape_digit;
                for(char8_t c : cppp::sv(text).substr(1uz)){
                    switch(escape){
                        case 0:
                            if(c==u8'%'){
                                escape = 1;
                            }else{
                                s.push_back(c);
                            }
                            break;
                        case 1:
                            if(c==u8'%'){
                                s.push_back(c);
                                escape = 0;
                            }else{
                                escape_digit = htoi(c);
                                ++escape;
                            }
                            break;
                        case 2:
                            s.push_back(escape_digit<<4|htoi(c));
                            escape = 0;
                            break;
                        default: std::unreachable();
                    }
                }
                rt.put(s);
                break;
            }
            case u8'^': {
                if(rt.top().has<mnd::function>()){
                    mnd::function_ref ref{std::move(rt.top().get<mnd::function>().fn)};
                    rt.pop();
                    rt.put(mnd::Value(cppp::emplace_tag<mnd::function_ref>,std::move(ref)));
                }else{
                    std::cerr << "Error: Invalid end of function call"sv << std::endl;
                    return 1;
                }
                break;
            }
            case u8'-': {
                rt.pop();
                break;
            }
            default:
                if(auto it=ftab.find(text);it!=ftab.end()){
                    rt.put(mnd::function(it->second));
                }else{
                    std::cerr << "Error: Invalid identifier"sv << std::endl;
                    return 1;
                }
                break;
        }
    }while(!ifs.eof());
    if(std::size_t ss=rt.ss()){
        std::cout << '\n' << ss << " item(s) remain on stack"sv << std::endl;
    }
    return 0;
}
