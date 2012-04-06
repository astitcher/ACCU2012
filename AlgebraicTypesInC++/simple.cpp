// Transcription into class style of c++
// C++11 features reduced to work with gcc 4.6
// compile with g++ -std=c++0x

#include <functional>
#include <string>
#include <tuple>
#include <memory>

using std::function;
using std::string;
using std::tuple;
using std::make_tuple;
using std::unique_ptr;

// Missed out of the standard
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique( Args&& ...args )
{
  return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
}

class Value {
public:
  virtual float asfloat() const = 0;
};

class Int : public Value {
  int value;
  float asfloat() const /*override*/ {return value;}
public:
  Int(int v) : value(v) {};
};

class Float : public Value {
  double value;
  float asfloat() const /*override*/ {return value;}
public:
  Float(float v) : value(v) {};
};

auto asfloat = [](const Value& v) {
  return v.asfloat();
};


typedef
tuple<string, int> binop_info;

class Binop {
public:
  virtual binop_info lookup_binop() const = 0;
  virtual unique_ptr<Value> eval(const Value&, const Value&) const = 0;
};

class Plus: public Binop {
  binop_info lookup_binop() const {
    return make_tuple("+", 0);
  }
  /*
  unique_ptr<Value> eval(const Value&, const Value&) const {
  }
  */
};

class Minus: public Binop {
  binop_info lookup_binop() const {
    return make_tuple("-", 0);
  }
};

class Multiply: public Binop {
  binop_info lookup_binop() const {
    return make_tuple("*", 2);
  }
};

class Divide: public Binop {
  binop_info lookup_binop() const {
    return make_tuple("/", 2);
  }
};

class Power: public Binop {
  binop_info lookup_binop() const {
    return make_tuple("**", 4);
  }
};

auto lookup_binop = [](const Binop& b) {
  return b.lookup_binop();
};

// 
enum UnOp {
  Negate, Sqr, Sqroot
};


#include <iostream>

using std::cout;
int main() {
  cout << std::showpoint;
  cout << asfloat(Int(3)) << "\n";
  cout << asfloat(Float(17.d)) << "\n";
};
