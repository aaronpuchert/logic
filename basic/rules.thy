; Syntax:
; (rule name variable-list statement)
; (equivrule name variable-list statement statement)
; (deductionrule name variable-list statement-list statement)

; SÄTZE DER AUSSAGENLOGIK
; Satz vom ausgeschlossenen Dritten
(rule excluded_middle (list (statement a)) (or a (not a)))
; Satz vom Widerspruch
(rule noncontradiction (list (statement a)) (not (and a (not a))))
; Satz von der doppelten Verneinung
(equivrule double_negation (list (statement a)) (not (not a)) a)
; DeMorgan...
; Kontraposition...
; Satz zum Modus ponens
(deductionrule ponens (list (statement a) (statement b)) (list (impl a b) a) b)
; Satz zum Modus tollens
(deductionrule tollens (list (statement a) (statement b)) (list (impl a b) (not b)) (not a))
; Satz zum Modus Barbara
(deductionrule barbara (list (statement a) (statement b) (statement c)) (list (impl a b) (impl b c)) (impl a c))
; Distributivsätze...

; Regeln für and
(deductionrule and_first (list (statement a) (statement b)) (list (and a b)) a)
(deductionrule and_second (list (statement a) (statement b)) (list (and a b)) b)
(deductionrule and_compose (list (statement a) (statement b)) (list a b) (and a b))
; Regeln für or
(deductionrule or_first (list (statement a) (statement b)) (list a) (or a b))
(deductionrule or_second (list (statement a) (statement b)) (list b) (or a b))
(deductionrule or_neg_first (list (statement a) (statement b)) (list (or a b) (not a)) (not b))
(deductionrule or_neg_second (list (statement a) (statement b)) (list (or a b) (not b)) (not a))

; SÄTZE DER PRADIKATENLOGIK
; Umbenennung
; ... ist das eine Regel, oder ist das elementar?
; Spezialisierung
(deductionrule specialization ((predicate P) (typename T) (T y)) (forall (T x) P(x)) P(y))
; Was sonst?
