type result =
  | Path of (Command.t * Automaton.Node.t) list
  | Empty of bool

let rec _search automaton bound solver ctx var z3list i node =
  Z3.Solver.add solver z3list;
  Z3.Solver.push solver;
  let status = Z3.Solver.check solver [] in
  Z3.Solver.pop solver 1;
  match status with
  | Z3.Solver.UNKNOWN       -> Empty false
  | Z3.Solver.UNSATISFIABLE -> Empty true
  | Z3.Solver.SATISFIABLE   ->
     if (Automaton.Node.equal node (Automaton.final automaton)) then Path []
     else if (i == bound) then Empty false
     else
       let res = ref (Empty true)
       and succs = Automaton.succ automaton node
       in
       for j = 0 to ((List.length succs) - 1) do
	 match !res with
	 | Path _ -> raise Exit
	 | Empty rese ->
	    let (cmd,loc) = List.nth succs j
	    in
	    let canard = _search automaton bound solver ctx var
	      (z3list @ (Semantics.formula ctx var i cmd)) (i+1) loc
	    in
	    match canard with
	    | Path p -> res := Path ([(cmd,loc)] @ p)
	    | Empty e -> res := Empty (rese && e)
       done;
       !res
	 
      
let search automaton bound =
  let ctx = Z3.mk_context []
  and var = Automaton.variables automaton
  and init = Automaton.initial automaton
  in let solver = Z3.Solver.mk_simple_solver ctx
  in
  _search automaton bound solver ctx var [] 0 init
