#include <tuple>
#include <typeinfo>
#include <stdexcept>

using std::tuple;
using std::tie;
using std::get;
using std::logic_error;

namespace UnionSolution {
  enum intlist_const {Cons, Nil};
  struct intlist {
    intlist_const type;
    // Don't really need a union here but would
    // with more than one constructor carrying data
    union u {
      tuple <int, intlist*> consdata;
      u() {}; // Need this because of new unrestricted union rules
    } u;
    intlist(intlist_const t): type(t) {}
  };

  intlist* makeNil() {
    return new intlist(Nil);
  }

  intlist* makeCons(int i, intlist* il) {
    intlist* v = new intlist(Cons);
    v->u.consdata = tie(i, il);
    return v;
  }

  int length(intlist* v) {
    switch (v->type) {
    case Nil:
      return 0;
    case Cons:
      // Deconstruct the tuple
      int i; intlist* il; tie(i,il) = v->u.consdata;
      return 1 + length(il);
      // or could just do:
      // return 1 + get<1>(v->u.consdata);
    }
  }
}
// Note that the match construct in length is actually fairly simple
// and simply mirrors the ml version, although the mechanics of destructuring
// the tuple are a little fiddly std::tie really helps us here

namespace ClassSolution1 {
  class intlist {
  public:
    virtual ~intlist() {}
  };

  class Nil: public intlist {
  };

  class Cons: public intlist {
    tuple<int,intlist*> v;
  public:
    Cons(int i0, intlist* il0): v(i0, il0) {}
    tuple<int, intlist*> data() const {
      return v;
    }
  };

  intlist* makeNil() {
    return new Nil;
  }

  intlist* makeCons(int i, intlist* il) {
    return new Cons(i, il);
  }

  int length(intlist* v) {
    if (typeid(*v) == typeid(Nil)) {
      return 0;
    } else if (typeid(*v) == typeid(Cons)) {
      int i; intlist* il;
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
  int length1(intlist* v) {
    if (dynamic_cast<Nil*>(v)) {
      return 0;
    } else if (auto cons = dynamic_cast<Cons*>(v)) {
      int i; intlist* il;
      tie(i, il) = cons->data();
      return 1 + length1(il);
    } else throw logic_error("intlist: not all cases covered");
  }
}

// Notice that std::tie again makes the tuple destructuring nice and simple

// We can improve this by inventing our own little type info...
namespace ClassSolution2 {
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
    tuple<int, intlist*> v;
  public:
    static constexpr intlist_const code() {return intlist::Cons;}
    Cons(int i0, intlist* il0): v(i0, il0) {}
    tuple<int, intlist*> data() const {
      return v;
    }
  };

  intlist* makeNil() {
    return new Nil;
  }

  intlist* makeCons(int i, intlist* il) {
    return new Cons(i, il);
  }

  int length(intlist* v) {
    switch (v->type()) {
    case Nil::code():
      return 0;
    case Cons::code():
      int i; intlist* il;
      tie(i, il) = static_cast<Cons*>(v)->data();
      return 1 + length(il);
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }
}

// Note we've had to complicate the base and other classes a bit to 
// get the type code
// However now notice that we had to get the enum values a little
// differently - they can't share the same name as the actual classes
// Still the look of the length function is back to much closer to the
// ml original.

// It would be nice to do something about the ugly static_cast.

// Of course if you've heard much C++ advice you're thinking "switching
// on class types has a really bad smell! we should really do something
// polymorphic - that's real OO, what C++ is all about!"

// Well apart from the dubious conclusion straight out of the 80s, there
// may be something in this - lets take a look...
namespace ClassSolution3 {
  class intlist {
  public:
    virtual ~intlist() {}
    virtual int length() const = 0;
  };

  class Nil: public intlist {
    int length() const;
  };

  class Cons: public intlist {
    tuple<int, intlist*> v;
    int length() const;
  public:
    Cons(int i0, intlist* il0): v(i0, il0) {}
  };

  intlist* makeNil() {
    return new Nil;
  }

  intlist* makeCons(int i, intlist* il) {
    return new Cons(i, il);
  }

  int length(intlist* v) {
    return v->length();
  }
  int Nil::length() const {
    return 0;
  }
  int Cons::length() const {
    return 1 + get<1>(v)->length();
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

// What about using a visitor pattern instead of having to add to the class
// hierarchy all the time?

// Big question is how to return a value from the visitors and what type
// should it be - use a template parameter
namespace VisitorClassSolution {
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
    tuple<int,intlist*> v;
    int apply(const intlistVisitor<int>& v) {
      return v(*this);
    }
  public:
    Cons(int i0, intlist* il0): v(i0, il0) {}
    tuple<int, intlist*> data() const {
      return v;
    }
  };

  intlist* makeNil() {
    return new Nil;
  }

  intlist* makeCons(int i, intlist* il) {
    return new Cons(i, il);
  }

  class intlistlength: public intlistVisitor<int> {
    int operator() (const Nil&) const {
      return 0;
    }
    int operator() (const Cons& c) const {
      int i; intlist* il;
      tie(i, il) = c.data();
      return 1 + il->apply(*this); // This is what recursion looks like!
    }
  };

  int length(intlist* v) {
    return v->apply(intlistlength());
  }
}

// As I said earlier, this code is likely to leak badly, so lets address that
// [Probably better to get this right ab initio actually]

// Here we encounter a problem - what kind of pointers do we need? Are the cons
// cells in the list shared with anyone else? Well they aren't in our trivial
// example, but in "the real world" our lists will be accessible and so we need
// to assume they are shared - this is where garbage collection comes into its
// own - you don't need to worry about the ownership patterns.

// So lets just use shared_ptr<>! (unique_ptr<> is fine too in some other
// application)

#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace ClassSolution4 {
  class intlist {
  public:
    virtual ~intlist() {}
    virtual int length() const = 0;
  };
  typedef shared_ptr<intlist> intlist_ptr;

  class Nil: public intlist {
    int length() const;
  };

  class Cons: public intlist {
    int i;
    intlist_ptr il;
    int length() const;
  public:
    Cons(int i0, const intlist_ptr& il0): i(i0), il(il0) {}
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
    return 1 + il->length();
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

using namespace VariantSolution;

int main(){
  auto a = makeCons(12, makeCons(23, makeNil()));
  cout << length1(a) << "\n";
}
