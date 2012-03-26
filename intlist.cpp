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

namespace ClassSolution2 {
  enum intlist_const {Cons, Nil};
  class intlist {
  public:
    virtual ~intlist() {}
    virtual intlist_const type() = 0;
  };

  class NilC: public intlist {
    intlist_const type() {return Nil;}
  };

  class ConsC: public intlist {
    intlist_const type() {return Cons;}
    int i;
    intlist* il;
  public:
    ConsC(int i0, intlist* il0): i(i0), il(il0) {}
    tuple<int, intlist*> data() {
      return tie(i, il);
    }
  };

  intlist* makeNil() {
    return new NilC;
  }

  intlist* makeCons(int i, intlist* il) {
    return new ConsC(i, il);
  }

  int length(intlist* v) {
    switch (v->type()) {
    case Nil:
      return 0;
    case Cons:
      int i; intlist* il;
      tie(i, il) = static_cast<ConsC*>(v)->data();
      return 1 + length(il);
    default:
      throw logic_error("intlist: not all cases covered");
    }
  }
}

#include <iostream>

using std::cout;

using namespace ClassSolution2;

int main(){
  auto a = makeCons(12, makeCons(23, makeNil()));
  cout << length(a) << "\n";
}
