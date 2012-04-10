#include <tuple>

using std::tuple;
using std::tie;
using std::get;

// Abstract away the pointer type we're using for our type
// so we can carefully examine the memory management later
namespace UnionSolution {
  struct intlist;
  typedef intlist* intlist_ptr;
}

namespace UnionSolution {
  enum intlist_const {Cons, Nil};
  struct intlist {
    intlist_const type;
    // Don't really need a union here but would
    // with more than one constructor carrying data
    union u {
      tuple <int, intlist_ptr> consdata;
      u() {}; // Need this because of new unrestricted union rules
    } u;
    intlist(intlist_const t): type(t) {}
  };

  intlist* makeNil() {
    return new intlist(Nil);
  }

  intlist* makeCons(int i, intlist_ptr il) {
    intlist_ptr v = new intlist(Cons);
    v->u.consdata = tie(i, il);
    return v;
  }

  int length(intlist_ptr v) {
    switch (v->type) {
    case Nil:
      return 0;
    case Cons:
      return 1 + length(get<1>(v->u.consdata));
    }
  }

// In this version of the length function we explicitly access the 2nd element
// of the tuple by position, which gets us what we want, but isn't like the template
// tuple in the ocaml match operation.

// To make the code more like that we can use std::tie to destructure the tuple.
// We'll do this from now on - unfortunately we can't (or I can't figure out a
// way to) declare the destructured types at the same time as splitting them out.
// Could be done with a preprocessor macro, but that might be worse.

  int length1(intlist_ptr v) {
    switch (v->type) {
    case Nil:
      return 0;
    case Cons:
      // Deconstruct the tuple
      int i; intlist_ptr il;
      tie(i,il) = v->u.consdata;
      return 1 + length1(il);
    }
  }
}

// Note that the match construct in length is actually fairly simple
// and simply mirrors the ml version, although the mechanics of destructuring
// the tuple are a little fiddly std::tie really helps us here


