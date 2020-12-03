let div_plus = ref []
  
let rec expr ctx i =
  function
  | Command.Expression.Cst c -> Z3.Arithmetic.Integer.mk_numeral_i ctx c
  | Command.Expression.Var v ->
     Z3.Arithmetic.Integer.mk_const_s ctx (v^"$"^(string_of_int (i)))
  | Command.Expression.Op (e, o, e') ->
     let e1  = expr ctx i e
     and e'1 = expr ctx i e'
     in
     match o with
     | Command.Expression.Add -> Z3.Arithmetic.mk_add ctx [e1 ; e'1]
     | Command.Expression.Sub -> Z3.Arithmetic.mk_sub ctx [e1 ; e'1]
     | Command.Expression.Mul -> Z3.Arithmetic.mk_mul ctx [e1 ; e'1]
     | Command.Expression.Div ->
	div_plus := List.append !div_plus [(Z3.Boolean.mk_not ctx
					      (Z3.Boolean.mk_eq ctx e'1
						 (Z3.Arithmetic.Integer.mk_numeral_i ctx 0)))];
       Z3.Arithmetic.mk_div ctx e1 e'1
	 
let guard ctx i (e,o,e') =
  let e1  = expr ctx i e
  and e'1 = expr ctx i e'
  in
  match o with
  | Command.Predicate.Eq  -> Z3.Boolean.mk_eq ctx e1 e'1
  | Command.Predicate.Lst -> Z3.Arithmetic.mk_lt ctx e1 e'1
  | Command.Predicate.Gst -> Z3.Arithmetic.mk_gt ctx e1 e'1
  | Command.Predicate.Leq -> Z3.Arithmetic.mk_le ctx e1 e'1
  | Command.Predicate.Geq -> Z3.Arithmetic.mk_ge ctx e1 e'1
     
let op_skip ctx i v =
  Z3.Boolean.mk_eq ctx
    (Z3.Arithmetic.Integer.mk_const_s ctx (v^"$"^(string_of_int ( i ))))
    (Z3.Arithmetic.Integer.mk_const_s ctx (v^"$"^(string_of_int (i+1))))
    
let op_assign ctx i v0 e0 v =
  if (Variable.equal v v0)
  then Z3.Boolean.mk_eq ctx
    (Z3.Arithmetic.Integer.mk_const_s ctx (v0^"$"^(string_of_int (i+1))))
    (expr ctx i e0)
  else op_skip ctx i v
    
let op ctx var i =
  div_plus := [];
  function
  | Command.Assign (v, e) ->
     let l = List.map (op_assign ctx i v e) var in
     List.append l !div_plus
  | Command.Guard g ->
     let l = List.append [guard ctx i g] (List.map (op_skip ctx i) var) in
     List.append l !div_plus
  | Command.Skip ->
     List.map (op_skip ctx i) var
       
       
let  formula ctx var i cmd = op ctx var i cmd
     
