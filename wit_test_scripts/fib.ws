let repeat = n: int -> f -> arg ->
  if n > 0 then { repeat(n - 1)(f)(arg) f(arg, n) }

let fib = n: int ->
  if n < 2 then n
  else fib(n - 1) + fib(n - 2)


let main = () ->
  repeat(10)((_, i) -> print(i, ":", fib(i)))()


main()
