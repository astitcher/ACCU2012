type value =
    Int of int
  | Float of float

type binop =
    Plus
  | Minus
  | Multiply
  | Divide
  | Power

type unop =
    Negate
  | Square
  | Sqroot

type expression =
    BinOp of binop * expression * expression
  | UnOp of unop * expression
  | Value of value

let rec eval e =
  let asfloat v =
    match v with
        Int i -> (float i)
      | Float f -> f
  in
  let evalbinop op v1 v2 =
    let dobinop iop fop v1 v2 =
      match (v1, v2) with
          (Int i, Int j) -> Int (iop i j)
        | _  -> Float ( fop (asfloat v1) (asfloat v2))
    in
    let dofunc =
      match op with
          Plus -> dobinop (+) (+.)
        | Minus -> dobinop (-) (-.)
        | Multiply -> dobinop ( * ) ( *. )
        | Divide -> dobinop (/) (/.)
        | Power -> 
            (function x -> function y -> Float( (asfloat x)**(asfloat y)) )
    in
    dofunc v1 v2
  in
  let evalunop op v =
    let dounop iop fop v = match v with
        Int i -> Int (iop i)
      | Float f -> Float (fop f)
    in
    let thingyop op v = op v v
    in
    let dofunc =
      match op with
          Negate -> dounop (~-) (~-.)
        | Square -> dounop (thingyop ( * )) (thingyop ( *. ))
        | Sqroot -> function v -> Float (sqrt (asfloat v))
    in
    dofunc v
  in
  match e with
      BinOp (op, e1, e2) -> evalbinop op (eval e1) (eval e2)
    | UnOp (op, e) -> evalunop op (eval e)
    | Value (v) -> v

let string_of_exp e =
  let rec string_of_exp_b e p =
    let dobraces p1 p2 s =
      if p1 < p2
      then "("^s^")"
      else s
    in
    let string_of_binop op e1 e2 p =
      let s, pred =
        match op with
            Plus -> "+",0
          | Minus -> "-",0
          | Multiply -> "*",2
          | Divide -> "/",2
          | Power -> "**",4
      in
      dobraces pred p
        (string_of_exp_b e1 pred)^s^(string_of_exp_b e2 pred)
    in
    let string_of_unop op e p =
      let s =
        match op with
            Negate -> "~-"
          | Square -> "sqr"
          | Sqroot -> "sqroot"
      in
      s^"("^(string_of_exp_b e p)^")"
    in
    let string_of_value v =
      match v with
          Int i -> string_of_int i
        | Float f -> string_of_float f
    in
    match e with
        BinOp (op, e1, e2) -> string_of_binop op e1 e2 p
      | UnOp (op, e) -> string_of_unop op e p
      | Value v -> string_of_value v
  in
  string_of_exp_b e 0
