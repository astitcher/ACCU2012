(* [[[[SmlIntlist *)
type intlist = Cons of int * intlist | Nil

let rec length il = 
  match il with
    | Nil -> 0
    | Cons (_, tl) -> 1 + length(tl)

let seq = Cons (12, Cons (23, Nil))
(* ]]]]SmlIntlist *)
(* [[[[SmlIntlistAcc *)
let length1 il = 
  let rec len n il =
  match il with
    | Nil -> n
    | Cons (_, tl) -> len (n+1) tl
  in
  len 0 il
(* ]]]]SmlIntlistAcc *)

let () =
    let a = Cons (12, Cons (23, Nil)) in
    Printf.printf "%d\n" (length a);
    Printf.printf "%d\n" (length1 a)

