(* [[[[SmlIntlist *)
type intlist = Cons of int * intlist | Nil

let rec length il = 
  match il with
    | Nil -> 0
    | Cons (_, il) -> 1 + length(il)
(* ]]]]SmlIntlist *)
(* [[[[SmlIntlistAcc *)
let length1 il = 
  let rec len n il =
  match il with
    | Nil -> n
    | Cons (_, il) -> len (n+1) il
  in
  len 0 il
(* ]]]]SmlIntlistAcc *)

let () =
    let a = Cons (12, Cons (23, Nil)) in
    Printf.printf "%d\n" (length a);
    Printf.printf "%d\n" (length1 a)

