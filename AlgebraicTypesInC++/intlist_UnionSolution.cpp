#include <tuple>
#include <memory>

using std::tuple;
using std::tie;
using std::get;
using std::shared_ptr;
using std::make_shared;

#define SHARED_PTR

// Abstract away the pointer type we're using for our type
// so we can carefully examine the memory management later
namespace UnionSolution {
  struct intlist;
#ifdef SHARED_PTR
  typedef shared_ptr<intlist> intlist_ptr;
#else
  typedef intlist* intlist_ptr;
#endif
}

namespace UnionSolution {
  enum intlist_const {Cons, Nil};
  struct intlist {
    intlist_const type;
    // Don't really need a union here but would
    // with more than one constructor carrying data
    union u {
      tuple<int, intlist_ptr> consdata;
      u() {}; // Need this because of new unrestricted union rules
      ~u() {};
    } u;
    intlist(intlist_const t): type(t) {}
  };

  // Have one accessor function here for every datatype constructor
  tuple<int, intlist_ptr>& cons(const intlist_ptr& il) { return il->u.consdata; }

  // Have one constructor (factory) function here for each datatype constructor
#ifdef SHARED_PTR  
  intlist_ptr makeNil() {
    return make_shared<intlist>(Nil);
  }

  intlist_ptr makeCons(int i, const intlist_ptr& il) {
    auto v = make_shared<intlist>(Cons);
    cons(v) = tie(i, il);
    return v;
  }
#else
  intlist_ptr makeNil() {
    return new intlist(Nil);
  }

  intlist_ptr makeCons(int i, const intlist_ptr& il) {
    auto v = new intlist(Cons);
    cons(v) = tie(i, il);
    return v;
  }
#endif
  int length(const intlist_ptr& v) {
    switch (v->type) {
    case Nil:
      return 0;
    case Cons:
      return 1 + length(get<1>(cons(v)));
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }

// In this version of the length function we explicitly access the 2nd element
// of the tuple by position, which gets us what we want, but isn't like the template
// tuple in the ocaml match operation.

// To make the code more like that we can use std::tie to destructure the tuple.
// We'll do this from now on - unfortunately we can't (or I can't figure out a
// way to) declare the destructured types at the same time as splitting them out.
// Could be done with a preprocessor macro, but that might be worse.

// This actually turns out to be important later on when we consider using a shared_ptr
// rather than a raw pointer (as we can't use a reference and tie it and the means
// that it will be copied.

  int length1(const intlist_ptr& v) {
    switch (v->type) {
    case Nil:
      return 0;
    case Cons: {
      // Deconstruct the tuple
      int i; intlist_ptr il;
      tie(i,il) = cons(v);
      return 1 + length1(il);
    }
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }
}

// Note that the match construct in length is actually fairly simple
// and simply mirrors the ml version, although the mechanics of destructuring
// the tuple are a little fiddly std::tie really helps us here


