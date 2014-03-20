;;
;; VCSI - SRFI-0 / cond-expand Implementation
;;

(define-syntax cond-expand
  (syntax-rules (and or not else srfi-0 srfi-1 fi-8 srfi-28)
    ((cond-expand) (error "Unfulfilled cond-expand" cond-expand))
    ((cond-expand (else body ...))
     (begin body ...))
    ((cond-expand ((and) body ...) more-clauses ...)
     (begin body ...))
    ((cond-expand ((and req1 req2 ...) body ...) more-clauses ...)
     (cond-expand
      (req1
       (cond-expand
	((and req2 ...) body ...)
	more-clauses ...))
      more-clauses ...))
    ((cond-expand ((or) body ...) more-clauses ...)
     (cond-expand more-clauses ...))
    ((cond-expand ((or req1 req2 ...) body ...) more-clauses ...)
     (cond-expand
      (req1
       (begin body ...))
      (else
       (cond-expand
	((or req2 ...) body ...)
	more-clauses ...))))
    ((cond-expand ((not req) body ...) more-clauses ...)
     (cond-expand
      (req
       (cond-expand more-clauses ...))
      (else body ...)))
    ((cond-expand (srfi-0 body ...) more-clauses ...)
     (begin body ...))
    ((cond-expand (srfi-1 body ...) more-clauses ...)
     (begin body ...))
    ((cond-expand (srfi-8 body ...) more-clauses ...)
     (begin body ...))
    ((cond-expand (srfi-28 body ...) more-clauses ...)
     (begin body ...))
    ((cond-expand (feature-id body ...) more-clauses ...)
     (cond-expand more-clauses ...))))
