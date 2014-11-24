; Deklariere einen Typ "person" und zwei Prädikate für Personen.
(type person)
(predicate schüler? (list person))
(predicate dumm? (list person))

; Wir kennen eine Person Fritz. Fritz ist ein Schüler.
(person fritz)
(axiom (schüler? fritz))
; Alle Schüler sind dumm.
(axiom (forall (person x) (impl (schüler? x) (dumm? x))))

; Also ist Fritz dumm.
(statement
	(dumm? fritz)
	(proof
		(statement (impl (schüler? fritz) (dumm? fritz))
			(specialization
				(list (predicate (person x) (impl (schüler? x) (dumm? x))) person fritz)
				(list (ref 1~1))
			)
		)
		(statement (dumm? fritz)
			(ponens
				(list (schüler? fritz) (dumm? fritz))
				(list (ref 1~2) (ref 0~1))
			)
		)
	)
)