#!/usr/local/bin/vcsi
(dynamic-load "/home/fm/c-progs/vcsi/modules/http/http.so")

(define (gamerscore x)
  
  (cond
   ((string-ci=? x "fm") (set! x "strego"))
   ((string-ci=? x "bisstardo") (set! x "b1sstardo"))
   ((string-ci=? x "snoogins") (set! x "bennid"))
   ((string-ci=? x "debo") (set! x "sassydebo"))
   ((string-ci=? x "lostxanadu") (set! x "L0stXanadu")))
  
  (define gs (http-simple-get (format "http://gamercard.xbox.com/~a.card" x)))
  
  (cadr (=~ gs "/<span class=\\\"XbcFRAR\\\">([1234567890]+)<\/span>/")))

(display (gamerscore "fm"))
