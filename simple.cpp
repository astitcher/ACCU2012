// Transcription into class style of c++

class Value {};

class IntValue : public Value {
  int value;
};

class FloatValue : public Value {
  double value;
};

enum Binop {
  Plus, Minus, Multiply, Divide, Power
};

enum UnOp {
  Negate, Sqr, Sqroot
};


