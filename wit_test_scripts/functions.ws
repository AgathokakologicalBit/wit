let f_0 = a -> 0
let f_1 = (a, b) -> (c, d) -> (c,d,a,b)
let f_2 = a -> b -> c -> d -> e -> f
let f_3 = () -> () -> 10

let fc_0 = f_0(1)
let fc_1 = f_0(fc_0)
let fc_2 = f_3()()
