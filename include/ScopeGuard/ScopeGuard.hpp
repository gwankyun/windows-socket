#pragma once
#ifndef SCOPE_GUARD_HPP
#define SCOPE_GUARD_HPP
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 26812)
#endif // _MSC_VER

#ifndef SCOPE_GUARD_ARG
#ifdef __cpp_rvalue_references
#define SCOPE_GUARD_ARG(T) T&&
#else 
#define SCOPE_GUARD_ARG(T) T&
#endif // __cpp_rvalue_references
#endif // !SCOPE_GUARD_ARG

class ScopeGuard
{
public:

    enum Type { Delete = 0, DeleteArray };

    template<typename CB>
    explicit ScopeGuard(CB callback_)
        : dismissed(false), base(new CallBack0<CB>(callback_))
    {
    }

    template<typename CB, typename T>
    explicit ScopeGuard(CB callback_, SCOPE_GUARD_ARG(T) value_)
        : dismissed(false), base(new CallBack1<CB, T>(callback_, value_))
    {
    }

    template<typename CB, typename T1, typename T2>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_)
        : dismissed(false), base(new CallBack2<CB, T1, T2>(callback_, value1_, value2_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_)
        : dismissed(false), base(new CallBack3<CB, T1, T2, T3>(callback_, value1_, value2_, value3_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_)
        : dismissed(false), base(new CallBack4<CB, T1, T2, T3, T4>(callback_, value1_, value2_, value3_, value4_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_,
        SCOPE_GUARD_ARG(T5) value5_)
        : dismissed(false), base(new CallBack5<CB, T1, T2, T3, T4, T5>(callback_, value1_, value2_, value3_, value4_, value5_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_,
        SCOPE_GUARD_ARG(T5) value5_, SCOPE_GUARD_ARG(T6) value6_)
        : dismissed(false), base(new CallBack6<CB, T1, T2, T3, T4, T5, T6>(callback_, value1_, value2_, value3_, value4_, value5_, value6_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_,
        SCOPE_GUARD_ARG(T5) value5_, SCOPE_GUARD_ARG(T6) value6_,
        SCOPE_GUARD_ARG(T7) value7_)
        : dismissed(false), base(new CallBack7<CB, T1, T2, T3, T4, T5, T6, T7>(callback_, value1_, value2_, value3_, value4_, value5_, value6_, value7_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_,
        SCOPE_GUARD_ARG(T5) value5_, SCOPE_GUARD_ARG(T6) value6_,
        SCOPE_GUARD_ARG(T7) value7_, SCOPE_GUARD_ARG(T8) value8_)
        : dismissed(false), base(new CallBack8<CB, T1, T2, T3, T4, T5, T6, T7, T8>(callback_, value1_, value2_, value3_, value4_, value5_, value6_, value7_, value8_))
    {
    }

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    explicit ScopeGuard(CB callback_,
        SCOPE_GUARD_ARG(T1) value1_, SCOPE_GUARD_ARG(T2) value2_,
        SCOPE_GUARD_ARG(T3) value3_, SCOPE_GUARD_ARG(T4) value4_,
        SCOPE_GUARD_ARG(T5) value5_, SCOPE_GUARD_ARG(T6) value6_,
        SCOPE_GUARD_ARG(T7) value7_, SCOPE_GUARD_ARG(T8) value8_,
        SCOPE_GUARD_ARG(T9) value9_)
        : dismissed(false), base(new CallBack9<CB, T1, T2, T3, T4, T5, T6, T7, T8, T9>(callback_, value1_, value2_, value3_, value4_, value5_, value6_, value7_, value8_, value9_))
    {
    }

    template<typename T>
    explicit ScopeGuard(Type type, T*& value_) // C26812 reported here
        : dismissed(false), base(new Ptr<T>(type, value_))
    {
    }

    ~ScopeGuard()
    {
        if (!dismissed)
        {
            if (base != NULL)
            {
                delete base;
                base = NULL;
            }
        }
    }

    void dissmiss()
    {
        dismissed = true;
    }

private:

    ScopeGuard(const ScopeGuard&);
    ScopeGuard& operator=(const ScopeGuard&);

    struct Base
    {
        Base() {}
        virtual ~Base() {}
    };

    template<typename CB>
    struct CallBack0 : public Base
    {
        CallBack0(CB callback_) : callback(callback_)
        {
        }

        ~CallBack0()
        {
            callback();
        }

        CB callback;
    };

    template<typename CB, typename T>
    struct CallBack1 : public Base
    {
        CallBack1(CB callback_, T& value_)
            : value(value_), callback(callback_)
        {
        }

        ~CallBack1()
        {
            callback(value);
        }

        T& value;
        CB callback;
    };

    template<typename CB, typename T1, typename T2>
    struct CallBack2 : public Base
    {
        CallBack2(CB callback_, T1& value1_, T2& value2_)
            : callback(callback_), value1(value1_), value2(value2_)
        {
        }

        ~CallBack2()
        {
            callback(value1, value2);
        }

        CB callback;
        T1& value1;
        T2& value2;
    };

    template<typename CB, typename T1, typename T2, typename T3>
    struct CallBack3 : public Base
    {
        CallBack3(CB callback_, T1& value1_, T2& value2_, T3& value3_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_)
        {
        }

        ~CallBack3()
        {
            callback(value1, value2, value3);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4>
    struct CallBack4 : public Base
    {
        CallBack4(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_)
        {
        }

        ~CallBack4()
        {
            callback(value1, value2, value3, value4);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T3& value4;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5>
    struct CallBack5 : public Base
    {
        CallBack5(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_, T5& value5_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_), value5(value5_)
        {
        }

        ~CallBack5()
        {
            callback(value1, value2, value3, value4, value5);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T4& value4;
        T5& value5;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct CallBack6 : public Base
    {
        CallBack6(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_, T5& value5_, T6& value6_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_), value5(value5_), value6(value6_)
        {
        }

        ~CallBack6()
        {
            callback(value1, value2, value3, value4, value5, value6);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T4& value4;
        T5& value5;
        T6& value6;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct CallBack7 : public Base
    {
        CallBack7(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_, T5& value5_, T6& value6_, T7& value7_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_), value5(value5_), value6(value6_), value7(value7_)
        {
        }

        ~CallBack7()
        {
            callback(value1, value2, value3, value4, value5, value6, value7);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T4& value4;
        T5& value5;
        T6& value6;
        T7& value7;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct CallBack8 : public Base
    {
        CallBack8(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_, T5& value5_, T6& value6_, T7& value7_, T8& value8_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_), value5(value5_), value6(value6_), value7(value7_), value8(value8_)
        {
        }

        ~CallBack8()
        {
            callback(value1, value2, value3, value4, value5, value6, value7, value8);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T4& value4;
        T5& value5;
        T6& value6;
        T7& value7;
        T8& value8;
    };

    template<typename CB, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct CallBack9 : public Base
    {
        CallBack9(CB callback_, T1& value1_, T2& value2_, T3& value3_, T4& value4_, T5& value5_, T6& value6_, T7& value7_, T8& value8_, T9& value9_)
            : callback(callback_), value1(value1_), value2(value2_), value3(value3_), value4(value4_), value5(value5_), value6(value6_), value7(value7_), value8(value8_), value9(value9_)
        {
        }

        ~CallBack9()
        {
            callback(value1, value2, value3, value4, value5, value6, value7, value8, value9);
        }

        CB callback;
        T1& value1;
        T2& value2;
        T3& value3;
        T4& value4;
        T5& value5;
        T6& value6;
        T7& value7;
        T8& value8;
        T9& value9;
    };

    template<typename T>
    struct Ptr : public Base
    {
        Ptr(Type type_, T*& value_) : type(type_), value(value_)
        {
        }

        ~Ptr()
        {
            if (value != NULL)
            {
                switch (type)
                {
                case ScopeGuard::Delete:
                    delete value;
                    value = NULL;
                    break;
                case ScopeGuard::DeleteArray:
                    delete[] value;
                    value = NULL;
                    break;
                default:
                    break;
                }
            }
        }

        T*& value;
        Type type;
    };

    bool dismissed;
    Base* base;
};

#define SCOPE_GUARD_LINENAME_CAT(name, line) name##line
#define SCOPE_GUARD_LINENAME(name, line) SCOPE_GUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(...) ScopeGuard SCOPE_GUARD_LINENAME(ScopeGuard_, __LINE__)(##__VA_ARGS__)
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
#endif // !SCOPE_GUARD_HPP
