(define mylist '(1 2 3))
(car mylist)
(cdr mylist)

(setq mylist (cdr mylist))
(car mylist)
(cdr mylist)

(if (equal 2 (cdr mylist)) (+ 666 0) (+ "fuck" "~"))