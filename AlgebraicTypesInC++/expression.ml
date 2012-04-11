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

let _ =
  let vars = function
    | "a" -> 4.
    | "b" -> 0.
    | "c" -> -2.
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