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
    D data() const {
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
  typedef shared_ptr<intlist> intlist_ptr;

  class Nil;
  class Cons;

  enum class types {Cons, Nil};

  class intlist : public ADatatype_base<types, AVisitor<int, Nil, Cons>> {
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

  intlist_ptr makeNil() {
    return make_shared<Nil>();
  }

  intlist_ptr makeCons(int i, intlist_ptr il) {
    return make_shared<Cons>(i, il);
  }

  int length(const intlist_ptr v) {
    return v->length();
  }

  int Nil::length() const {
    return 0;
  }
  int Cons::length() const {
    int i; intlist_ptr il;
    tie(i, il) = data();
    return 1 + il->length();
    // return 1 + get<1>(data())->length();
  }

  int length1(intlist_ptr v) {
    if (typeid(*v.get()) == typeid(Nil)) {
      return 0;
    } else if (typeid(*v.get()) == typeid(Cons)) {
      int i; intlist_ptr il;
      tie(i, il) = static_cast<Cons*>(v.get())->data();
      return 1 + length1(il);
    } else throw logic_error("intlist: not all cases covered");
  }

  int length2(intlist_ptr v) {
    if (dynamic_cast<Nil*>(v.get())) {
      return 0;
    } else if (auto cons = dynamic_cast<Cons*>(v.get())) {
      int i; intlist_ptr il;
      tie(i, il) = cons->data();
      return 1 + length2(il);
    } else throw logic_error("intlist: not all cases covered");
  }

  int length3(intlist_ptr v) {
    switch (v->type()) {
    case Nil::code():
      return 0;
    case Cons::code(): {
      int i; intlist_ptr il;
      tie(i, il) = static_cast<Cons*>(v.get())->data();
      return 1 + length3(il);
    }
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }

  class intlistlength: public AVisitor<int,Nil,Cons> {
    int operator() (const Nil&) const {
      return 0;
    }
    int operator() (const Cons& c) const {
      int i; intlist_ptr il;
      tie(i, il) = c.data();
      return 1 + il->apply(*this); // This is what recursion looks like!
    }
  };

  int length4(intlist_ptr v) {
    return v->apply(intlistlength());
  }
}

#include <iostream>
#include <functional>

using std::cout;
using std::function;

void run_loop(int count, const function<void()>& f) {
    for (int i=0; i<count; ++i) f();
}

int main(){
  {
    using namespace AllClassSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    run_loop(10000,[&a]{auto l = length(a);});
    run_loop(10000,[&a]{auto l = length1(a);});
    run_loop(10000,[&a]{auto l = length2(a);});
    run_loop(10000,[&a]{auto l = length3(a);});
    run_loop(10000,[&a]{auto l = length4(a);});
  }
}