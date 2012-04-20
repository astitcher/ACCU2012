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

  // Visitor base template
  template <typename R, typename T>
  class VisitorBase {
  public:
    virtual R operator()(const T&) const = 0;
  };

  // Empty combining class
  template <typename R, typename... Ts>
  class Visitor : public VisitorBase<R, Ts>... {
  public:
    typedef R ReturnType;
  };
  
  // Cast Visitor to correct base so that you can
  // call unambiguous visit function - without this
  // you can't call because they are defined in multiple
  // baseclasses which fails the name lookup phase
  // (even before you get to the overload resolution)qq
  template <typename R, typename T, typename... Ts>
  const VisitorBase<R,T>& visitor_cast(const Visitor<R,Ts...>& r) {
    return r;
  }
  
  // General Base class for algebraic datatypes
  // the root type inherits from this class
  // supplying the discriminator type and the
  // Visitor base class
  template <typename D, typename V>
  class ADatatypeBase {
  public:
    typedef D Discriminator;
    typedef V ADVisitor;

    virtual ~ADatatypeBase() {}
    virtual Discriminator type() const = 0;
    virtual typename ADVisitor::ReturnType apply(const ADVisitor&) = 0;
  };

  // The individual data type constructors inherit from here
  // with supplying the root datatype class, a discriminator value
  // the datat type itself (CRTP) and the type of the contained data
  template <typename T, typename T::Discriminator C, typename M, typename D>
  class ADatatype: public T {
    const D v;

    typename T::Discriminator type() const {return C;}
    typename T::ADVisitor::ReturnType apply(const typename T::ADVisitor& v) {
      return visitor_cast<typename T::ADVisitor::ReturnType,M>(v)(static_cast<M&>(*this));
    }
  public:
    template<typename... Args>
    explicit ADatatype(Args... a) :
      v(a...)
    {}
    const D& data() const {
      return v;
    }
    static constexpr typename T::Discriminator code() {return C;}
  };

  template <typename T, typename T::Discriminator C, typename M>
  class ADatatype<T,C,M,void>: public T {
    typename T::Discriminator type() const {return C;}
    typename T::ADVisitor::ReturnType apply(const typename T::ADVisitor& v) {
      return visitor_cast<typename T::ADVisitor::ReturnType,M>(v)(static_cast<M&>(*this));
    }
  public:
    static constexpr typename T::Discriminator code() {return C;}
  };

namespace AllClassSolution {
  class intlist;
#ifdef SHARED_PTR
  typedef shared_ptr<intlist> intlist_ptr;
  inline intlist* get(const intlist_ptr& il) {
    return il.get();
  }
#else
  typedef intlist* intlist_ptr;
  inline intlist* get(const intlist_ptr& il) {
    return il;
  }
#endif

  class Nil;
  class Cons;
  
  enum class types {Cons, Nil};

  class intlist : public ADatatypeBase<types, Visitor<int,Nil,Cons>> {
  public:
    virtual int length() const = 0;
  };

  class Nil: public ADatatype<intlist, types::Nil, Nil, void> {
    int length() const;
  };

  class Cons: public ADatatype <intlist, types::Cons, Cons, tuple<int, intlist_ptr>> {
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

  // Did try to speed this up by creating these constant
  // versions of the typeids that are constant but it makes
  // no difference. The compiler is already smart enough to
  // know that they are constant:
  // const std::type_info& NilTypeInfo(typeid(Nil));
  // const std::type_info& ConsTypeInfo(typeid(Cons));
  // For gcc at least typeid comparison is slow because it actually
  // has to compare the actual mangled type name which can be large.
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

  class intlistlength: public Visitor<int,Nil,Cons> {
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
