let print_solutions = n ->
{
  let __print_solutions_helper = (n: int, sgen) ->
    if n < 1 then {}
    else if n == 1 then print(sgen("1"))
    else {
      if n % 3 == 0 then __print_solutions_helper(n / 3 : int, v: string -> sgen("(" + v + ") * 3"))
      __print_solutions_helper(n - 5, v: string -> sgen(v + " + 5"))
    }
  
  __print_solutions_helper(n : int, v: string -> v)
}


let main = () ->
  input("Enter a number: ", print_solutions)


main()
