type intlist = Cons of int * intlist | Nil

let rec length il = 
  match il with
    | Nil -> 0
    | Cons (_, il) -> 1 + length(il)

