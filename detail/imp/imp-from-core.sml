
structure ImpFromCore : sig
   val run:
      Core.Spec.t ->
         Imp.Spec.t CompilationMonad.t
end = struct

   structure CM = CompilationMonad

   exception ImpTranslationBug

   open Core
   open Imp

   fun get s = VarInfo.lookup (!SymbolTables.varTable, Atom.atom s)
   val prim_fun_table = foldl SymMap.insert' SymMap.empty
      let
         fun unboxI args = map (fn arg => UNBOXexp (INTvtype, arg)) args
         fun boxI arg = BOXexp (INTvtype, arg)
         fun boxB arg = BOXexp (BITvtype 1, arg)
         fun ftype args res = FUNvtype { result = res, closure = [], args = args }
         val iii = ftype [INTvtype, INTvtype] INTvtype
         val sv =  ftype [STRINGvtype] VOIDvtype
         val ooo = ftype [OBJvtype, OBJvtype] OBJvtype
         val oob = ftype [OBJvtype, OBJvtype] (BITvtype 1)
         val oi = ftype [OBJvtype] INTvtype
         val iib = ftype [INTvtype, INTvtype] (BITvtype 1)
         val oo = ftype [OBJvtype] OBJvtype
         val is =  ftype [INTvtype] STRINGvtype
         val os =  ftype [OBJvtype] STRINGvtype
         val oiio = ftype [OBJvtype, INTvtype, INTvtype] OBJvtype
      in [
         (get "raise", fn args => PRIexp (RAISEprim,sv,args)),
         (get (Atom.toString Op.andAlso), fn args => PRIexp (ANDprim,ooo,args)),
         (get (Atom.toString Op.orElse), fn args => PRIexp (ORprim,ooo,args)),
         (get "sx", fn args => PRIexp (SIGNEDprim,oi,args)),
         (get "zx", fn args => PRIexp (UNSIGNEDprim,oi,args)),
         (get "+", fn args => boxI (PRIexp (ADDprim,iii,unboxI args))),
         (get "-", fn args => boxI (PRIexp (SUBprim,iii,unboxI args))),
         (get "===", fn args => boxB (PRIexp (EQprim,iib,unboxI args))),
         (get "*", fn args => boxI (PRIexp (MULprim,iii,unboxI args))),
         (get "<", fn args => boxB (PRIexp (LTprim,iib,unboxI args))),
         (get ">", fn args => boxB (PRIexp (LTprim,iib,unboxI (rev args)))),
         (get "<=", fn args => boxB (PRIexp (LEprim,iib,unboxI args))),
         (get ">=", fn args => boxB (PRIexp (LEprim,iib,unboxI (rev args)))),
         (get "not", fn args => PRIexp (NOT_VECprim,oo,args)),
         (get "==", fn args => boxB (PRIexp (EQ_VECprim,oob,args))),
         (get "^", fn args => PRIexp (CONCAT_VECprim,ooo,args)),
         (get "showint", fn args => PRIexp (INT_TO_STRINGprim,is,unboxI args)),
         (get "showbitvec", fn args => PRIexp (BITVEC_TO_STRINGprim,os,args)),
         (get "+++", fn args => PRIexp (CONCAT_STRINGprim,ooo,args)),
         (get "slice", fn args => (case args of
             [sz,ofs,vec] => PRIexp (SLICEprim,oiio,unboxI [sz,ofs] @ [vec])
           | _ => raise ImpTranslationBug)),
         (get "index", fn args => PRIexp (INDEXprim,oi,args))
         ]
      end
   
   val prim_state_table = foldl SymMap.insert' SymMap.empty
      let
         fun boxI arg = BOXexp (INTvtype, arg)
         fun ftype args res = FUNvtype { result = res, closure = [], args = args }
         val ov =  ftype [OBJvtype] VOIDvtype
         val vi = ftype [OBJvtype] INTvtype
      in [
         (get "ipget", fn (res,args) => STATEstmt (res,INmonkind,IPGETprim,vi,args)),
         (get "consume8", fn (res,args) => STATEstmt (res,INOUTmonkind,CONSUME8prim,vi,args)),
         (get "consume16", fn (res,args) => STATEstmt (res,INOUTmonkind,CONSUME16prim,vi,args)),
         (get "consume32", fn (res,args) => STATEstmt (res,INOUTmonkind,CONSUME32prim,vi,args)),
         (get "unconsume8", fn (res,args) => STATEstmt (res,INOUTmonkind,UNCONSUME8prim,vi,args)),
         (get "unconsume16", fn (res,args) => STATEstmt (res,INOUTmonkind,UNCONSUME16prim,vi,args)),
         (get "unconsume32", fn (res,args) => STATEstmt (res,INOUTmonkind,UNCONSUME32prim,vi,args)),
         (get "println", fn (res,args) => STATEstmt (res,INmonkind,PRINTLNprim,ov,args))
         ]
      end
   
   val constructors: (Spec.sym * Spec.ty option) SymMap.map ref = ref SymMap.empty

   fun freeVars (Exp.LETVAL (s,b,e)) =
      SymSet.union (freeVars b,
         SymSet.difference (freeVars e, SymSet.singleton s))
     | freeVars (Exp.LETREC (ds,e)) =
      foldl (fn ((f, args, body), ss) =>
            SymSet.union (ss,
               SymSet.difference (
                  freeVars body,
                  SymSet.addList (SymSet.singleton f, args)
               )
            )) (freeVars e) ds
     | freeVars (Exp.IF (c,e,t)) =
      SymSet.union (freeVars c, SymSet.union (freeVars e, freeVars t))
     | freeVars (Exp.CASE (e, cases)) =
      let
         fun freeInCase (Pat.CON (c, SOME a), e) =
            SymSet.difference (freeVars e, SymSet.fromList [a,c])
           | freeInCase (Pat.CON (c, NONE), e) =
            SymSet.difference (freeVars e, SymSet.singleton c)
           | freeInCase (Pat.ID s, e) =
            SymSet.difference (freeVars e, SymSet.singleton s)
           | freeInCase (_, e) = freeVars e
      in
         foldl (fn (c, ss) => SymSet.union (freeInCase c, ss)) (freeVars e) cases
      end
     | freeVars (Exp.APP (e,args)) =
      foldl (fn (arg,ss) => SymSet.union (freeVars arg, ss)) (freeVars e) args
     | freeVars (Exp.PRI (_,args)) = SymSet.addList (SymSet.empty, args)
     | freeVars (Exp.FN (s,e)) =
      SymSet.difference (freeVars e, SymSet.singleton s)
     | freeVars (Exp.SEQ seq) =
      let
         fun gather (Exp.ACTION t :: rem) =
               SymSet.union (freeVars t, gather rem)
           | gather (Exp.BIND (s,t) :: rem) =
               SymSet.union (freeVars t,
                  SymSet.difference (gather rem, SymSet.singleton s))
           | gather [] = SymSet.empty
      in
         gather seq
      end
     | freeVars (Exp.ID s) = SymSet.singleton s
     | freeVars _ = SymSet.empty


   fun addLocalVar { globalVars = gv, localVars = lv, declVars = ds, mutables = ms } sym =
      let
         val _ = ds := SymSet.add (!ds, sym)
      in
         { globalVars = gv, localVars = SymSet.add (lv,sym), declVars = ds, mutables = ms }
      end
   fun genTmpVar { globalVars = gv, localVars = lv, declVars = ds, mutables = ms } =
      let
         val tab = !SymbolTables.varTable
         val (tab, sym) = SymbolTable.fresh (tab, Atom.atom "tmp")
         val _ = SymbolTables.varTable := tab
         val _ = ds := SymSet.add (!ds, sym)
       in
         sym
      end
   fun withNewDeclVars { globalVars = gv, localVars = lv, declVars = ds, mutables = ms } f =
      let
         val localDs = ref SymSet.empty
         val res = f { globalVars = gv, localVars = lv, declVars = localDs, mutables = ms }
      in
         (res, !localDs)
      end
   fun addGlobal { globalVars = gv, localVars = lv, declVars = ds, mutables = ms } v =
      { globalVars = SymSet.add (gv,v), localVars = lv, declVars = ds, mutables = ms }
   
   (* functions operating on the mutable variables *)
   fun addFunction { globalVars = gv, localVars = lv, declVars = ds, mutables = ms } decl =
      let
         val { functions = fs, updates = us, queries = qs, records = rs } = ms
      in
         fs := decl :: !fs
      end
   fun addUpdate s fields =
      let
         val tab = !SymbolTables.varTable
         val name = Atom.atom (foldl
                     (fn ((ty,sym), str) =>
                        str ^ "_" ^ SymbolTable.getInternalString (tab,sym))
                     "update" fields)
      in
         case SymbolTable.find (tab, name) of
            NONE =>
               let
                  val (tab, sym) = SymbolTable.fresh (tab, name)
                  val _ = SymbolTables.varTable := tab
               in
                  sym
               end
          | SOME sym => sym
      end
   fun addSelect s field =
      let
         val tab = !SymbolTables.varTable
         val name = Atom.atom ("select_" ^ 
                               SymbolTable.getInternalString (tab,field))
      in
         case SymbolTable.find (tab, name) of
            NONE =>
               let
                  val (tab, sym) = SymbolTable.fresh (tab, name)
                  val _ = SymbolTables.varTable := tab
               in
                  sym
               end
          | SOME sym => sym
      end
   
   fun trExpr s (Exp.LETVAL (x,b,e)) =
      let
         val (bStmts, bExp) = trExpr (addLocalVar s x) b
         val (eStmts, eExp) = trExpr s e
      in
         (bStmts @ ASSIGNstmt (x,bExp) :: eStmts, eExp)
      end
     | trExpr s (Exp.LETREC (ds, e)) =
      let
         val s = foldl (fn ((f,_,_),s) => addGlobal s f) s ds
         val ((eStmts, eExp), localDecls) =
            withNewDeclVars s (fn s => trExpr s e)
         val _ = List.map (trDecl s) ds
      in
         (eStmts, eExp)
      end
     | trExpr s (Exp.IF (c,t,e)) =
      let
         val res = genTmpVar s
         val (cStmts, cExp) = trExpr s c
         val (tStmts, tExp) = trExpr s t
         val tStmts = tStmts @ [ASSIGNstmt (res, tExp)]
         val (eStmts, eExp) = trExpr s e
         val eStmts = eStmts @ [ASSIGNstmt (res, eExp)]
      in
         (cStmts @ [IFstmt (cExp, tStmts, eStmts)], IDexp res)
      end
     | trExpr s (Exp.CASE (e, cs)) =
      let
         val (stmts, exp) = trExpr s e
         val res = genTmpVar s
         fun trCase (pat,e) =
            let
               val (stmts, exp) = trExpr s e
            in
               (pat, stmts @ [ASSIGNstmt (res,exp)])
            end
         val cases = map trCase cs
      in
         (stmts @ [CASEstmt (exp, cases)], IDexp res)
      end
     | trExpr s (Exp.APP (func, args)) =
      let
         val (stmts, funcExp) = trExpr s func
         val (stmtss, argExps) = foldl (fn (arg, (stmtss, args)) =>
            case trExpr s arg of (stmts, argExp) =>
            (stmtss @ stmts, args @ [argExp])) ([],[]) args
      in
         (stmtss, INVOKEexp (funcExp, argExps))
      end
     | trExpr s (Exp.PRI (name, args)) =
         (case SymMap.find (prim_fun_table, name) of
            SOME gen => ([], gen (map IDexp args))
          | NONE => (case SymMap.find (prim_state_table, name) of
             SOME gen =>
               let
                  val res = genTmpVar s
               in
                  ([gen (res,map IDexp args)], IDexp res)
               end
           | NONE => raise ImpTranslationBug))
     | trExpr s (Exp.FN (var, e)) =
      let
         val tab = !SymbolTables.varTable
         val (tab, sym) = SymbolTable.fresh (tab, Atom.atom "lambda")
         val _ = SymbolTables.varTable := tab
         val fType = trDecl (addLocalVar s var) (sym, [var], e)
      in
         ([], CLOSUREexp (fType, sym, [(OBJvtype, var)]))
      end
     | trExpr s (Exp.RECORD fs) =
      let
         fun trans acc res [] = (acc,res)
           | trans acc res ((f,e) :: es) = (case trExpr s e of
              (stmts, e') => trans (acc @ stmts) (res @ [(f,e')]) es)
         val (stmts, unsortedFields) = trans [] [] fs
         fun fieldCmp ((f1,_),(f2,_)) = SymbolTable.compare_symid (f1,f2)
         val fields = ListMergeSort.uniqueSort fieldCmp unsortedFields
      in
         (stmts, RECORDexp fields)
      end
     | trExpr s (Exp.UPDATE us) =
      let
         (* evaluate expressions in the sequence as they were specified
            by then generate an update function with sorted arguments *)
         fun trans acc res [] = (acc,res)
           | trans acc res ((f,e) :: es) = (case trExpr s e of
              (stmts, e') => trans (acc @ stmts) (res @ [(f,e')]) es)
         val (stmts, unsortedUpdates) = trans [] [] us
         fun updateCmp ((f1,_),(f2,_)) = SymbolTable.compare_symid (f1,f2)
         val updates = ListMergeSort.uniqueSort updateCmp unsortedUpdates
         val args = map (fn (f,_) => (OBJvtype,f)) updates
         val updateFun = addUpdate s args
         val fType = FUNvtype { result = OBJvtype,
                                closure = map (fn (t,_) => t) args,
                                args = [OBJvtype] }
      in
         (stmts, CLOSUREexp (fType, updateFun, args))
      end
     | trExpr s (Exp.SELECT f) =
      let
         val selectFun = addSelect s f
      in
         ([], (IDexp selectFun))
      end
     | trExpr s (Exp.SEQ seq) =
      let
         fun transSeq s acc [Exp.ACTION e] =
               let
                  val (stmts, exp) = trExpr s e
               in
                  (stmts @ acc, exp)
               end
           | transSeq s acc ((Exp.ACTION e) :: seq) =
               let
                  val (stmts, exp) = trExpr s e
               in
                  transSeq s (stmts @ acc) seq
               end
           | transSeq s acc ((Exp.BIND (sym,e)) :: seq) =
               let
                  val (stmts, exp) = trExpr s e
               in
                  transSeq s (stmts @ acc) seq
               end
           | transSeq s acc _ = raise ImpTranslationBug
      in
         transSeq s [] seq
      end
     | trExpr s (Exp.LIT lit) = ([], LITexp lit)
     | trExpr s (Exp.CON sym) =
      (case SymMap.find (!constructors, sym) of
         NONE => ([], BOXexp (INTvtype, CONexp sym))
       | SOME _ =>  ([], CLOSUREexp (OBJvtype, sym, []))
       )
     | trExpr s (Exp.ID sym) = ([], IDexp sym)
   and trDecl s (sym, args, body) =
      let
         val ((stmts, exp), declVars) =
            withNewDeclVars s (fn s => trExpr s body)
         val availInClosure = SymSet.union (#globalVars s,
               SymSet.addList (SymSet.singleton sym, args))
         val inClosure = SymSet.difference (freeVars body, availInClosure)
         fun setToArgs ss = map (fn s => (OBJvtype, s)) (SymSet.listItems ss)
         val clArgs = setToArgs inClosure
         val stdArgs = map (fn s => (OBJvtype, s)) args
         val fType = FUNvtype { result = OBJvtype,
                                closure = map (fn (t,_) => t) clArgs,
                                args = map (fn (t,_) => t) stdArgs }
         val _ =
            addFunction s (FUNCdecl {
              funcMonadic = INOUTmonkind,
              funcClosure = clArgs,
              funcType = fType,
              funcName = sym,
              funcArgs = map (fn s => (OBJvtype, s)) args,
              funcLocals = setToArgs declVars,
              funcBody = stmts,
              funcRes = exp
            })
      in
         fType
      end

   fun translate spec =
      Spec.upd
         (fn cs =>
            let
               val () = constructors := Spec.get#constructors spec
               fun exports cs =
                  rev (foldl
                     (fn ((f, _, _), acc) => 
                        let
                           val fld =  f
                        in
                           (fld, Exp.ID f)::acc
                        end)
                     [] cs)
               fun exports spec =
                  let 
                     val es = Spec.get#exports spec
                  in
                     map (fn e => (Exp.ID e)) es
                  end
               val fs = ref ([] : decl list)
               val us = ref ([] : decl list)
               val qs = ref ([] : decl list)
               val rs = ref ([] : decl list)
               val ms = { functions = fs,
                          updates = us,
                          queries = qs,
                          records = rs }
               val initialState = { globalVars = SymSet.empty,
                                    localVars = SymSet.empty,
                                    declVars = ref SymSet.empty,
                                    mutables = ms
                                   }
               val bogusExp = Exp.LIT (SpecAbstractTree.INTlit 42)
               val _ = trExpr initialState (Exp.LETREC (cs, bogusExp))
            in
               !(#functions ms) 
            end) spec

   fun dumpPre (os, spec) = Pretty.prettyTo (os, Core.PP.spec spec)
   fun dumpPost (os, spec) = Pretty.prettyTo (os, Imp.PP.spec spec)
 
   val translate =
      BasicControl.mkKeepPass
         {passName="impConversion",
          registry=CPSControl.registry,
          pass=translate,
          preExt="core",
          preOutput=dumpPre,
          postExt="imp",
          postOutput=dumpPost}

   fun run spec = CM.return (translate spec)
end
