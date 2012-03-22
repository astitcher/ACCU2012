type value =
    Int of int
  | Float of float

type binop =
    Plus
  | Minus
  | Multiply
  | Divide
  | Power

let asfloat v =
  match v with
      Int i -> (float i)
    | Float f -> f

let lookup_binop =
  let dobinop iop fop v1 v2 =
    match (v1, v2) with
        (Int i, Int j) -> Int (iop i j)
      | _  -> Float ( fop (asfloat v1) (asfloat v2))
  in
  function
    | Plus -> "+",0, dobinop (+) (+.)
    | Minus -> "-",0, dobinop (-) (-.)
    | Multiply -> "*",2, dobinop ( * ) ( *. )
    | Divide -> "/",2, dobinop (/) (/.)
    | Power -> "**",4, 
        (function x -> function y -> Float( (asfloat x)**(asfloat y)) )

type unop =
    Negate
  | Square
  | Sqroot

let lookup_unop =
  let dounop iop fop v = match v with
      Int i -> Int (iop i)
    | Float f -> Float (fop f)
  in
  let thingyop op v = op v v
  in
  function
    | Negate -> "~-", dounop (~-) (~-.)
    | Square -> "sqr", dounop (thingyop ( * )) (thingyop ( *. ))
    | Sqroot -> "sqroot", function v -> Float (sqrt (asfloat v))

type expression =
    BinOp of binop * expression * expression
  | UnOp of unop * expression
  | Value of value

let rec eval e =
  let evalbinop op v1 v2 =
    let _, _, dofunc = lookup_binop op
    in
    dofunc v1 v2
  in
  let evalunop op v =
    let _, dofunc = lookup_unop op
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
      let s, pred, _ = lookup_binop op
      in
      dobraces pred p
        ((string_of_exp_b e1 pred)^s^(string_of_exp_b e2 pred))
    in
    let string_of_unop op e p =
      let s, _ = lookup_unop op
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
