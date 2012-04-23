(* [[[[SimpleEnum *)
type boolean = False | True

type primary_colours = Red | Green | Blue
(* ]]]]SimpleEnum *)
(* [[[[SimpleRecord *)
type complex = float * float

type rational = int * int
(* ]]]]SimpleRecord *)
(* [[[[SmlNumberType *)
type number =
    | Int of int
    | Float of float
    | Rational of rational
    | Complex of complex
(* ]]]]SmlNumberType *)
(* [[[[SmlNumberMag *)
let magnitude n =
    match n with
    | Int i -> abs_float (float i)
    | Float f -> abs_float f
    | Complex (r,i) -> sqrt(r**2. +. i**2.)
    | Rational (a,b) -> abs_float (float a /. float b)
(* ]]]]SmlNumberMag *)

