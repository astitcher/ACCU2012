type intlist = Cons of int * intlist | Nil

let rec length il = 
  match il with
    | Nil -> 0
    | Cons (_, il) -> 1 + length(il)

let () =
    let a = Cons (12, Cons (23, Nil)) in
    Printf.printf "%d\n" (length a)

