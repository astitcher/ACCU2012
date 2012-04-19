#include <tuple>
#include <typeinfo>
#include <stdexcept>
#include <memory>

using std::shared_ptr;
using std::make_shared;
using std::tuple;
using std::tie;
using std::get;
using std::logic_error;

#define SHARED_PTR

// General template to make algebraic datatype declarations easier

  template <typename T, typename V>
  class ADatatype_base {
  public:
    typedef T types;
    typedef V Visitor;

    virtual ~ADatatype_base() {}
    virtual types type() const = 0;
    virtual int apply(const Visitor&) = 0;
  };

  template <typename T, typename T::types C, typename D>
  class ADatatype: public T {
    const D v;

    typename T::types type() const {return C;}
  public:
    template<typename... Args>
    ADatatype(Args... a) :
      v(a...)
    {}
    const D& data() const {
      return v;
    }
    static constexpr typename T::types code() {return C;}
  };

  template <typename T, typename T::types C>
  class ADatatype<T,C,void>: public T {
    typename T::types type() const {return C;}
  public:
    static constexpr typename T::types code() {return C;}
  };

  // This approach to providing multiple virtual functions
  // with the different signature fails to work because function
  // lookup only looks for the first place with a member of the
  // name we're after. If that scope doesn't have a matching
  // member function then it will just fail.
  template <typename R, typename... Ts>
  class AVisitor;

  template <typename R, typename T>
  class AVisitor<R,T> {
  public:
    virtual R operator()(const T&) const = 0;
  };

  template <typename R, typename T, typename... Ts>
  class AVisitor<R,T,Ts...> : public AVisitor<R, Ts...> {
  public:
    virtual R operator()(const T&) const = 0;
  };
  
namespace AllClassSolution {
  class intlist;
#ifdef SHARED_PTR
  typedef shared_ptr<intlist> intlist_ptr;
  inline intlist* get(intlist_ptr il) {
    return il.get();
  }
#else
  typedef intlist* intlist_ptr;
  inline intlist* get(intlist_ptr il) {
    return il;
  }
#endif

  class Nil;
  class Cons;
  
  template <typename T>
  class intlistVisitor {
  public:
    virtual T operator()(const Nil&) const = 0;
    virtual T operator()(const Cons&) const = 0;
  };

  enum class types {Cons, Nil};

  class intlist : public ADatatype_base<types, intlistVisitor<int>> {
  public:
    virtual int length() const = 0;
  };

  class Nil: public ADatatype<intlist, types::Nil, void> {
    int apply(const Visitor& v) {
      return v(*this);
    }

    int length() const;
  };

  class Cons: public ADatatype <intlist, types::Cons, tuple<int, intlist_ptr>> {
    int apply(const Visitor& v) {
      return v(*this);
    }

    int length() const;

  public:
    Cons(int i0, const intlist_ptr& il0):
      ADatatype(i0, il0)
    {}
  };

  const tuple<int, intlist_ptr>& cons(const intlist_ptr& il) { return static_cast<Cons*>(get(il))->data(); }

#ifdef SHARED_PTR
  intlist_ptr makeNil() {
    return make_shared<Nil>();
  }

  intlist_ptr makeCons(int i, const intlist_ptr& il) {
    return make_shared<Cons>(i, il);
  }
#else
  intlist_ptr makeNil() {
    return new Nil();
  }

  intlist_ptr makeCons(int i, const intlist_ptr& il) {
    return new Cons(i, il);
  }
#endif
  int length(const intlist_ptr& v) {
    return v->length();
  }

  int Nil::length() const {
    return 0;
  }
  int Cons::length() const {
    //int i; intlist_ptr il;
    //tie(i, il) = data();
    auto& il = std::get<1>(data());
    return 1 + il->length();
  }

  int length1(const intlist_ptr& v) {
    if (typeid(*get(v)) == typeid(Nil)) {
      return 0;
    } else if (typeid(*get(v)) == typeid(Cons)) {
      //int i; intlist_ptr il;
      //tie(i, il) = cons(v);
      auto& il = std::get<1>(cons(v));
      return 1 + length1(il);
    } else throw logic_error("intlist: not all cases covered");
  }

  int length2(const intlist_ptr& v) {
    if (dynamic_cast<Nil*>(get(v))) {
      return 0;
    } else if (auto cons = dynamic_cast<Cons*>(get(v))) {
      //int i; intlist_ptr il;
      //tie(i, il) = cons->data();
      auto& il = std::get<1>(cons->data());
      return 1 + length2(il);
    } else throw logic_error("intlist: not all cases covered");
  }

  int length3(const intlist_ptr& v) {
    switch (v->type()) {
    case Nil::code():
      return 0;
    case Cons::code(): {
      //int i; intlist_ptr il;
      //tie(i, il) = cons(v);
      auto& il = std::get<1>(cons(v));
      return 1 + length3(il);
    }
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }

  class intlistlength: public intlistVisitor<int> {
    int operator() (const Nil&) const {
      return 0;
    }
    int operator() (const Cons& c) const {
      //int i; intlist_ptr il;
      //tie(i, il) = c.data();
      auto& il = std::get<1>(c.data());
      return 1 + il->apply(*this); // This is what recursion looks like!
    }
  };

  int length4(const intlist_ptr& v) {
    return v->apply(intlistlength());
  }
}

#include <iostream>
#include <functional>
#include <chrono>

using std::cout;
using std::function;

void run_loop(const char* label, int count, const function<void()>& f) {
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i=0; i<count; ++i) f();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << label << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " us\n";
}

int main(){
  {
    using namespace AllClassSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    auto c = 1000000;
    run_loop("Template length  ", c,[&a]{(void) length(a);});
    run_loop("Template length1 ", c,[&a]{(void) length1(a);});
    run_loop("Template length2 ", c,[&a]{(void) length2(a);});
    run_loop("Template length3 ", c,[&a]{(void) length3(a);});
    run_loop("Template length4 ", c,[&a]{(void) length4(a);});
  }
}
