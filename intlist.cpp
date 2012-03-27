#include <tuple>
#include <typeinfo>
#include <stdexcept>

using std::tuple;
using std::tie;
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
    int i;
    intlist* il;
  public:
    Cons(int i0, intlist* il0): i(i0), il(il0) {}
    tuple<int, intlist*> data() {
      return tie(i, il);
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
    int i;
    intlist* il;
  public:
    static constexpr intlist_const code() {return intlist::Cons;}
    Cons(int i0, intlist* il0): i(i0), il(il0) {}
    tuple<int, intlist*> data() {
      return tie(i, il);
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
    int i;
    intlist* il;
    int length() const;
  public:
    Cons(int i0, intlist* il0): i(i0), il(il0) {}
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
    return 1 + il->length();
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

#include <iostream>

using std::cout;

using namespace ClassSolution1;

int main(){
  auto a = makeCons(12, makeCons(23, makeNil()));
  cout << length1(a) << "\n";
}
