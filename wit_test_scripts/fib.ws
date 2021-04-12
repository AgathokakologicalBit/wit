let fib = n ->
  if n < 2 then n
  else n * fib(n - 1)


let main = () ->
{
  print(fib(1))
  print(fib(2))
  print(fib(3))
}

main()
