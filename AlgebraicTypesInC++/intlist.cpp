#include <tuple>
#include <typeinfo>
#include <stdexcept>

using std::tuple;
using std::tie;
using std::get;
using std::logic_error;

#include "intlist_UnionSolution.cpp"


namespace ClassSolution1 {
  class intlist;
  typedef intlist* intlist_ptr;

  class intlist {
  public:
    virtual ~intlist() {}
  };

  class Nil: public intlist {
  };

  class Cons: public intlist {
    tuple<int,intlist_ptr> v;
  public:
    Cons(int i0, intlist_ptr il0): v(i0, il0) {}
    tuple<int, intlist_ptr> data() const {
      return v;
    }
  };

  intlist_ptr makeNil() {
    return new Nil;
  }

  intlist_ptr makeCons(int i, intlist_ptr il) {
    return new Cons(i, il);
  }

  int length(intlist_ptr v) {
    if (typeid(*v) == typeid(Nil)) {
      return 0;
    } else if (typeid(*v) == typeid(Cons)) {
      int i; intlist_ptr il;
      tie(i, il) = static_cast<Cons*>(v)->data();
      return 1 + length(il);
    } else throw logic_error("intlist: not all cases covered");
  }
}
// Note that the data type definition itself seems a lot nicer here -
// it's still way more verbose than the ml version, but we'll probably
// have to live with that in any solution.

// Note that class solution 1 has a really horrible looking length function
// that uses chained typeid comparisons to perform the matching. This could
// also be done by using dynamic cast, which would also solve the static cast
// issue - trouble is dynamic_cast may not be very fast - but lets save
// such considerations until we measure them!

// So you could have..
namespace ClassSolution1 {
  int length1(intlist_ptr v) {
    if (dynamic_cast<Nil*>(v)) {
      return 0;
    } else if (auto cons = dynamic_cast<Cons*>(v)) {
      int i; intlist_ptr il;
      tie(i, il) = cons->data();
      return 1 + length1(il);
    } else throw logic_error("intlist: not all cases covered");
  }
}

// Notice that std::tie again makes the tuple destructuring nice and simple

// We can improve this by inventing our own little type info...
namespace ClassSolution2 {
  class intlist;
  typedef intlist* intlist_ptr;

  class intlist {
  public:
    virtual ~intlist() {}
    enum intlist_const {Cons, Nil};
    virtual intlist_const type() const = 0;
  };

  class Nil: public intlist {
    intlist_const type() const {return code();}
  public:
    static constexpr intlist_const code() {return intlist::Nil;}
  };

  class Cons: public intlist {
    intlist_const type() const {return code();}
    tuple<int, intlist_ptr> v;
  public:
    static constexpr intlist_const code() {return intlist::Cons;}
    Cons(int i0, intlist_ptr il0): v(i0, il0) {}
    tuple<int, intlist_ptr> data() const {
      return v;
    }
  };

  intlist_ptr makeNil() {
    return new Nil;
  }

  intlist_ptr makeCons(int i, intlist* il) {
    return new Cons(i, il);
  }

  int length(intlist_ptr v) {
    switch (v->type()) {
    case Nil::code():
      return 0;
    case Cons::code():
      int i; intlist_ptr il;
      tie(i, il) = static_cast<Cons*>(v)->data();
      return 1 + length(il);
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }
}

// Note we've had to complicate the base and other classes a bit to 
// get the type code - we've used constexpr to avoid any runtime overhead.
// However now notice that we had to get the enum values a little
// differently - they can't share the same name as the actual classes
// Still the look of the length function is back to much closer to the
// ml original.

// Note that the explicit enum in the intlist class doesn't limit the
// things you could do with the data structure as algebraic types are
// closed so enumerating all the possible sub-types is no problem.

// It would be nice to do something about the ugly static_cast.

// Of course if you've heard much C++ advice you're thinking "switching
// on class types has a really bad smell! we should really do something
// polymorphic - that's real OO, what C++ is all about!"

// Well apart from the dubious conclusion straight out of the 80s, there
// may be something in this - lets take a look...
namespace ClassSolution3 {
  class intlist;
  typedef intlist* intlist_ptr;

  class intlist {
  public:
    virtual ~intlist() {}
    virtual int length() const = 0;
  };

  class Nil: public intlist {
    int length() const;
  };

  class Cons: public intlist {
    tuple<int, intlist_ptr> v;
    int length() const;
  public:
    Cons(int i0, intlist_ptr il0): v(i0, il0) {}
    tuple<int, intlist_ptr> data() const {
      return v;
    }
  };

  intlist_ptr makeNil() {
    return new Nil;
  }

  intlist_ptr makeCons(int i, intlist_ptr il) {
    return new Cons(i, il);
  }

  int length(intlist_ptr v) {
    return v->length();
  }
  int Nil::length() const {
    return 0;
  }
  int Cons::length() const {
    return 1 + get<1>(v)->length();
    // Can't use length(get<1>(v)) because 
    // name resolution picks out the class member
    // not the free function, which makes it look
    // different from the original recursive intent.
  }
}
// Well this does make the length function nice and brief actually about
// as short as the ml version. However it has a serious downside - it's
// actually not a single function at all! We've now split the functionality
// of our previous single function, sort of smeared it out over all the
// classes we're using to implement our rather simple list.

// What this means is this that following this approach for the many
// functions you might expect defined on a list type means adding
// a function to every class for every function we need.

// Not necessarily so bad in itself, but it might be a maintenance issue
// as we have to add new functions in every class, so the class has to
// change. So no separate compilation, can't add funcations to classes
// without the source code (in other words someone else's classes).

// In sum though this is the simplest and most easily understood version
// of the code.

// Another important thing to note about this solution and the first class
// based solution the class definitions are identical. In fact all the code
// except the length function itself is identical. This tells us that these
// approaches are not mutually exclusive, and it is possible that functions
// may be implemented as virtual functions, and others might be implemented
// using switch type logic - this may help us with our maintenance.

// Another approach often used to extend operations on a class hierarchy
// without having to add extra member functions to each member of the hierarchy
// is to use the visito pattern, how would this look?

// Big question is how to return a value from the visitors and what type
// should it be - use a template parameter
namespace VisitorClassSolution {
  class intlist;
  typedef intlist* intlist_ptr;

  class Nil;
  class Cons;

  template <typename T>
  class intlistVisitor {
  public:
    virtual T operator()(const Nil&) const = 0;
    virtual T operator()(const Cons&) const = 0;
  };

  class intlist {
  public:
    virtual ~intlist() {}
    virtual int apply(const intlistVisitor<int>&) = 0;
  };

  class Nil: public intlist {
    int apply(const intlistVisitor<int>& v) {
      return v(*this);
    }
  };

  class Cons: public intlist {
    tuple<int,intlist_ptr> v;
    int apply(const intlistVisitor<int>& v) {
      return v(*this);
    }
  public:
    Cons(int i0, intlist_ptr il0): v(i0, il0) {}
    tuple<int, intlist_ptr> data() const {
      return v;
    }
  };

  intlist_ptr makeNil() {
    return new Nil;
  }

  intlist_ptr makeCons(int i, intlist_ptr il) {
    return new Cons(i, il);
  }

  class intlistlength: public intlistVisitor<int> {
    int operator() (const Nil&) const {
      return 0;
    }
    int operator() (const Cons& c) const {
      int i; intlist_ptr il;
      tie(i, il) = c.data();
      return 1 + il->apply(*this); // This is what recursion looks like!
    }
  };

  int length(intlist_ptr v) {
    return v->apply(intlistlength());
  }
}

// As with our previous class based solutions the code is very similar especially
// the structure definitions. The visitor code is the previous code with apply
// member functions added.

// In my opinion, the length function when expressed in this visitor form starts
// to drift further away from looking like a single length function than I like.
// It's good that the function can go into a single visitor class, which gives
// it more unity.

// The other issue is the machinery we need to make it work. In order to produce
// a function returning a specific type we need a virtual apply() returning that
// type. Since virtual functions can't be templated member functions we would be
// limited in the types we can return from functions constructed with visitors.
// [Have I missed something? boost::variant seems to get around this somehow]

// Up to this point I've skirted over the details of the type of pointer we should
// use. If we use plain pointers (as I have done in the examples up till now) we will
// end up leaking memory. So we should use a smart pointer to avoid having to worry
// about it ourselves.

// Here we encounter a problem - what kind of pointers do we need? Are the cons
// cells in the list shared with anyone else? Well they aren't in our trivial
// example, but in "the real world" our lists will be accessible and so we need
// to assume they are shared - this is where garbage collection comes into its
// own - you don't need to worry about the ownership patterns.

// So lets just use shared_ptr<>! (unique_ptr<> is fine too in some other
// applications where the cons cells will never be shared)

// Whilst we're at it we can also collect here all of the class based possibilities
// and we can see that they can in fact all be used together, although it probably
// makes no sense to combine them all like this.

#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace AllClassSolution {
  class intlist;
  typedef shared_ptr<intlist> intlist_ptr;

  class Nil;
  class Cons;

  template <typename T>
  class intlistVisitor {
  public:
    virtual T operator()(const Nil&) const = 0;
    virtual T operator()(const Cons&) const = 0;
  };

  class intlist {
  public:
    virtual ~intlist() {}
    enum intlist_const {Cons, Nil};
    virtual intlist_const type() const = 0;
    virtual int apply(const intlistVisitor<int>&) = 0;

    virtual int length() const = 0;
  };

  class Nil: public intlist {
    intlist_const type() const {return code();}
    int apply(const intlistVisitor<int>& v) {
      return v(*this);
    }

    int length() const;

  public:
    static constexpr intlist_const code() {return intlist::Nil;}
  };

  class Cons: public intlist {
    intlist_const type() const {return code();}
    int apply(const intlistVisitor<int>& v) {
      return v(*this);
    }

    tuple<int,intlist_ptr> v;
    int length() const;

  public:
    static constexpr intlist_const code() {return intlist::Cons;}

    Cons(int i0, const intlist_ptr& il0): v(i0, il0) {}
    tuple<int, intlist_ptr> data() const {
      return v;
    }
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
    return 1 + get<1>(v)->length();
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

  class intlistlength: public intlistVisitor<int> {
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

// Now to some of you all of this might be suggestive of a different approach
// using boost::Variant to model the algebraic datatypes.

// How does this look?
// For a start it doesn't get rid of the classes per individual datatype
// constructor:
#include <boost/variant.hpp>
namespace VariantSolution {
  class Cons;
  class Nil;

  typedef boost::variant<Cons, Nil> intlist;

  class Nil {
  };

  class Cons {
    tuple<int, intlist*> v;
  public:
    Cons(int i0, intlist* il0): v(i0, il0) {}
    tuple<int, intlist*> data() const {
      return v;
    }
  };

  intlist* makeNil() {
    return new intlist(Nil());
  }

  intlist* makeCons(int i, intlist* il) {
    return new intlist(Cons(i, il));
  }

  int length(intlist* v) {
    if (auto n = boost::get<Nil>(v)) {
      return 0;
    } else if (auto cons = boost::get<Cons>(v)) {
      int i; intlist* il;
      tie(i, il) = cons->data();
      return 1 + length(il);
    } else throw logic_error("intlist: not all cases covered");
  }
}

// Note that the structure of this is not very different from the original
// class based solutions.

// Boost::Variant does have one very good idea, which is to use visitors
// solving the problem of extending the classes for every new operation.
// But I bet you lot already thought of that being the OO hotshots you are!
// [Note need to rearrange material sequence here introduced visitor above]
namespace VariantSolution {
  class lengthVisitor: public boost::static_visitor<int> {
  public:
    int operator()(const Nil&) const {
      return 0;
    }
    int operator()(const Cons& c) const {
      int i; intlist* il;
      tie(i, il) = c.data();
      return 1 + boost::apply_visitor(*this, *il);
    }
  };
    
  int length1(intlist* v) {
    return boost::apply_visitor(lengthVisitor(), *v);
  }
}

// Note that there are some limitations of the class based with
// virtual function approach and the visitor approach - It only allows you to match a particular
// type constructor once using the pure virtual function approach.
// If the match logic is more complex then you might need extra conditional
// logic in the virtual function.
// However on the positive side
// the compiler ensures that you have covered all the cases, because it
// won't allow you to instantiate the class that represents that constructor
// without the relevant virtual function needed for the match case.

#include <iostream>

using std::cout;


int main(){
  {
    using namespace UnionSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
    cout << length1(a) << "\n";
  }
  {
    using namespace ClassSolution1;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
    cout << length1(a) << "\n";
  }
  {
    using namespace ClassSolution2;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
  }
  {
    using namespace ClassSolution3;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
  }
  {
    using namespace VisitorClassSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
  }
  {
    using namespace AllClassSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
    cout << length1(a) << "\n";
    cout << length2(a) << "\n";
    cout << length3(a) << "\n";
    cout << length4(a) << "\n";
  }
  {
    using namespace VariantSolution;
    auto a = makeCons(12, makeCons(23, makeNil()));
    cout << length(a) << "\n";
    cout << length1(a) << "\n";
  }
}