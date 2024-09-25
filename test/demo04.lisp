(define a '(1 1 3))
(while (eq (car a) 1) (setq a (cdr a)))
(car a)
(cdr a)