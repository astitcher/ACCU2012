type expression =
  | Plus of expression * expression
  | Minus of expression * expression
  | Times of expression * expression
  | Divide of expression * expression
  | Power of expression * expression
  | Negate of expression
  | Variable of string
  | Integer of int
  | Float of float


let rec string_of_exp = function
  | Plus (e1, e2) -> "("^(string_of_exp e1)^"+"^(string_of_exp e2)^")"
  | Minus (e1, e2) -> "("^(string_of_exp e1)^"-"^(string_of_exp e2)^")"
  | Times (e1, e2) -> "("^(string_of_exp e1)^"*"^(string_of_exp e2)^")"
  | Divide (e1, e2) -> "("^(string_of_exp e1)^"/"^(string_of_exp e2)^")"
  | Power (e1, e2) -> "("^(string_of_exp e1)^"^"^(string_of_exp e2)^")"
  | Negate (e1) -> "-"^(string_of_exp e1)
  | Variable v -> v
  | Integer i -> string_of_int i
  | Float f -> string_of_float f

(* Model environment as function used to lookup variable *)
let rec eval exp env =
  let lookup v env = env v
  in
  match exp with
    | Plus (e1, e2) -> (eval e1 env) +. (eval e2 env)
    | Minus (e1, e2) -> (eval e1 env) -. (eval e2 env)
    | Times (e1, e2) -> (eval e1 env) *. (eval e2 env)
    | Divide (e1, e2) -> (eval e1 env) /. (eval e2 env)
    | Power (e1, e2) -> (eval e1 env) ** (eval e2 env)
    | Negate (e1) -> ~-. (eval e1 env)
    | Integer i -> float i
    | Float f -> f
    | Variable v -> lookup v env

exception VariableNotfound of string

(* Pretty roundabout quadratic solver *)
let quadratic_solver a b c =
  let vars = function
    | "a" -> a
    | "b" -> b
    | "c" -> c
    | v -> raise (VariableNotfound v)
  in
  let quadraticP =
    Divide(
        Plus(Negate (Variable "b"),Power(Minus(Power(Variable "b",Integer 2),Times(Integer 4,Times(Variable "a",Variable "c"))),Float 0.5)),
        Times(Integer 2,Variable "a"))
  in
  let quadraticN =
    Divide(
        Minus(Negate (Variable "b"),Power(Minus(Power(Variable "b",Integer 2),Times(Integer 4,Times(Variable "a",Variable "c"))),Float 0.5)),
        Times(Integer 2,Variable "a"))
  in
    Printf.printf "%s\n%f\n%s\n%f\n"
        (string_of_exp quadraticP) (eval quadraticP vars)
        (string_of_exp quadraticN) (eval quadraticN vars)

(* Now some more complex matching *)
let multiplyout = function
  | Times(Plus(a,b),Plus(c,d)) -> Plus(Plus(Times(a,c),Times(a,d)),Plus(Times(b,c),Times(b,d)))
  | e -> e

let lift = function
  | Plus(a,b) when a=b -> Times(Integer 2, a)
  | Times(a,b) when a=b -> Power(a, Integer 2)
  | e -> e

let identity = function
  | Plus(e,Integer 0)
  | Plus(e,Float 0.)
  | Plus(Integer 0, e)
  | Plus(Float 0., e)
  | Minus(e,Integer 0)
  | Minus(e,Float 0.)
  | Minus(Integer 0, e)
  | Minus(Float 0., e)
  | Times(e, Integer 1)
  | Times(e, Float 1.)
  | Times(Integer 1, e)
  | Times(Float 1., e)
  | Divide(e, Integer 1)
  | Divide(e, Float 1.)
  | Power(e, Integer 1)
  | Power(e, Float 1.)
  | Negate(Negate (e)) -> e
  | e -> e

let constants = function
  | Plus (Integer i1, Integer i2) -> Integer (i1 + i2)
  | Plus (e1, Negate (e2)) -> Minus (e1, e2)
  | Plus (Negate (e1), e2) -> Minus (e2, e1)
  | Minus (e1, e2) when e1=e2 -> Integer 0
  | Minus (Integer i1, Integer i2) -> Integer (i1 - i2)
  | Times (Integer i1, Integer i2) -> Integer (i1 * i2)
  | Times (Negate (e1), Negate (e2)) -> Times (e1, e2)
  | Times (e1, Negate (e2))
  | Times (Negate (e1), e2) -> Negate (Times (e1, e2))
  | Divide (Integer i1, Integer i2) -> Integer (i1 / i2)
  | Power (Float f1, Float f2) -> Float ( f1 ** f2)
  | Negate (Integer i) -> Integer (-i)
  | Negate (Float f) -> Float (-.f)
  | e -> e

let rec apply f = function
  | Plus (e1, e2) -> f (Plus (apply f e1, apply f e2))
  | Minus (e1, e2) -> f (Minus (apply f e1, apply f e2))
  | Times (e1, e2) -> f (Times (apply f e1, apply f e2))
  | Divide (e1, e2) -> f (Divide (apply f e1, apply f e2))
  | Power (e1, e2) -> f (Power (apply f e1, apply f e2))
  | Negate (e) -> f (Negate (apply f e))
  | e -> e

let simplify_test = 
  let identity = apply identity in
  let constants = apply constants in
  let lift = apply lift in
  let e1 = Times(Plus(Variable "x",Integer 1),Plus(Variable "x",Negate (Integer 1)))
  in
    print_string (string_of_exp e1);
    print_newline();
    print_string (string_of_exp (lift (constants (identity (multiplyout e1)))));
    print_newline()

let () =
  quadratic_solver 4. 0. (-2.);
  simplify_test


