;;
;; VCSI - SRFI-8 / Receive: Binding to Multiple Values
;;

(define-syntax receive
  (syntax-rules ()
    ((receive formals expression body ...)
     (call-with-values (lambda () expression)
                       (lambda formals body ...)))))
